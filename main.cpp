#include "net/Socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

typedef struct sockaddr SA;

int main(int agrc, char* agrv[])
{
    char buff[2048];
    int listenfd = socket_bind_listen(13), connfd = 0;
    struct sockaddr_in cliaddr;
    time_t ticks;

    socklen_t len;

    for ( ; ; )
    {
        len = sizeof(cliaddr);
        connfd = accept(listenfd, (SA *) &cliaddr, &len);

        printf("connection from %s, port %d\n",
        inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));

        close(connfd);
    }

    return 0;
}