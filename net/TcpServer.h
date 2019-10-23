#ifndef _NET_TCPSERVER_H
#define _NET_TCPSERVER_H

#include "base/nocopyable.h"
#include "base/type.h"
#include "base/Atomic.h"
#include "TcpConnection.h"
#include "Callbacks.h"

#include <map>
#include <netinet/in.h>

class EventLoop;
class Acceptor;

class TcpServer : nocopyable
{
    public:
        TcpServer();
        ~TcpServer();

    private:
        // not thread safe, but in loop
        void newConnection(int sockfd, const struct ::sockaddr_in *peeraddr);

        //typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

        EventLoop *loop_;
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_;
        //ConnectionCallback connectionCallback_;
        //MessageCallback messageCallback_;
        bool started_;
        int nextConnId_;
        //ConnectionMap connections_;
};

#endif // end _NET_TCPSERVER_H