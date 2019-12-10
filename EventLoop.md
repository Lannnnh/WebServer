# EventLoop设计与实现

​	在项目中我们使用的是**“non-blocking IO+one loop one thread”**模式来编写多线程C++网络服务程序的。在这种模型下，程序里的每一个IO线程都有一个event loop（或者叫Reactor），用于处理读写和（单次或者周期性）定时事件。

​	在“one loop one thread”这种模型下，EventLoop代表了线程的主循环，需要让哪个线程工作，就把timer或者IO channel注册到哪个线程的loop里面就可以了。

​		