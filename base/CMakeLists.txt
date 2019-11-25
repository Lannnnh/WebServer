set(base_SRCS
    AsyncLogging.cpp
    Condition.cpp
    CountDownLatch.cpp
    CurrentThread.cpp
    FileUtil.cpp
    LogFile.cpp
    Logging.cpp
    LogStream.cpp
    TimeStamp.cpp
    Thread.cpp
)

add_library(base ${base_SRCS})
target_link_libraries(base pthread rt)