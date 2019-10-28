#ifndef _NET_SOCKET_H_
#define _NET_SOCKET_H_

#include "base/nocopyable.h"

#include <string>

// int socket_bind_listen(int port);
// ssize_t writen(int fd, const void *vptr, size_t n);
// ssize_t writen(int fd, std::string &vptr);
// ssize_t readn(int fd, const void *vptr, size_t n);
// ssize_t readn(int fd, std::string &vptr);

namespace sockets
{
    int createNonblockingOrDie(sa_family_t family);

    int  connect(int sockfd, const struct sockaddr* addr);
    void bindOrDie(int sockfd, const struct sockaddr* addr);
    void listenOrDie(int sockfd);
    int  accept(int sockfd, struct sockaddr_in6* addr);
    ssize_t read(int sockfd, void *buf, size_t count);
    ssize_t readv(int sockfd, const struct ::iovec *iov, int iovcnt);
    ssize_t write(int sockfd, const void *buf, size_t count);
    void close(int sockfd);
    void shutdownWrite(int sockfd);
    int getSocketError(int sockfd);
}

class Socket : nocopyable
{
    public:
        explicit Socket(int sockfd)
            : sockfd_(sockfd)
        {   }

        ~Socket();

        int fd() { return sockfd_; }
        void bindAddress(const struct sockaddr_in *localaddr);
        void listen();
        int accept(struct sockaddr_in *peeraddr);

        void shutdownWrite();
        void setTcpNoDelay(bool on);
        void setReuseAddr(bool on);
        void setReusePort(bool on);
        void setKeepAlive(bool on);

    private:
        const int sockfd_;
};

#endif // end _NET_SOCKET_H_