#ifndef _BASE_CONDITION_H
#define _BASE_CONDITION_H

#include "MutexLock.h"
#include "nocopyable.h"
#include <pthread.h>


class Condition : nocopyable
{
    public:
        explicit Condition(MutexLock &_mutex)
            : mutex(_mutex)
        {
            pthread_cond_init(&cond, NULL);
        }

        ~Condition()
        {
            pthread_cond_destroy(&cond);
        }

        /*
            wait中的mutex用于保护条件变量，调用这个函数进行等待的条件发生时，mutex会被自动释放，以供其他线程改变条件
            wait中的两个步骤是原子性的： 1. 把调用线程放到条件等待队列上。 2. 释放mutex
        */
        void wait()
        {
            pthread_cond_wait(&cond, mutex.get());
        }

        void notify()
        {
            pthread_cond_signal(&cond);
        }

        void notifyall()
        {
            pthread_cond_broadcast(&cond);
        }

    private:
        MutexLock &mutex;
        pthread_cond_t cond;
};

#endif // _BASE_CONDITION_H