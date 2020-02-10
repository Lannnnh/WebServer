#include "Acceptor.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Logging.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

Acceptor::Acceptor(EventLoop* loop, const struct sockaddr_in& listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie(AF_INET)),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{   
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::hanldeRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::hanldeRead()
{
    loop_->assertInLoopThread();
    struct sockaddr_in peeraddr;
    int connfd = acceptSocket_.accept(peeraddr);

    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peeraddr);
        }
        else        
        {
            sockets::close(connfd);
        }
    }
    else
    {
        LOG << "in Acceptor::handleRead";
        /*
            错误类型EMFILE:超过最大连接数
                当超过最大连接数时，而accept队列里可能还一直在增加新的连接等你接受，并前muduo用的是epoll的LT模式时，
            那么因为你连接达到了文件描述符的上限，此时没有可供你保存新连接套接字描述符的文件符了，那么新来的连接
            就会一直放在accept队列中，于是呼其对应的可读事件就会一直触发读事件(因为你一直不读，也没办法读走它)，
            此时就会造成我们常说的busy loop。

            解决方案：
                如果当fd超过了最大连接数，我们首先关闭我们开始创建的空洞文件fd，这样我们就有多余的fd去接受新连接，
            然后把接受到的新连接关闭，再创建一个空洞文件fd，就一直靠这个空洞文件fd来维持不超过最大连接数。
        */
        if (errno == EMFILE)
        {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}