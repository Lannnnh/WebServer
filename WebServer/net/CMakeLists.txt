cmake_minimum_required (VERSION 2.8)

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
    TcpClient.cpp
    TcpConnection.cpp
    TcpServer.cpp
    Timer.cpp
    TimerQueue.cpp
)

include_directories(/home/lan/beap)

add_library(net ${net_SRCS})
target_link_libraries(net /home/lan/beap/WebServer/base/libbase.a)

add_subdirectory(http)