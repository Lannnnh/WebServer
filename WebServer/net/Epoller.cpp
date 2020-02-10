#include "Epoller.h"
#include "Channel.h"
#include "Logging.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

Epoller::Epoller(EventLoop* loop) 
    : ownerLoop_(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(KInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG << "Epoller::Epoller()";
    }
}

Epoller::~Epoller()
{
    ::close(epollfd_);
}

Timestamp Epoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG << "fd total count " << channels_.size();
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),
                                 events_.size(),
                                 timeoutMs);

    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        LOG << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if (numEvents == 0)
    {
        LOG << "Nothing happened";
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG << "Epoller::poll()";
        }
    }
    return now;
}

void Epoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    assert(numEvents <= events_.size());
    for (int i = 0; i < numEvents; ++ i )
    {
        Channel* channel = static_cast<Channel*> (events_[i].data.ptr);
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);

        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}