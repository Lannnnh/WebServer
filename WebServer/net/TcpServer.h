#ifndef _NET_TCPSERVER_H
#define _NET_TCPSERVER_H

#include "WebServer/base/nocopyable.h"
#include "WebServer/base/type.h"
#include "WebServer/base/Atomic.h"
#include "WebServer/net/TcpConnection.h"
#include "Callbacks.h"

#include <map>
#include <netinet/in.h>

class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class TcpServer : nocopyable
{
    public:
        typedef std::function<void (EventLoop*)> ThreadInitCallback;
        enum Option
        {
            kNoReusePost,
            kReusePort,
        };

        TcpServer(EventLoop* loop,
                  const struct sockaddr_in& listenAddr,
                  const std::string& name,
                  Option option = kNoReusePost);
        ~TcpServer(); // force out-line dtor, for unqiue_ptr members.

        const std::string name() const { return name_; }
        EventLoop* getLoop() const { return loop_; }

        // set the number of threads for handling input.
        // always accepts new connection in loop's thread.
        // 0 means all I/O in loop's thread, no thread will created.
        // this is the default value.
        // 1 means all I/O in another thread.
        // N means a thread pool with N threads, new connection
        // are assigned on a round-robin basis.
        void setThreadNum(int numThreads);
        void setThreadInitCallback(const ThreadInitCallback &cb)
        { threadInitCallback_ = cb; }
        // valid after calling start()
        std::shared_ptr<EventLoopThreadPool> threadPool()
        { return threadPool_; }

        // start the server if it's not listenning.
        // it's harmless to call it mutiple times.
        // thread safe
        void start();

        // not thread safe.
        void setConnectionCallback(const ConnectionCallback& cb)
        { connectionCallback_ = cb; }
        //not thread safe
        void setMessageCallback(const MessageCallback& cb)
        { messageCallback_ = cb; }
        // not thread safe
        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        { writeCompleteCallback_ = cb; }

    private:
        // not thread safe, but in loop
        void newConnection(int sockfd, const struct sockaddr_in& peeraddr);
        // thread safe
        void removeConnection(const TcpConnectionPtr& conn);
        // not thread safe, but in loop
        void removeConnectionInLoop(const TcpConnectionPtr& conn);

        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

        EventLoop* loop_; // the acceptor loop
        const std::string name_;
        const std::string ipPort_;
        std::unique_ptr<Acceptor> acceptor_;
        std::shared_ptr<EventLoopThreadPool> threadPool_;
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