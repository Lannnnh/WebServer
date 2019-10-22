#ifndef _NET_ACCEPTOR_H
#define _NET_ACCEPTOR_H

#include "Socket.h"
#include "Channel.h"
#include "base/nocopyable.h"

#include <functional>
#include <netinet/in.h>

class EventLoop;

class Acceptor : nocopyable
{
    public:
        typedef std::function<void (int sockfd, const struct sockaddr_in*)> NewConnectionCalback;
        Acceptor(EventLoop *loop, const struct sockaddr_in *listenaddr, bool reuseport);
        ~Acceptor();

        void setNewConnectionCallback(const NewConnectionCalback &cb)
        {
            newConnectionCallback_ = cb;
        }

        bool listenning() const { return listenning_; }
        void listen();

    private:
        void hanldeRead();

        EventLoop *loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCalback newConnectionCallback_;
        bool listenning_;
        int idleFd_;
};

#endif // end _NET_ACCEPTOR_H