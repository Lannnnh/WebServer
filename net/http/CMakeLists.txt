set(http_SRCS
    HttpServer.cpp
    HttpResponse.cpp
    HttpContext.cpp
)

add_library(http ${http_SRCS})
target_link_libraries(http net)

add_execultable(httpserver_test http_test.cpp)
target_link_libraries(httpserver_test http)