#ifndef _NET_POLLER_H
#define _NET_POLLER_H

#include "base/nocopyable.h"
#include "base/Timestamp.h"

#include <functional>
#include <map>
#include <vector>

struct pollfd;
class Channel;
class EventLoop;

class Poller : nocopyable
{
    public:
        typedef std::vector<Channel*> ChannelList;

        Poller(EventLoop *loop);
        ~Poller();

        Timestamp poll(int timeoutMs, ChannelList *activeChannels);

        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        bool hasChannel(Channel *channel);
        void assertInLoopThread();
        static Poller *newDefaultPoller(EventLoop *loop);

    private:
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel*> ChannelMap;

        EventLoop *ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;
};

#endif // end _NET_POLLER_H 