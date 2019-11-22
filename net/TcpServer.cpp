#include "TcpServer.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Socket.h"
#include "EventLoopThreadPool.h"

#include <stdio.h>
#include <memory>

TcpServer::TcpServer(EventLoop *loop,
                     const struct sockaddr_in &listenAddr,
                     const std::string &name,
                     Option option)
    : loop_(loop),
      ipPort_(sockets::toIpPort(&listenAddr)),
      name_(name),
      acceptor_(new Acceptor(loop, &listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      nextConnId_(1)
{   
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    // LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(numThreads >= 0);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if (started_.getAndSet(1) == 0)
    {
        threadPool_->start(threadInitCallback_);

        assert(!acceptor_->listenning());
        loop_->runInLoop(
            std::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd, const struct sockaddr_in &peerAddr)
{
    loop_->assertInLoopThread();
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    // LOG_INFO << "TcpServer::newConnection [" << name_
    //          << "] - new connection [" << connName
    //          << "] from " << sockets::toIpPort(peeraddr);

    struct sockaddr_in localAddr;
    socklen_t addrLen = sizeof(localAddr);
    ::getsockname(sockfd, static_cast<struct sockaddr*> (implicit_cast<void*> (&localAddr)), &addrLen);

    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    // LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
    //          << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    (void) n;
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}