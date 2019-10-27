#ifndef _NET_CHANNEL_H
#define _NET_CHANNEL_H
/*
    功能： 将IO multiplexing拿到的IO事件分发给各个文件描述符（fd）的事件处理函数
    每个channel对象自始自终都只属于一个EventLoop，因为每个channel对象都只属于某一个IO线程。
    每个channel对象自始自终只负责一个文件描述符（fd）的IO事件分发，但它并不拥有，析构时不会关闭fd。
*/

#include "base/nocopyable.h"
#include <string>
#include <functional>

class EventLoop;

class Channel : nocopyable
{
    public:
        typedef std::function<void()> EventCallback;
        typedef std::function<void (Timestamp)> ReadEventCallback;

        Channel(EventLoop *loop, int fd);
        ~Channel();

        void handleEvent(Timestamp receiveTime);
        void setReadCallback(ReadEventCallback cb)
        { readCallback_ = std::move(cb); }
        void setWriteCallback(EventCallback cb)
        { writeCallback_ = std::move(cb); }
        void setErrorCallback(EventCallback cb)
        { errorCallback_ = std::move(cb); }
        void setCloseCallback(EventCallback cb)
        { closeCallback_ = std::move(cb); }

        // tie this channel to the owner object managed by shared_ptr
        // prevent the owner object being destroyed in handleEvnet
        void tie(const std::shared_ptr<void> &);

        int fd() const { return fd_; }
        int events() const { return event_; }
        void set_revents(int revt) { revents_ = revt; }
        bool isNoneEvent() const { return event_ == kNoneEvent; }

        void enableReading() { event_ |= kReadEvent; update(); }
        void enableWriting() { event_ |= kWriteEvent; update(); }
        void disableAll() { event_ = kNoneEvent; update(); }
        void disableWriting() { event_ &= ~kWriteEvent; update(); }
        void disableReading() { event_ &= ~kReadEvent; update(); }
        bool isReading() const { return event_ & kReadEvent; }
        bool isWriting() const { return event_ & kWriteEvent; }
        
        //for poller
        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }

        void doNotLogHup() { logHup_ = false; }

        EventLoop* ownerLoop() { return loop_; }
        void remove();
 
    private:
        void update();
        void handleEventWithGuard(Timestamp receiveTime);

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;
        int event_;
        int revents_;
        int index_;
        bool logHup_;

        std::weak_ptr<void> tie_;
        bool tied_;
        bool eventHandling_;
        bool addedToLoop_;

        ReadEventCallback readCallback_;
        EventCallback closeCallback_;
        EventCallback errorCallback_;
        EventCallback writeCallback_;
};



#endif // end _NET_CHANNEL_H