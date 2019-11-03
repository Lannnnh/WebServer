// #include "Socket.h"
// #include <string.h>
// #include <errno.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <netinet/tcp.h>

// typedef struct sockaddr SA;
// const int LISTENQ = 2048;

// // 往一个描述符fd写n个字节
// ssize_t writen(int fd, const void *vptr, size_t n)
// {
//     size_t nleft;
//     ssize_t nwritten;
//     const char *ptr;

//     ptr = (char *) vptr;
//     nleft = n;
//     while (nleft > 0)
//     {
//         if ((nwritten = write(fd, ptr, nleft)) <= 0)
//         {
//             if (nwritten < 0 && errno == EINTR)
//                 nwritten = 0;
//             else return -1;
//         }
//         nleft -= nwritten;
//         ptr += nwritten;
//     }

//     return n;
// }

// // 往一个描述符fd写一个string
// ssize_t writen(int fd, std::string &vptr)
// {
//     size_t nleft;
//     ssize_t nwritten;
//     ssize_t writesize = 0;
//     const char *ptr = vptr.c_str();

//     nleft = vptr.size();
//     while (nleft > 0)
//     {
//         if ((nwritten = write(fd, ptr, nleft)) <= 0)
//         {
//             if (nwritten < 0 && errno == EINTR)
//                 nwritten = 0;
//             else return -1;
//         }

//         nleft -= nwritten;
//         ptr += nwritten;
//         writesize += nwritten;
//     }

//     return writesize;
// }

// // 从一个描述符fd读n个字节
// ssize_t readn(int fd, const void *vptr, size_t n)
// {
//     size_t nleft;
//     ssize_t nread;
//     char *ptr;
//     ptr = (char *) vptr;
//     nleft = n;

//     while (nleft > 0)
//     {
//         if ((nread = read(fd, ptr, nleft)) < 0)
//         {
//             if (errno == EINTR)
//                 nread = 0;
//             else return -1;
//         }
//         else if (nread == 0) break;    // EOF
//         nleft -= nread;
//         ptr += nread;
//     }

//     return n - nleft;
// }

// // 从一个描述符fd读一个string
// ssize_t readn(int fd, std::string &vptr)
// {
//     size_t nleft = vptr.size();
//     ssize_t nread;
//     ssize_t readsize = 0;
//     char *ptr = (char *) vptr.c_str();

//     while (nleft > 0)
//     {
//         if ((nread = read(fd, ptr, nleft)) < 0)
//         {
//             if (errno == EINTR)
//                 nread = 0;
//             else return -1;
//         }
//         else if (nread == 0) break;

//         nleft -= nread;
//         ptr += nread;
//         readsize += nread;
//     }

//     return readsize;
// }

// // 创建socket套接字 绑定端口 监听
// int socket_bind_listen(int port)
// {
//     // 检查port在可用区间内
//     if (port < 0 || port > 65535) return -1;

//     // 创建socket(IPV4 — TCP)， listenfd为监听描述符
//     int listenfd = 0;

//     if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;

//     // 消除bind时因为time_wait引起的“Address already in use”错误
//     int optval = 1;
//     if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) return -1;

//     // 绑定服务器IP和port
//     struct sockaddr_in servaddr;

//     bzero(&servaddr, sizeof(servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_addr.s_addr =  htonl(INADDR_ANY);
//     servaddr.sin_port = htons(port);
//     if ((bind(listenfd, (SA *) &servaddr, sizeof(servaddr))) == -1) return -1;

//     // 开始监听描述符， 最大等待队列长度为SIZE
//     if ((listen(listenfd, LISTENQ))) return -1;

//     // 无效的监听描述符
//     if (listenfd == -1)
//     {
//         close(listenfd);
//         return -1;
//     }
    
//     // 返回监听描述符
//     return listenfd;
// }

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

std::string toIpPort(const struct ::sockaddr_in *addr)
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

void sockets::bindOrDie(int sockfd, const struct ::sockaddr *addr)
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

    int connfd = ::accept4(sockfd, static_cast<struct ::sockaddr*>(implicit_cast<void*> (addr)),
                            &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd < 0)
    {
        //LOG_SYSERR << "Socket::accept";
    }
    
    return connfd;
}

int sockets::connect(int sockfd, const struct ::sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

ssize_t sockets::read(int sockfd, void *buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct ::iovec *iov, int iovcnt)
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

int Socket::accept(struct ::sockaddr_in *peeraddr)
{
    struct ::sockaddr_in addr;
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
