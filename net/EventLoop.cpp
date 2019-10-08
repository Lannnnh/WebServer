#include "EventLoop.h"
#include <unistd.h>
#include <assert.h>

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
    : looping_(false), 
      threadId_(CurrentThread::tid())
{
    // LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    // if (t_loopInThisThread)
    // {
    //     LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
    // }
    // else
    // {
        t_loopInThisThread = this;
    // }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

// 目前loop什么都不做 等待5s就退出
void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    // ::poll(NULL, 0, 5*1000); 

    // LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::abortNotInLoopThread()
{
    // LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
    //         << " was created in threadId_ = " << threadId_
    //         << ", current thread id = " <<  CurrentThread::tid();
}



