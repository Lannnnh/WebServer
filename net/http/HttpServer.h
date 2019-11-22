#ifndef _NET_HTTP_HTTPSERVER_H
#define _NET_HTTP_HTTPSERVER_H

#include "net/TcpServer.h"

class HttpRequest;
class HttpResponse;

class HttpServer : nocopyable
{
    public:
        typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;

        HttpServer(EventLoop* loop,
                   const struct sockaddr_in listenAddr,
                   const std::string& name,
                   TcpServer::Option option = TcpServer::kNoReusePost);

        EventLoop* getLoop() const { return server_.getLoop(); }

        // not thread safe, callback be registered before calling start()
        void setHttpCallback(const HttpCallback& cb)
        {
            httpCallback_ = cb;
        }

        void setThreadNum(int numThreads)
        {
            server_.setThreadNum(numThreads);
        }

        void start();

    private:
        void onConnection(const TcpConnectionPtr& conn);
        void onMessage(const TcpConnectionPtr& conn,
                       Buffer* buf,
                       Timestamp receiveTime);
        void onRequest(const TcpConnectionPtr&, const HttpRequest&);

        TcpServer server_;
        HttpCallback httpCallback_;
};

#endif // end _NET_HTTP_HTTPSERVER_H