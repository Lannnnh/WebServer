#include <iostream>
#include "net/Socket.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef struct sockaddr SA;
const int SIZE = 2048;

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
    if ((listen(listenfd, SIZE))) return -1;

    // 无效的监听描述符
    if (listenfd == -1) 
    {
        close(listenfd);
        return -1;
    }
    
    // 返回监听描述符
    return listenfd;
}