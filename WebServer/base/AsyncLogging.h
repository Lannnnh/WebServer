#ifndef _BASE_ASYNCLOGGING_H
#define _BASE_ASYNCLOGGING_H

#include "CountDownLatch.h"
#include "MutexLock.h"
#include "Thread.h"
#include "LogStream.h"
#include "nocopyable.h"

#include <functional>
#include <string>
#include <vector>

class AsyncLogging : nocopyable
{
    public:
        AsyncLogging(const std::string basename, int flushInterval = 2);
        ~AsyncLogging()
        {
            if (running_) stop();
        }

        void append(const char* logline, int len);

        void strat()
        {
            running_ = true;
            thread_.start();
            latch_.wait();
        }

        void stop()
        {
            running_ = false;
            cond_.notify();
            thread_.join();
        }

    private:
        void threadFunc();

        typedef FixedBuffer<kLargeBuffer> Buffer;
        typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
        typedef std::shared_ptr<Buffer> BufferPtr;

        const int flushInterval_;
        bool running_;
        std::string basename_;
        Thread thread_;
        MutexLock mutex_;
        Condition cond_;
        BufferPtr currentBuffer_;
        BufferPtr nextBuffer_;
        BufferVector buffers_;
        CountDownLatch latch_;
};

#endif // end _BASE_ASYNCLOGGING_H