#ifndef _NET_EVENTLOOP_
#define _NET_EVENTLOOP_

#include "base/nocopyable.h"
#include "base/CurrentThread.h"
#include <sys/types.h>

class EventLoop : nocopyable
{
    public:
        EventLoop();
        ~EventLoop();

        void loop();
        void quit();

        void assertInLoopThread()
        {
            if (!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
        static EventLoop *getEventLoopOfCurrentThread();

    private:
        void abortNotInLoopThread();

        bool looping_;
        const pid_t threadId_;
};

#endif // end _NET_EVENTLOOP_