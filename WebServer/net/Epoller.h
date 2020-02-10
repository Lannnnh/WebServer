#ifndef _NET_EPOLLER_H
#define _NET_EPOLLER_H

#include "nocopyable.h"
#include "Timestamp.h"
#include "EventLoop.h"

#include <functional>
#include <map>
#include <vector>

struct epoll_event;
class Channel;

class Epoller : nocopyable
{
    public:
        typedef std::vector<Channel*> ChannelList;

        Epoller(EventLoop* loop);
        ~Epoller();
        
        Timestamp poll(int timeoutMs, ChannelList* activeChannels);

        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);
        void assertInLoopThread() const 
        {
            ownerLoop_->assertInLoopThread();
        }
        static Epoller* newDefaultPoller(EventLoop* loop);

    protected:
        typedef std::map<int, Channel*> ChannelMap;
        ChannelMap channels_;

    private:
        static const int KInitEventListSize = 16;

        static const char* operationToString(int op);

        void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
        void update(int operation, Channel* channel);

        typedef std::vector<struct epoll_event> EventList;

        int epollfd_;
        EventList events_;
        EventLoop* ownerLoop_;
};

#endif // end _NET_EPOLLER_H