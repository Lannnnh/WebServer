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
        Channel(EventLoop *loop, int fd);

        void handleEvent();
        void setReadCallback(const EventCallback& cb)
        { readCallback_ = cb; }
        void setWriteCallback(const EventCallback& cb)
        { writeCallbak_ = cb; }
        void setErrorCallback(const EventCallback& cb)
        { errorCallback_ = cb; }

        int fd() const { return fd_; }
        int events() const { return event_; }
        void set_revents(int revt) { revents_ = revt; }
        bool isNoneEvent() const { return event_ == kNoneEvent; }

        void enableReading() { event_ |= kReadEvent; update(); }
        // void enableWriting() { event_ |= kWriteEevnt; update(); }
        // void disableAll() { event_ = kNoneEvent; update(); }
        // void disableWriting() { event_ &= ~kWriteEevnt; update(); }
        
        //for poller
        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }

        EventLoop* ownerLoop() { return loop_; }
 
    private:
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;
        int event_;
        int revents_;
        int index_;

        EventCallback readCallback_;
        EventCallback errorCallback_;
        EventCallback writeCallbak_;
};



#endif // end _NET_CHANNEL_H