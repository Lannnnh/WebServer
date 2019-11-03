#include "TcpServer.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Socket.h"
#include "EventLoopThreadPool.h"

#include <stdio.h>

TcpServer::TcpServer(EventLoop *loop,
                     const ::sockaddr_in &listenAddr,
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



void TcpServer::newConnection(int sockfd, const struct ::sockaddr_in *peeraddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    // LOG_INFO << "TcpServer::newConnection [" << name_
    //          << "] - new connection [" << connName
    //          << "] from" << peeraddr;
}