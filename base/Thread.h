#ifndef _BASE_THREAD_H
#define _BASE_THREAD_H

#include <sys/types.h>
#include "nocopyable.h"
#include <functional>
#include <pthread.h>
#include <string>
#include <memory>

class Thread : nocopyable
{
    public:
        typedef std::function<void ()> ThreadFunc;

        //explicit Thread(ThreadFunc, const std::string& name = string());

    private:
        void setDefaultName();

        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        pid_t tid_;
        ThreadFunc func_;
        std::string name_;
};

#endif // end _BASE_THREAD_H