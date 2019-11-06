#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <errno.h>
#include <assert.h>
#include <netinet/in.h>

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const struct ::sockaddr_in &serverAddr)
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