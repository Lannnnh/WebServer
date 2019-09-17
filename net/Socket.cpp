#include "Socket.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

typedef struct sockaddr SA;
const int LISTENQ = 2048;

// 往一个描述符fd写n个字节
ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = (char *) vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return n;
}

// 往一个描述符fd写一个string
ssize_t writen(int fd, std::string &vptr)
{
    size_t nleft;
    ssize_t nwritten;
    ssize_t writesize = 0;
    const char *ptr = vptr.c_str();

    nleft = vptr.size();
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else return -1;
        }

        nleft -= nwritten;
        ptr += nwritten;
        writesize += nwritten;
    }

    return writesize;
}

// 从一个描述符fd读n个字节
ssize_t readn(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;
    ptr = (char *) vptr;
    nleft = n;

    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
                nread = 0;
            else return -1;
        }
        else if (nread == 0) break;    // EOF
        nleft -= nread;
        ptr += nread;
    }

    return n - nleft;
}

// 从一个描述符fd读一个string
ssize_t readn(int fd, std::string &vptr)
{
    size_t nleft = vptr.size();
    ssize_t nread;
    ssize_t readsize = 0;
    char *ptr = (char *) vptr.c_str();

    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR)
                nread = 0;
            else return -1;
        }
        else if (nread == 0) break;

        nleft -= nread;
        ptr += nread;
        readsize += nread;
    }

    return readsize;
}

// 创建socket套接字 绑定端口 监听
int socket_bind_listen(int port)
{
    // 检查port在可用区间内
    if (port < 0 || port > 65535) return -1;

    // 创建socket(IPV4 — TCP)， listenfd为监听描述符
    int listenfd = 0;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;

    // 消除bind时因为time_wait引起的“Address already in use”错误
    int optval = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) return -1;

    // 绑定服务器IP和port
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr =  htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if ((bind(listenfd, (SA *) &servaddr, sizeof(servaddr))) == -1) return -1;

    // 开始监听描述符， 最大等待队列长度为SIZE
    if ((listen(listenfd, LISTENQ))) return -1;

    // 无效的监听描述符
    if (listenfd == -1)
    {
        close(listenfd);
        return -1;
    }
    
    // 返回监听描述符
    return listenfd;
}