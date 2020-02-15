cmake_minimum_required (VERSION 2.8)

set(net_SRCS
    Acceptor.cpp
    Buffer.cpp
    Channel.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    Poller.cpp
    Socket.cpp
    TcpConnection.cpp
    TcpServer.cpp
    Timer.cpp
    TimerQueue.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/WebServer/base)

add_library(net ${net_SRCS})
target_link_libraries(net ${PROJECT_SOURCE_DIR}/WebServer/base/libbase.a)

add_subdirectory(http)