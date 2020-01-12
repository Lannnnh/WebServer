#include "WebServer/net/Epoller.h"
#include "WebServer/net/Channel.h"
#include "WebServer/base/Logging.h"

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

void Epoller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    const int index = channel->index();
    LOG << "fd = " << channel->fd() << " events = " << channel->events()
        << " index = " << index;
    
    if (index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        // index == kDeleted
        else
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else 
    {
        // 更新一个已经存在的
        int fd = channel->fd(); (void) fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] = channel);
        assert(index == kAdded);
        // 没有新的事件响应
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        // 有新的事件响应
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Epoller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int fd = channel->fd();
    LOG << "removeChannel fd: " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    size_t n = channels_.erase(fd); (void) n;
    // 删除的数目为1
    assert(n == 1);

    if (index == kAdded)
    {
        update(EPOLL_CTL_ADD, channel);
    }
    channel->set_index(kNew);
}

void Epoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG << "epoll_ctl op = " << operationToString(operation) 
        << " fd = " << fd
        << " event = " << channel->events();

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        LOG << "ERROR: epoll_ctl " << operationToString(operation);
    }    
}

const char* Epoller::operationToString(int op)
{
    switch (op)
    {
    case EPOLL_CTL_ADD:
         return "ADD";
    case EPOLL_CTL_DEL:
         return "DEL";
    case EPOLL_CTL_MOD:
         return "MOD";
    default:
         return "Unknow Operation!";
    }
}