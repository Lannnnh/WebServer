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

add_library(net ${net_SRCS})
target_link_libraries(net base)

install(TARGETS net DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/WebServer/net)

add_subdirectory(http)