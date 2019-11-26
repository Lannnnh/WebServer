#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "WebServer/base/type.h"

#include <errno.h>
#include <assert.h>
#include <netinet/in.h>

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const struct sockaddr_in &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kMaxRetryDelayMs)
{
    // LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
    // LOG_DEBUG << more;
    assert(!channel_);
}

void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if (connect_)
    {
        connect();
    }
    else
    {
        // LOG_DEBUG << "don't connect";
    }
}

void Connector::stop()
{
    connect_ = false;
    loop_->runInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnecting)
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = sockets::createNonblockingOrDie(serverAddr_.sin_family);
    int ret = sockets::connect(sockfd, static_cast<struct sockaddr*> (implicit_cast<void*> (&serverAddr_)));
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case EISCONN:
             connecting(sockfd);
             break;

        case ENETUNREACH:
             retry(sockfd);
             break;

        case ENOTSOCK:
             // LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
             sockets::close(sockfd);
             break;

        default:
             // LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
             sockets::close(sockfd);
             break;
    }
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd)
{
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(
        std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(
        std::bind(&Connector::handleError, this));

    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    // cant reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

void Connector::handleWrite()
{
    // LOG_TRACE << "Connector::handleWrite" << state_;

    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);

        if (err)
        {
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if (connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                sockets::close(sockfd);
            }
        }
    }
    else
    {
        assert(state_ == kDisconnected);
    }
}

void Connector::handleError()
{
    // LOG_ERROR << "Connector::handleError state=" << state_;
    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        // LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    sockets::close(sockfd);
    setState(kDisconnected);
    if (connect_)
    {
        // LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
        //          << " in " << retryDelayMs_ << " milliseconds. ";
        loop_->runAfter(retryDelayMs_ / 1000.0,
                        std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        // LOG_DEBUG << "don't connect";
    }
}