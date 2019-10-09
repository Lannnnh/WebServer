#include "Channel.h"
#include "EventLoop.h"
#include <poll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fdArg)
    : loop_(loop),
      fd_(fdArg),
      event_(0),
      revents_(0),
      index_(-1)
{   }

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent()
{
    // POLLNVAL 描述字不是一个打开的文件
    if (revents_ & POLLNVAL) 
    {
        //LOG_WARN << "Channel::handle_event() POLLNVAL";
    }

    // POLLERR 发生错误
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_) errorCallback_();
    }

    // POLLIN 普通/优先级带数据可读 POLLPRI 高优先级数据可读 POLLRDHUP TCP连接被对方关闭或 对方关闭了写操作
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_) readCallback_();
    }

    // POLLOUT 普通/优先级数据可写
    if (revents_ & POLLOUT)
    {
        if (writeCallbak_) writeCallbak_();
    }
}


