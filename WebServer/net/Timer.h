#ifndef _NET_TIMER_H
#define _NET_TIMER_H

#include "nocopyable.h"
#include "Atomic.h"
#include "Timestamp.h"
#include "Callbacks.h"

class Timer : nocopyable
{
    public:
        Timer(TimerCallback cb, Timestamp when, double interval)
            : callback_(std::move(cb)),
              expiration_(when),
              interval_(interval),
              repeat_(interval_ > 0.0),
              sequence_(s_numCreated_.incrementAndGet())
        {   }

        void run() const
        {
            callback_();
        }

        Timestamp expiration() const { return expiration_; }
        bool repeat() const { return repeat_; }
        int64_t sequence() const { return sequence_; }

        void restart(Timestamp now);

        static int64_t numCreated() { return s_numCreated_.get(); }

    private:
        const TimerCallback callback_;
        Timestamp expiration_;
        const double interval_;
        const bool repeat_;
        const int64_t sequence_;

        static AtomicInt64 s_numCreated_;
};


#endif // end _NET_TIMER_H