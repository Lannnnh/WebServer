#include "Channel.h"
#include "EventLoop.h"
#include <poll.h>
#include <assert.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fdArg)
    : loop_(loop),
      fd_(fdArg),
      event_(0),
      revents_(0),
      index_(-1),
      logHup_(true),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false)
{   }

Channel::~Channel()
{
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread)
    {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if (logHup_)
        {
            // LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";    
        }
        if (closeCallback_) closeCallback_();
    }
    // POLLNVAL 描述字不是一个打开的文件
    if (revents_ & POLLNVAL)
    {
        // LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }
    // POLLERR 发生错误
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_) errorCallback_();
    }
    // POLLIN 普通/优先级带数据可读 POLLPRI 高优先级数据可读 POLLRDHUP TCP连接被对方关闭或 对方关闭了写操作
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_) readCallback_(receiveTime);
    }
    // POLLOUT 普通/优先级数据可写
    if (revents_ & POLLOUT)
    {
        if (writeCallback_) writeCallback_();
    }

    eventHandling_ = false;
}

void Channel::handleEvent(Timestamp receiveTime)
{
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}
