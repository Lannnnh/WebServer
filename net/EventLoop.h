#ifndef _NET_EVENTLOOP_
#define _NET_EVENTLOOP_

#include "base/nocopyable.h"
#include <sys/types.h>

class EventLoop : nocopyable
{
    public:
        EventLoop();
        ~EventLoop();

        void loop();

        // void assertInLoopThread()
        // {
        //     if (!isInLoopThread())
        //     {
        //         abortNotInLoopThread();
        //     }
        // }

        //bool isInLoopThread() const () { return threadId_ = CurrentThread::tid(); }
    
    private:
        void abortNotInLoopThread();

        bool looping_;
        const pid_t threadId_;
};

#endif // end _NET_EVENTLOOP_