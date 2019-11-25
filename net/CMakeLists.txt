set(net_SRCS
    Acceptor.cpp
    Buffer.cpp
    Channel.cpp
    Connector.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    Poller.cpp
    Socket.cpp
    Tcpclient.cpp
    TcpConnection.cpp
    TcpServer.cpp
    Timer.cpp
    TimerQueue.cpp
)

add_library(net ${net_SRCS})
target_link_libraries(net base)

add_subdirectory(http)