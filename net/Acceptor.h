#ifndef _NET_ACCEPTOR_H
#define _NET_ACCEPTOR_H

#include "Socket.h"
#include "Channel.h"
#include "base/nocopyable.h"

#include <functional>
#include <netinet/in.h>

class EventLoop;
/*
    muduo中限制并发连接数：
        为它增加一个int成员（muduo::AtomicInt32类型），表示当前连接数。然后判断当前活动数。
        如果超过最大允许数，则踢掉连接。可以积极地防止耗尽fd。
*/
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