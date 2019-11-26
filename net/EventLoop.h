#ifndef _NET_EVENTLOOP_
#define _NET_EVENTLOOP_

#include "WebServer/base/nocopyable.h"
#include "WebServer/base/CurrentThread.h"
#include "WebServer/base/MutexLock.h"
#include "TimerId.h"
#include "WebServer/base/Timestamp.h"
#include "Callbacks.h"

#include <sys/types.h>
#include <memory>
#include <vector>

class Channel;
class TimerQueue;
class Poller;

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

        // run callback at time
        // safe to call from other threads
        TimerId runAt(Timestamp time, TimerCallback cb);

        // run callback after delay seconds
        // safe to call from other threads
        TimerId runAfter(double delay, TimerCallback cb);

        // run callback every interval seconds
        // safe to call from other threads
        TimerId runEvery(double interval, TimerCallback cb);

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
        bool hasChannel(Channel* channel);

    private:
        void abortNotInLoopThread();
        void handleRead(); // wake up
        void doPendingFunctors();

        typedef std::vector<Channel*> ChannelList;

        bool EventHandling_;    // atomic
        bool callingPendingFunctors_;   // atomic
        bool quit_;
        ChannelList activeChannels_;
        std::unique_ptr<Poller> poller_;
        std::unique_ptr<TimerQueue> timerQueue_;
        bool looping_;  // atomic
        const pid_t threadId_;
        Timestamp pollReturnTime_;
        Channel* currentActiveChannel_;
        int wakeupFd_;
        // unlike in TimerQueue, whis is an internal class,
        // we don't expose Channel to client.
        std::unique_ptr<Channel> wakeupChannel_;

        mutable MutexLock mutex_;
        std::vector<Functor> pendingFunctors_;
};

#endif // end _NET_EVENTLOOP_