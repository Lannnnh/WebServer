#ifndef _NET_EVENTLOOPTHREAD_H
#define _NET_EVENTLOOPTHREAD_H

#include "WebServer/base/nocopyable.h"
#include "WebServer/base/Thread.h"
#include "WebServer/base/MutexLock.h"
#include "WebServer/base/Condition.h"

#include <functional>

class EventLoop;

class EventLoopThread : nocopyable
{
    public:
        typedef std::function<void (EventLoop*)> ThreadInitCallback;

        EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());
        ~EventLoopThread();
        EventLoop* startLoop();

    private:
        void threadFunc();

        EventLoop* loop_;
        bool exiting_;
        Thread thread_;
        MutexLock mutex_;
        Condition cond_;
        ThreadInitCallback callback_;
};

#endif // end _NET_EVENTLOOPTHREAD_H