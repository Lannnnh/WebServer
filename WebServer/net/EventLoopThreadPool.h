#ifndef _NET_EVENTLOOPTHREADPOOL_H
#define _NET_EVENTLOOPTHREADPOOL_H

#include "nocopyable.h"
#include "type.h"

#include <functional>
#include <memory>
#include <vector>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : nocopyable
{
    public:
        typedef std::function<void (EventLoop*)> ThreadInitCallback;

        EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
        ~EventLoopThreadPool();
        void setThreadNum(int numThreads) { numThreads_ = numThreads; }
        void start(const ThreadInitCallback& cb = ThreadInitCallback());

        // valid after calling start()
        // round-robin 轮询调度算法
        EventLoop* getNextLoop();
        
        // with the same hash code, it will always return the same EventLoop
        EventLoop* getLoopForHash(size_t hashCode);

        std::vector<EventLoop*> getAllLoops();

        bool started() const { return started_; }
        const std::string& name() const { return name_; }

    private:
        EventLoop* baseLoop_;
        std::string name_;
        bool started_;
        int numThreads_;
        int next_; // always in loop thread
        std::vector<std::unique_ptr<EventLoopThread>> threads;
        std::vector<EventLoop*> loops_;

};

#endif // end _NET_EVENTLOOPTHREADPOOL_H