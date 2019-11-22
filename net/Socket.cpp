#include "Socket.h"
#include "base/type.h"

#include <netinet/in.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <unistd.h>

int sockets::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

std::string sockets::toIpPort(const struct sockaddr_in *addr)
{
    char buf[64] = "";
    ::inet_ntop(AF_INET, &addr->sin_port, buf, sizeof(buf));
    return buf;
}

int sockets::createNonblockingOrDie(sa_family_t family)
{
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        //LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }

    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr *addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    if (ret < 0)
    {
        //LOG_SYSFATAL << "sockets::bindOrDie";
    }
}

void sockets::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        //LOG_SYSFATAL << "sockets::listenOrDie";
    }
}

int sockets::accept(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

    int connfd = ::accept4(sockfd, static_cast<struct sockaddr*> (implicit_cast<void*> (addr)),
                            &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd < 0)
    {
        //LOG_SYSERR << "Socket::accept";
    }
    
    return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

ssize_t sockets::read(int sockfd, void *buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        //LOG_SYSERR << "sockets::close";
    }
}

void sockets::shutdownWrite(int sockfd)
{
    if (::shutdown(sockfd, SHUT_WR) < 0)
    {
        //LOG_SYSERR << "sockets::shutdownWrite";
    }
}

Socket::~Socket()
{
    sockets::close(sockfd_);
}

void Socket::bindAddress(const struct sockaddr_in *localaddr)
{
    sockets::bindOrDie(sockfd_, static_cast<const struct sockaddr*>(implicit_cast<const void*> (localaddr)));
}

void Socket::listen()
{
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(struct sockaddr_in* peeraddr)
{
    struct sockaddr_in addr;
    memZero(&addr, sizeof(addr));
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) 
    {
        peeraddr = &addr;
    }

    return connfd;
}

void Socket::shutdownWrite()
{
    sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                &optval, static_cast<socklen_t>(sizeof optval));
}

// 消除bind时因为time_wait引起的“Address already in use”错误
void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                            &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        // LOG_SYSERR << "SO_REUSEPORT failed.";
    }
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                &optval, static_cast<socklen_t>(sizeof optval));
}
