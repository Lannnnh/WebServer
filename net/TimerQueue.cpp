#include "TimerQueue.h"

#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <sys/timerfd.h>
#include <unistd.h>

int creatTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (timerfd < 0)
    {
        //LOG_SYSFATAL << "Failed in timerfd_create";
    }
    
    return timerfd;
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(creatTimerfd()),
      timerfdCannel_(loop, timerfd_),
      timers_()
{   }

TimerQueue::~TimerQueue()
{
    timerfdCannel_.disableAll();
    timerfdCannel_.remove();
    ::close(timerfd_);

    // do not remove channel, since we're in EventLoop::dtor()
    // because we use unique_ptr, so we don't need to delete raw pointer
    // for (const Entry &timer : timers_)
    // {
    //     delete timer.second;
    // }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer *timer = new Timer(std::move(cb), when, interval);
    // end in this place, continue
    //loop_->runInLoop(std::bind(&TimerQueue::add));

}

