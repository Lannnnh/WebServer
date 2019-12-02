#include "CountDownLatch.h"

/*
    允许一个或者多个线程等待其他线程执行到某一个操作。

    两种典型用法：
    1. 某一个线程在开始运行之前等待n个线程执行完毕。

    2. 实现多个线程开始执行任务的最大并行性，比如，在初始化一个共享的CountDownLatch对象，count为1，多个线程各自wait（），在主线程countdown（），
    那么各个线程就一起运行了。
*/

CountDownLatch::CountDownLatch(int count)
    : mutex_(),
      condition_(mutex_),
      count_(count)
{ }

void CountDownLatch::wait()
{
    MutexLockGuard lock(mutex_);
    while (count_ > 0)
        condition_.wait();
}

void CountDownLatch::countDown()
{
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0)
        condition_.notifyall();
}


