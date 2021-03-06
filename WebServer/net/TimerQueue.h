#ifndef _NET_TIMERQUEUE_H
#define _NET_TIMERQUEUE_H

#include "nocopyable.h"
#include "Callbacks.h"
#include "Timestamp.h"
#include "Channel.h"

#include <set>
#include <vector>

class EventLoop;
class TimerId;
class Timer;

class TimerQueue : nocopyable
{
    public:
        TimerQueue(EventLoop *loop);
        ~TimerQueue();

        TimerId addTimer(const TimerCallback cb, Timestamp when, double interval);
        
        void cancel(TimerId timerId);

    private:
        // use unique_ptr instead of raw pointer
        typedef std::pair<Timestamp, Timer*> Entry;
        typedef std::set<Entry> TimerList;
        typedef std::pair<Timer*, int64_t> ActiveTimer;
        typedef std::set<ActiveTimer> ActiveTimerSet;

        void addTimerInLoop(Timer *timer);
        void cancelInLoop(TimerId timerId);

        // call when timerfd alarms
        void handleRead();

        // move out all expired timers
        std::vector<Entry> getExpired(Timestamp now);
        void reset(const std::vector<Entry> &expired, Timestamp now);

        bool insert(Timer *timer);

        EventLoop *loop_;
        const int timerfd_;
        Channel timerfdCannel_;

        //timer list sorted by expiration
        TimerList timers_;

        // for cancel()
        ActiveTimerSet activeTimers_;
        bool callingExpiredTimers_; // atomic
        ActiveTimerSet cancelingTimers_;
};



#endif // end _NET_TIMERQUEUE_H