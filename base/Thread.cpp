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

// 得到线程真实的pid 称为tid
pid_t gettid()
{
    return static_cast<pid_t> (::syscall(SYS_gettid));
}

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

void Thread::start()
{
    // need add
}

int Thread::join()
{
    // need add
}

