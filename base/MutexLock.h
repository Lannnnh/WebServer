#ifndef _BASE_MUTEXLCOK_H
#define _BASE_MUTEXLCOK_H

#include "nocopyable.h"
#include <pthread.h>

class MutexLock : nocopyable
{
    public:
        MutexLock()
        {
            pthread_mutex_init(&mutex, NULL);
        }

        ~MutexLock()
        {
            pthread_mutex_lock(&mutex);
            pthread_mutex_destroy(&mutex);
        }

        void lock()
        {
            pthread_mutex_lock(&mutex);
        }

        void unlock()
        {
            pthread_mutex_unlock(&mutex);
        }
        
        pthread_mutex_t *get()
        {
            return &mutex;
        }

    private:
        pthread_mutex_t mutex;

    //友元类
    private:
        friend class Condition;
};

class MutexLockGuard : nocopyable
{
    public: 
        explicit MutexLockGuard(MutexLock &_mutex) 
            : mutex(_mutex)
        {
            mutex.lock();
        }

        ~MutexLockGuard()
        {
            mutex.unlock();
        }

    private:
        MutexLock &mutex;
};

#endif // _BASE_MUTEXLCOK_H