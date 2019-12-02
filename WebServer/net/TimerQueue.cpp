#include "TimerQueue.h"

#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include "WebServer/base/type.h"
#include "WebServer/base/Logging.h"

#include <sys/timerfd.h>
#include <assert.h>
#include <unistd.h>
#include <memory>

int creatTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (timerfd < 0)
    {
        LOG << "Failed in timerfd_create";
    }
    
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t> (microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long> ((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memZero(&newValue, sizeof(newValue));
    memZero(&oldValue, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    //if (ret) LOG << "timerfd_settime()";
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    // LOG << more
    if (n != sizeof(howmany))
    {
        LOG << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(creatTimerfd()),
      timerfdCannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false)
{ 
    timerfdCannel_.setReadCallback(
        std::bind(&TimerQueue::handleRead, this));
    timerfdCannel_.enableReading();    
}

TimerQueue::~TimerQueue()
{
    timerfdCannel_.disableAll();
    timerfdCannel_.remove();
    ::close(timerfd_);

    // do not remove channel, since we're in EventLoop::dtor()
    // because we use unique_ptr, so we don't need to delete raw pointer
    for (const Entry &timer : timers_)
    {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer *timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    loop_->assertInLoopThread();
    bool earliestChanged  = insert(timer);

    if (earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

bool TimerQueue::insert(Timer *timer)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
        assert(result.second); (void) result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second); (void) result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    /* 
        reinterpret_cast<type_id> type-id 必须是一个指针、引用、算术类型、函数指针或者成员指针。
        它可以把一个指针转换成一个整数，也可以把一个整数转换成一个指针
        （先把一个指针转换成一个整数，再把该整数转换成原类型的指针，还可以得到原先的指针值）。
        仅仅是重新解释了给出的对象的比特模型而没有进行二进制转换。
        是为了映射到一个完全不同类型的意思，这个关键词在我们需要把类型映射回原有类型时用到它。
        我们映射到的类型仅仅是为了故弄玄虚和其他目的，这是所有映射中最危险的。
    */
    // 为了得到第一个未到期的Timer的迭代器，哨兵值的Timer*转换成一个较大值，因为有可能时间戳正好为now
    Entry sentry(now, reinterpret_cast<Timer*> (UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    // back_inserter 安插型迭代器可以使算法以安插（insert）方向而非覆写（overwrite）方式运作。使用它可以解决目标空间不足问题。
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (Entry &it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1); (void) n;
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end())
    {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1); (void) n;
        delete it->first;
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_)
    {
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now)
{
    Timestamp nextExpire;

    for (const Entry &it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        if (it.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            delete it.second;
        }
    }

    if (!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    // safe to callback outside critical section
    for (const Entry &it : expired)
    {
        it.second->run();
    }
    callingExpiredTimers_ = false;

    reset(expired, now);
}

