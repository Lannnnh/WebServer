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
                             const std::string &nameArg,
                             int sockfd,
                             const ::sockaddr_in &localAddr,
                             const ::sockaddr_in &peerAddr)
    : loop_(loop),
      name_(nameArg),
      state_(kConnecting),
      reading_(false),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64*1024*1024)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead(), this, _1)); //bug
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite(), this)); // bug
    
}
