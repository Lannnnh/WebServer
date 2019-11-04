#include "Acceptor.h"
#include "EventLoop.h"
#include "Socket.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

Acceptor::Acceptor(EventLoop *loop, const struct sockaddr_in *listenaddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie(AF_INET)),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{   
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenaddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::hanldeRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen()
{
    loop_->assertInLoopThread;
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::hanldeRead()
{
    loop_->assertInLoopThread();
    struct ::sockaddr_in peeraddr;
    int connfd = acceptSocket_.accept(&peeraddr);

    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peeraddr);
        }
        else        
        {
            sockets::close(connfd);
        }
    }
    else
    {
        // LOG_SYSERR << "in Acceptor::handleRead";
    }
}