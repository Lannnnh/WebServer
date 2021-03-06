#ifndef _NET_TCPSERVER_H
#define _NET_TCPSERVER_H

#include "nocopyable.h"
#include "type.h"
#include "Atomic.h"
#include "TcpConnection.h"
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

        // 非线程安全
        void setConnectionCallback(const ConnectionCallback& cb)
        { connectionCallback_ = cb; }
        // 非线程安全
        void setMessageCallback(const MessageCallback& cb)
        { messageCallback_ = cb; }
        // 非线程安全
        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        { writeCompleteCallback_ = cb; }

    private:
        // 非线程安全
        void newConnection(int sockfd, const struct sockaddr_in& peeraddr);
        // 线程安全
        void removeConnection(const TcpConnectionPtr& conn);
        // 非线程安全
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