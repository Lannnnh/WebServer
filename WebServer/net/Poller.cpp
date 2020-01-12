#include "Poller.h"
#include "WebServer/base/type.h"
#include "WebServer/base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"

#include <assert.h>
#include <sys/poll.h>
#include <errno.h>

Poller::Poller(EventLoop *loop)
    : ownerLoop_(loop)
{   }

Poller::~Poller() = default;

Timestamp Poller::poll(int timeoutMs, ChannelList *activeChannels)
{
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        LOG << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0)
    {
        LOG << "nothing happended";
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG << "Poller::poll()";
        }
    }
    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd )
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel *channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel *channel)
{
    assertInLoopThread();
    LOG << "fd = " << channel->fd() << " events = " << channel->events();
    if (channel->index() < 0)
    {
        // 增加一个新的去pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short> (channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int> (pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else
    {
        // 更新一个已经存在的
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int> (pollfds_.size()));
        struct pollfd &pfd =  pollfds_[idx];

        // 如果某个Channel不关心任何事件，就把pollfd.fd设为 -1 | -channel->fd()-1， 让poll忽略此项。
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.fd = channel->fd();
        pfd.events = static_cast<short> (channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent())
        {
            // ignore this pollfd
            pfd.fd = -channel->fd()-1;
        }
    }
}

void Poller::assertInLoopThread()
{
    ownerLoop_->assertInLoopThread(); 
}

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    return new Poller(loop);
}

void Poller::removeChannel(Channel *channel)
{
    assertInLoopThread();
    LOG << "fd = " << channel->fd() << "remove";
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int> (pollfds_.size()));
    const struct pollfd &pfd = pollfds_[idx]; (void) pfd;
    assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1); (void) n;
    if (implicit_cast<size_t> (idx) == pollfds_.size()-1)
    {
        pollfds_.pop_back();
    }
    else
    {
        // 获取pollfds_末尾的fd，然后和channel负责的fd位置互换，channel就去了pollfds_的末尾，然后删掉
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);
        // 有可能channel不关心任何事件 fd为-channel->fd()-1
        if (channelAtEnd < 0)
        {
            channelAtEnd = -channelAtEnd-1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
    
}

bool Poller::hasChannel(Channel *channel)
{
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}