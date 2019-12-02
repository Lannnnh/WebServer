#ifndef _BASE_THREAD_H
#define _BASE_THREAD_H

#include <sys/types.h>
#include "nocopyable.h"
#include "CountDownLatch.h"
#include <functional>
#include <pthread.h>
#include <string>
#include <memory>

class Thread : nocopyable
{
    public:
        typedef std::function<void ()> ThreadFunc;

        explicit Thread(const ThreadFunc&, const std::string& name = std::string());
        ~Thread();
        void start();
        int join();
        bool started() const { return started_; }
        pid_t tid() { return tid_; }
        const std::string& name() const { return name_; }

    private:
        void setDefaultName();

        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        pid_t tid_;
        ThreadFunc func_;
        std::string name_;
        CountDownLatch latch_;
};

#endif // end _BASE_THREAD_H