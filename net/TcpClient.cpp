#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "Socket.h"

#include <stdio.h>

void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn)
{
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr &connector)
{
    //connector->
}

TcpClient::TcpClient(EventLoop *loop,
                     const struct ::sockaddr_in &serverAddr,
                     const std::string &name)
    : loop_(loop),
      connector_(new Connector(loop, serverAddr)),
      name_(name),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      retry_(false),
      connect_(true),
      nextConnId_(1)
{
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, _1));
    // FIXME setConnectFailedCallback
    // LOG_INFO << "TcpClient::TcpClient[" << name_
    //          << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient()
{
    // LOG_INFO << "TcpClient::~TcpClient[" << name_
    //          << "] - connector " << get_pointer(connector_);
    TcpConnectionPtr conn;
    bool unique = false;
    {
        MutexLockGuard lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn)
    {
        assert(loop_ == conn->getLoop());

        CloseCallback cb = std::bind(&::removeConnection, loop_, _1);
        loop_->runInLoop(
            std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique)
        {
            conn->forceClose();
        }
    }
    else
    {
        connector_->stop();
        loop_->runAfter(1, std::bind(&::removeConnector, connector_));
    }
}

void TcpClient::connect()
{
    // LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
    //          << connector_->serverAddress().toIpPort();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;

    {
        MutexLockGuard lock(mutex_);
        if (connection_)
        {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
    loop_->assertInLoopThread();
    struct ::sockaddr_in peerAddr;
    // get peerAddress
    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t peerLen = static_cast<socklen_t> (sizeof(peerAddr));
    ::getpeername(sockfd, static_cast<struct ::sockaddr*> (implicit_cast<void*> (&peerAddr)), &peerLen);

    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", sockets::toIpPort(&peerAddr).c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    struct ::sockaddr_in localAddr;
    // get localAddr
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t localLen = static_cast<socklen_t> (sizeof(localAddr));
    ::getsockname(sockfd, static_cast<struct ::sockaddr*> (implicit_cast<void*> (&localAddr)), &localLen);

    TcpConnectionPtr conn(new TcpConnection(loop_,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));
    
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, _1));
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }

    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_)
    {
        // LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
        //          << connector_->serverAddress().toIpPort();

        connector_->restart();
    }

}