cmake_minimum_required (VERSION 2.8)

set(http_SRCS
    HttpServer.cpp
    HttpResponse.cpp
    HttpContext.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/WebServer/net)

add_library(http ${http_SRCS})
target_link_libraries(http net)

install(TARGETS http DESTINATION lib)

add_executable(httpserver_test http_test.cpp)
target_link_libraries(httpserver_test http)