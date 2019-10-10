#ifndef _NET_EVENTLOOP_
#define _NET_EVENTLOOP_

#include "base/nocopyable.h"
#include "base/CurrentThread.h"
#include "Poller.h"
#include <sys/types.h>
#include <memory>

class Channel;

class EventLoop : nocopyable
{
    public:
        EventLoop();
        ~EventLoop();

        void loop();
        void quit();

        void assertInLoopThread()
        {
            if (!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
        static EventLoop *getEventLoopOfCurrentThread();

        // Channel
        void updateChannel(Channel *channel);
        void removeChannel(Channel *Channel);

    private:
        void abortNotInLoopThread();

        typedef std::vector<Channel*> ChannelList;

        bool EventHanding_;
        bool quit_;
        ChannelList activeChannels_;
        std::unique_ptr<Poller> poller_;
        bool looping_;
        const pid_t threadId_;
        Channel* currentActiveChannel_;
};

#endif // end _NET_EVENTLOOP_