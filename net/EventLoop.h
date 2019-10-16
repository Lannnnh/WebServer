#ifndef _NET_EVENTLOOP_
#define _NET_EVENTLOOP_

#include "base/nocopyable.h"
#include "base/CurrentThread.h"
#include "Poller.h"
#include "base/MutexLock.h"

#include <sys/types.h>
#include <memory>

class Channel;

class EventLoop : nocopyable
{
    public:
        typedef std::function<void ()> Functor;

        EventLoop();
        ~EventLoop();

        void loop();
        void quit();

        // runs callback imediately in this thread
        void runInLoop(Functor cb);

        // queues callback in the loop thread
        void queueInLoop(Functor cb);

        void assertInLoopThread()
        {
            if (!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
        static EventLoop *getEventLoopOfCurrentThread();

        // Channel, internal usage
        void wakeup();
        void updateChannel(Channel *channel);
        void removeChannel(Channel *Channel);

    private:
        void abortNotInLoopThread();

        typedef std::vector<Channel*> ChannelList;

        bool EventHandling_;    // atomic
        bool callingPendingFunctors_;   // atomic
        bool quit_;
        ChannelList activeChannels_;
        std::unique_ptr<Poller> poller_;
        bool looping_;  // atomic
        const pid_t threadId_;
        Channel* currentActiveChannel_;
        int wakeupFd_;
        // unlike in TimerQueue, whis is an internal class,
        // we don't expose Channel to client.
        std::unique_ptr<Channel> wakeupChannel_;

        mutable MutexLock mutex_;
        std::vector<Functor> pendingFunctors_;
};

#endif // end _NET_EVENTLOOP_