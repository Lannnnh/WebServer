#ifndef _BASE_COUNTDOWNLATCH_H
#define _BASE_COUNTDOWNLATCH_H

#include "nocopyable.h"
#include "Condition.h"
#include "MutexLock.h"

class CountDownLatch : nocopyable 
{
    public:
        explicit CountDownLatch(int count);
        void wait();
        void countDown();

    private:
        mutable MutexLock mutex_;
        Condition condition_;
        int count_;
}; 

#endif // end _BASE_COUNTDOWNLATCH_H