#ifndef _NET_TCPCONNECTION_H
#define _NET_TCPCONNECTION_H

#include "base/nocopyable.h"
#include "base/type.h"
#include "Callbacks.h"
#include "Buffer.h"

#include <memory>
#include <netinet/in.h>
#include <boost/any.hpp>

class EventLoop;
class Socket;
class Channel;


// enable_shared_from_this是一个以其派生类模板类型实参的基类模板，继承它，this指针就能变身为shared_ptr。
// 注意，shared_from_this()不能在构造函数里面使用，因为那个时候this还没有被交给shared_ptr接管。
class TcpConnection : nocopyable, public std::enable_shared_from_this<TcpConnection>
{
    public:
        TcpConnection(EventLoop *loop,
                      const std::string &name,
                      int sockfd,
                      const struct sockaddr_in &lockAddr,
                      const struct sockaddr_in &peerAddr);
        ~TcpConnection();

        EventLoop* getLoop() const { return loop_; }
        const std::string& name() { return name_; }
        const struct sockaddr_in& localAddr() const { return localAddr_; }
        const struct sockaddr_in& peerAddr() const { return peerAddr_; }
        bool connected() const { return state_ == kConnected; }
        bool disconnected() const { state_ == kDisconnected; }
        // send
        void send(const void *messeage, int len);
        void send(const std::string& str);
        void send(Buffer *message); // this one will swap data
        void shutdown(); // not thread safe, no simitaneous calling
        void forceClose();
        void forceCloseWithDelay(double seconds);
        void setTcpNoDelay(bool on);
        // reading or not
        void startRead();
        void stopRead();
        bool isReading() const { return reading_; }

        void setContext(const boost::any& context)
        {
            context_ = context;
        }

        const boost::any& getContext() const
        {
            return context_;
        }

        boost::any* getMutableContext()
        {
            return &context_;
        }

        void setConnectionCallback(const ConnectionCallback &cb)
        { connectionCallback_ = cb; }

        void setMessageCallback(const MessageCallback &cb)
        { messageCallback_ = cb; }

        void setWriteCompleteCallback(const WriteCompleteCallback &cb)
        { writeCompleteCallback_ = cb; }

        void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
        { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

        // advanced interface
        Buffer* inputBuffer()
        { return &inputBuffer_; }

        Buffer* outputBuffer()
        { return &outputBuffer_; }

        // internal use only
        void setCloseCallback(const CloseCallback &cb)
        { closeCallback_ = cb; }

        // called when TcpServer accepts a new connection
        void connectEstablished(); // should be called only once
        // called when TcpServer has removed me from its map
        void connectDestroyed(); // should be called only once


    private:
        enum stateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();

        void sendInLoop(const void *message, size_t len);
        void shutdownInLoop();
        void forceCloseInLoop();

        void setState(stateE s) {}
        const char* stateToString() const;
        void startReadInLoop();
        void stopReadInLoop();

        EventLoop *loop_;
        std::string name_;
        stateE state_;
        bool reading_;
        // we don't expose those classes to client.
        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;
        struct sockaddr_in localAddr_;
        struct sockaddr_in peerAddr_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_; 
        WriteCompleteCallback writeCompleteCallback_;
        HighWaterMarkCallback highWaterMarkCallback_;
        CloseCallback closeCallback_;
        size_t highWaterMark_;
        Buffer inputBuffer_;
        Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
        boost::any context_;
};

#endif // end _NET_TCPCONNECTION_H