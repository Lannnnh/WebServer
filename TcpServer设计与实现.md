# TcpServer设计与实现

1.  EventLoop

	Reactor模式下最核心的是事件分发机制，整个框架最基本的一个模块就是EventLoop，EventLoop控制着该IO线程的事件分发。

	1. EventLoop在大部分时候都循环在EventLoop::loop()循环里面监听它的每一个文件描述符(fd)，每一个fd由不同的Channel负责，而IO multiplexing由EventLoop持有的Poller成员负责。
	
	2. EventLoop数据成员里包含一个wakeupFd，它是一个eventfd可以用来完成线程间通信，因为IO线程平时阻塞在Eventloop::loop()循环里，为了IO线程能够立即执行用户回调，用eventfd来唤醒它。在Eventloop构造函数初始化的时候enableReading()。
	3. EventLoop在处理待处理回调函数(PendingFunctors)时，需要把待回调函数swap给一块临时变量，这样有利于控制待回调函数数组长度，并且避免待回调函数中调用queueInLoop带来死锁的麻烦。
	
	eg: 以前唤醒线程是通过建立一个管道pipe，IO始终监视这个pipe，需要唤醒的时候其他线程往pipe里写入一个字节。现在有了eventfd之后可以更加高效的唤醒，不用管理缓冲区。
	
2.  TimerQueue定时器

	TimerQueue定时器是为了给Eventloop的回调函数runInLoop加上定时功能而存在的，这个模块主要由三个class实现（TimerId、Timer、TimerQueue），TimerQueue只能在所属的IO线程调用，所以不必加锁。
	
	1. Timer Class是定时器最基础的一个模块，它包含了定时器所需要的数据成员（回调函数、过期时间、间隔时间，序列号等等）。
	
	eg： 每一个Timer的序列号sequence都是由静态数据成员s_numCreated通过原子操作递增而来。
	
	2. TimerId Class简单封装了Timer Class数据成员包含一个Timer Class和自己的序列号。

	3. TimerQueue Class只提供addTimer()和cancel()两个接口给Eventloop。TimerQueue通过二叉搜索树std::set来管理定时器集合timers，有效的将线性查找$O(N)$的时间复杂度降低到了$O(logN)$级别。addTimer()创建(new)一个新的定时器Timer，并注册回调。在addTimerInLoop中把新定时器加入到定时器集合timers中，如果新定时器的超时时间早于所有已存在的定时器，那么要更新timerfd的更新时间。cancel()把一个要删除的Timer从定时器集合timers中删除，如果不在当前活跃的定时器集合里，正好又在处理过期定时器期间，那么需要把这个Timer加入取消的定时器集合。（解释如下）
	4. cancelingTimers标志存在的意义是防止headRead在处理过期Timer的时候，我要取消的Timer恰好在getExpired中被删除，但是headRead最后会重置过期Timer中时间间隔不为0的定时器，这个时候被删除的Timer在过期Timer中的话不会重置它。

3.  EventLoopThread(Pool)

    1.  EventLoopThread是one loop one thread的一种体现，它就是一个IO线程，会启动自己的线程，并在其中运行loop()。startLoop()函数来运行一个新线程，并且返回这个线程上EventLoop的地址。

    2.  EventLoopThreadPool是多线程Server的关键，它是一个线程池，会在新建TcpConnection的时候从池子里选择一个EventLoopThread出来分配给这个连接来执行IO。目前对线程池的调度采用round-robin轮询调度算法。

4.  Accpetor

    Acceptor Class完成的任务很简单就是accept新的TCP连接，并且通过回调通知调用它的EventLoop。最主要的两个函数就是listen和hanldeRead，listen开始监听socket套接字，并且让acceptChannel可读，当有连接来到唤醒hanldeRead函数建立连接并且处理连接。
    eg： 考虑到如果我们在Poller里面使用Epoll来做IO多路复用并且使用LT模式时，如果超过了最大连接数没有更多的fd可用，但accept队列里还一直增加新的连接等你接受，那么会一直触发读事件（因为你一直都不读它，也没法读它），这样就会造成busy loop。所以我们创建了一个空洞文件描述符idleFd，如果达到了最大连接数，我们首先关闭我们开始创建的空洞文件fd，这样我们就有多余的fd去接受新连接，然后把接受到的新连接关闭，再创建一个空洞文件fd，就一直靠这个空洞文件fd来维持不超过最大连接数。

5. TcpServer

    TcpServer拥有一个Accept来获得新的连接TcpConnection，它负责管理这些连接。

    1.  连接的建立：每当Accetpor接收到了新连接，它会回调newConnection函数创建一个TcpConnection对象，然后把对象加入ConnectionMap，最后调用connectEstablished()完成这整个过程。
    2.  连接的断开：TcpConnection会在自己的ioLoop调用removeConnection，所以需要把它移到TcpServer线程中来，把它从ConnectionMap删除，最后再回到自己的ioLoop中connectDestroyed()。

6. Buffer

    非阻塞IO是为了避免阻塞在read和write等IO调用上，让一个线程可以服务多个socket。那么TcpConnection拥有自己的缓冲区就变得有必要了，因为没有自己的缓冲区需要担心一次read或write是否一次性操作完了所有数据，如果系统没有读（写）完，那么便会出现数据丢失。TcpConnection在拥有了自己的缓冲区之后，read或write操作的对象就变成了Input Buffer和Output Buffer，将数据放在缓冲区里保证数据的完整性。

    eg: Input Buffer使用了一块临时栈上空间，避免了Buffer设置的初始值过大而浪费空间，也通过readv散步读来避免了重复调用read。

7. TcpConnection

    TcpConnection是一个短暂的对象，只要客户端断开了连接，对应的服务端中的TcpConnection对象也将结束，但是不能直接delete，可能在其他地方还持有它的引用。而且旧的连接如果断开直接销毁，另一个客户端新连接的socket fd等于之前断开的连接fd就会发生串话，所以TcpConnection用shared_ptr来管理Class，并继承enable_shared_this。

    1. TcpConnection代表一次“TCP连接”，对象创建的时候就已经创建了连接，因此其初始状态为kConnecting，等待connectEstablished()完成连接的建立（只应该调用一次）。
    2. TcpConnection发送数据比接收数据要来的复杂，当send(msg)的时候，先考虑OutputBuffer里面的数据发送完了没，如果没有发送完毕先通过handleWrite把OutputBuffer缓冲区的数据送出去，然后再write数据，这样保证了数据的发送顺序。