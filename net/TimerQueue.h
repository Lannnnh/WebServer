#ifndef _NET_TIMERQUEUE_H
#define _NET_TIMERQUEUE_H

#include "base/nocopyable.h"
#include "TimerId.h"
#include "Callbacks.h"
#include "base/Timestamp.h"

class EventLoop;

class TimerQueue : nocopyable
{
    public:
        TimerQueue(EventLoop *loop);
        ~TimerQueue();

        TimerId addTimer(const TimerCallback &cb,
                         Timestamp when,
                         double interval);

    private:
        EventLoop *loop_;
        const int timerfd_;
};



#endif // end _NET_TIMERQUEUE_H