#include "Thread.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <assert.h>
#include <sys/prctl.h>
#include <memory>
#include <sys/syscall.h>

#include "CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "default";
}

// 得到线程真实的pid 称为tid
pid_t gettid()
{
    return static_cast<pid_t> (::syscall(SYS_gettid));
}

// gettid获取的是内核中线程ID, 而pthread_self是posix描述的线程ID
void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d", t_cachedTid);
    }
}

Thread::Thread(const ThreadFunc &func, const std::string &name)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(func),
      name_(name),
      latch_(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        pthread_detach(pthreadId_);
    }
}

void Thread::setDefaultName()
{
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread");
        name_ = buf;
    }
}

// 保留线程数据
struct ThreadData
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t *tid_;
    CountDownLatch *latch_;

    ThreadData(const ThreadFunc &func, const std::string &name, pid_t *tid, CountDownLatch *latch)
      : func_(func),
        name_(name),
        tid_(tid),
        latch_(latch)
    {    }

    void runInThread()
    {
        *tid_ = CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;

        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        // 为线程设置name
        prctl(PR_SET_NAME, CurrentThread::t_threadName);

        func_();
        CurrentThread::t_threadName = "finished";
    }
};

void *startThread(void *obj)
{
    ThreadData *data = static_cast<ThreadData*> (obj);
    data->runInThread();
    delete data;
    return NULL;
}


void Thread::start()
{
    // assert的作用是先计算表达式 expression ，如果其值为假（即为0），那么它先向stderr打印一条出错信息，然后通过调用 abort 来终止程序运行。
    assert(!started_);
    started_ = true;
    ThreadData *data = new ThreadData(func_, name_, &tid_, &latch_);
    if (pthread_create(&pthreadId_, NULL, &startThread, data))
    {
        started_ = false;
        delete data;
    }
    else 
    {
        latch_.wait();
        assert(tid_ > 0);
    }

}

int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}

