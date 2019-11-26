cmake_minimum_required (VERSION 2.8)

set(http_SRCS
    HttpServer.cpp
    HttpResponse.cpp
    HttpContext.cpp
)

add_library(http ${http_SRCS})
target_link_libraries(http net)

install(TARGETS http DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/WebServer/net/http)

add_executable(httpserver_test http_test.cpp)
target_link_libraries(httpserver_test http)