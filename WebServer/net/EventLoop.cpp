#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "WebServer/base/Logging.h"

#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>
#include <algorithm>

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

// eventfd用来进行线程间通信 因为IO线程平时阻塞在事件循环EventLoop::loop的poll调用里，为了IO线程能够立即执行用户回调，用eventfd来唤醒它
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false), 
      threadId_(CurrentThread::tid()),
      EventHandling_(false),
      quit_(false),
      poller_(Poller::newDefaultPoller(this)),
      currentActiveChannel_(NULL),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      callingPendingFunctors_(false)
{
    LOG << "EventLoop created "  << " in thread " << threadId_;
    if (t_loopInThisThread)
    {
        LOG << "Another EventLoop " << " exists in this thread " << threadId_;
    }
    else
    {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    LOG << "EventLoop " << " start looping";

    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        EventHandling_ = true;
        for (auto channel : activeChannels_)
        {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = NULL;
        EventHandling_ = false;
        doPendingFunctors();
    }

    LOG << "EventLoop " << " stop looping";
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;

    // 如果不是IO线程调用quit，则需要wakeup唤醒IO线程，因为IO线程可能还阻塞在poll这个位置，这样重新循环判断while(!quit)才能推出循环
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::abortNotInLoopThread()
{
    LOG << "EventLoop::abortNotInLoopThread - EventLoop "
        << " was created in threadId_ = " << threadId_
        << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();

    if (EventHandling_)
    {
        assert(currentActiveChannel_ == channel || 
               std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG << "EventLoop::wakeip() writes " << n << " bytes instead of 8";
    }
}

// 唤醒IO线程的条件 1. 调用queueInLoop的不是IO线程 2. 调用queueInLoop时，正在调用pending functor
void EventLoop::queueInLoop(Functor cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::cancel(TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

// swap的好处 1. 减小了临界区的长度（不会阻塞其他线程调用queueInLoop()） 2. 避免了死锁（functor有可能再调用queueInLoop()）
void EventLoop::doPendingFunctors()
{
    std::vector<EventLoop::Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors )
    {
        functor();
    }

    callingPendingFunctors_ = false;
}