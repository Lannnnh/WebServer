/*
    TcpConnection表示的是“一次TCP连接”，是不可再生的，一旦连接断开，这个TcpConnextion对象就没啥用了。
    TcpConnection没有发起连接的功能，构造函数参数是已经建立好连接的socket fd，初始状态是kConnetcion
*/

#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "base/WeakCallback.h"
#include "Socket.h"

#include <errno.h>

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &name,
                             int sockfd,
                             const struct sockaddr_in &localAddr,
                             const struct sockaddr_in &peerAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      reading_(false),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64*1024*1024)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    // LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
    //         << " fd=" << channel_->fd()
    //         << " state=" << stateToString();
    assert(state_ == kDisconnected);
}

void TcpConnection::send(const void *message, int len)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message, ::strlen(static_cast<const char *>(message)));
        }
        else
        {
            void (TcpConnection::*fp) (const void*, size_t) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, 
                             this, 
                             message, 
                             ::strlen(static_cast<const char *> (message))));
        }
    }
}

void TcpConnection::send(Buffer *buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            void (TcpConnection::*fp) (const void*, size_t) = &TcpConnection::sendInLoop;
            
            loop_->runInLoop(std::bind(fp,
                             this, 
                             buf->peek(),
                             buf->readableBytes()));
        }
    }
}

void TcpConnection::sendInLoop(const void *message, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        // LOG_WARN << "disconnected, give up writing";
        return;
    }
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = sockets::write(channel_->fd(), message, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                // LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                // faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        
        outputBuffer_.append(static_cast<const char*>(message)+nwrote, remaining);
        
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        // we are not writing
        socket_->shutdownWrite();
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading())
    {
        channel_->disableReading();
        reading_ = false;
    }
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    loop_->assertInLoopThread();
    int savedErrno = 0;
    
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        // LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {   
        ssize_t n = sockets::write(channel_->fd(),
                                     outputBuffer_.peek(),
                                     outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            // LOG_SYSERR << "TcpConnection::handleWrite";
        }
        
    }
    else
    {
        // LOG_TRACE << "Connection fd = " << channel_->fd()
        //           << " is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    //LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    // must be the last line
    closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
  int err = sockets::getSocketError(channel_->fd());
//   LOG_ERROR << "TcpConnection::handleError [" << name_
//             << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
