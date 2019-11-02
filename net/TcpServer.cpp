#include "TcpServer.h"
#include "EventLoop.h"
#include "Acceptor.h"



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