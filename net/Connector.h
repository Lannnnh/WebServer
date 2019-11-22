#ifndef _NET_CONNECTOR_H
#define _NET_CONNECTOR_H

#include "base/nocopyable.h"

#include <functional>
#include <memory>
#include <netinet/in.h>

class EventLoop;
class Channel;

class Connector : nocopyable,
                  public std::enable_shared_from_this<Connector>
{
    public:
        typedef std::function<void (int sockfd)> NewConnectionCallback;

        Connector(EventLoop *loop, const struct sockaddr_in &serverAddr);
        ~Connector();

        void setNewConnectionCallback(const NewConnectionCallback &cb)
        { newConnectionCallback_ = cb; }

        void start(); // can be called in any thread
        void restart(); // must be called in loop thread
        void stop(); // can be called in any thread

        const struct sockaddr_in& serverAddress() const { return serverAddr_; }

    private:
        enum States { kDisconnected, kConnecting, kConnected};
        static const int kMaxRetryDelayMs = 30*1000;
        static const int kInitRetryDelayMs = 500;

        void setState(States s) { state_ = s; }
        void startInLoop();
        void stopInLoop();
        void connect();
        void connecting(int sockfd);
        void handleWrite();
        void handleError();
        void retry(int sockfd);
        int removeAndResetChannel();
        void resetChannel();

        EventLoop *loop_;
        struct sockaddr_in serverAddr_;
        bool connect_;
        States state_;
        std::unique_ptr<Channel> channel_;
        NewConnectionCallback newConnectionCallback_;
        int retryDelayMs_;
};

#endif // end _NET_CONNECTOR_H