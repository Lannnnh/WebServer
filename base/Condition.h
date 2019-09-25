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