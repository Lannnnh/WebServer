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
        typedef std::function<void (EventLoop*)> ThreadInitCallback;
        enum Option
        {
            kNoReusePost,
            kReusePort,
        };

        TcpServer();
        ~TcpServer();

    private:
        // not thread safe, but in loop
        void newConnection(int sockfd, const struct ::sockaddr_in *peeraddr);

        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

        EventLoop *loop_; // the acceptor loop
        const std::string name_;
        const std::string ipPort_;
        std::unique_ptr<Acceptor> acceptor_;
        //std::unique_ptr<EventLoopThreadPool> threadPool_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        ThreadInitCallback threadInitCallback_;
        AtomicInt32 started_;
        // always in loop thread
        int nextConnId_;
        ConnectionMap connections_;
};

#endif // end _NET_TCPSERVER_H