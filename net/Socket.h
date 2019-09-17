#ifndef _NET_SOCKET_H_
#define _NET_SOCKET_H_

#include <string>

int socket_bind_listen(int port);
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t writen(int fd, std::string &vptr);
ssize_t readn(int fd, const void *vptr, size_t n);
ssize_t readn(int fd, std::string &vptr);

#endif // end _NET_SOCKET_H_