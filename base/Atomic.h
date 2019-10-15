#ifndef _BASE_ATOMIC_H
#define _BASE_ATOMIC_H

#include "nocopyable.h"
#include <stdint.h>

template<typename T>
class AtomicIntegerT : nocopyable
{
    public:
        AtomicIntegerT()
            : value_(0)
        {   }

        T get()
        {
            return __sync_val_compare_and_swap(&value_, 0, 0);
        }

        T getAndAdd(T x)
        {
            return __sync_fetch_and_add(&value_, x);
        }

        T addAndGet(T x)
        {
            return getAndAdd(x) + x;
        }

        T incrementAndGet()
        {
            return addAndGet(1);
        }

        T decrementAndGet()
        {
            return addAndGet(-1);
        }

        void add(T x)
        {
            getAndAdd(x);
        }

        void increment()
        {
            incrementAndGet();
        }

        void decrement()
        {
            decrementAndGet();
        }

        T getAndSet(T newValue)
        {
            return __sync__lock_test_and_set(&value, newValue);
        }

    private:
        volatile T value_;
};

typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;


#endif // end _BASE_ATOMIC_H