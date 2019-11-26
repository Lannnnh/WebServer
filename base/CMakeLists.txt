cmake_minimum_required (VERSION 3.10)

set(base_SRCS
    AsyncLogging.cpp
    CountDownLatch.cpp
    FileUtil.cpp
    LogFile.cpp
    Logging.cpp
    LogStream.cpp
    Timestamp.cpp
    Thread.cpp
)

add_library(base ${base_SRCS})
target_link_libraries(base pthread rt)

install(TARGETS base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/WebServer/base)