#include "EventLoop.h"
#include "Channel.h"
#include "base/MutexLock.h"
#include "Socket.h"

#include <unistd.h>
#include <assert.h>
#include <algorithm>

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

EventLoop::EventLoop()
    : looping_(false), 
      threadId_(CurrentThread::tid()),
      EventHandling_(false),
      quit_(false),
      poller_(Poller::newDefaultPoller(this)),
      currentActiveChannel_(NULL)
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

// 
void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;


    while (!quit_)
    {
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        EventHandling_ = true;
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it )
        {
            currentActiveChannel_ = *it;
            (*it)->handleEvent();
        }
        currentActiveChannel_ = NULL;
        EventHandling_ = false;
    }

    // LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
}

void EventLoop::abortNotInLoopThread()
{
    // LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
    //         << " was created in threadId_ = " << threadId_
    //         << ", current thread id = " <<  CurrentThread::tid();
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
        assert(currentActiveChannel_ == channel || std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd_, (char *) &one, sizeof(one));
    if (n != sizeof(one))
    {
        //LOG_ERROR << "EventLoop::wakeip() writes " << n << " bytes instead of 8";
    }
}

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
        queueInLoop(cb);
    }
    
}