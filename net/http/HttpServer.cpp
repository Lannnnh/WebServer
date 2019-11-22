#include "HttpServer.h"
#include "base/Logging.h"
#include "http/HttpContext.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop,
                       const struct sockaddr_in listenAddr,
                       const std::string& name,
                       TcpServer::Option option)    
    : server_(loop, listenAddr, name, option),
      httpCallback_(::defaultHttpCallback)
{
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::start()
{
    LOG << "HttpServer[" << server_.name()
        << "] starts listenning on " << server_.ipPort();
    server_.start();
}

// need continue