#ifndef _NET_TCPCLIENT_H
#define _NET_TCPCLIENT_H

#include "base/MutexLock.h"
#include "TcpConnection.h"
#include "base/nocopyable.h"

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : nocopyable
{
    public:
        TcpClient(EventLoop *loop,
                  const ::sockaddr_in &serverAddr,
                  const std::string &name);
        ~TcpClient(); // force out-line dtor, for std::unique_ptr membres.

        void connect();
        void disconnect();
        void stop();

        TcpConnectionPtr connection() const
        {
            MutexLockGuard lock(mutex_);
            return connection_;
        }

        EventLoop* getLoop() const { return loop_; }
        bool retry() const { return retry_; }
        void enableRetry() { retry_ = true; }

        const std::string& name() const { return name_; }

        // set message callback.
        // not thread safe
        void setMessageCallback(MessageCallback cb)
        { messageCallback_ = std::move(cb); }

        // set connection callback.
        // not thread safe
        void setConnectionCallback(ConnectionCallback cb)
        { connectionCallback_ = std::move(cb); }

        // set write complete callback.
        // not thread safe
        void setWriteCompleteCallback(WriteCompleteCallback cb)
        { writeCompleteCallback_ = std::move(cb); }

    private:
        // not thread safe, but in loop
        void newConnection();
        // not thread safe, but in loop
        void removeConnection(const TcpConnectionPtr &conn);

        EventLoop *loop_;
        ConnectorPtr connector_; // avoid revealing Connector
        const std::string name_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        bool retry_; // atomic
        bool connect_; // atomic
        // always in loop thread
        int nextConnId_;
        mutable MutexLock mutex_;
        TcpConnectionPtr connection_;

};


#endif // end _NET_TCPCLIENT_H