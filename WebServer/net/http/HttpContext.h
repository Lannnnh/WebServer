#ifndef _NET_HTTP_HTTPCONTEXT_H
#define _NET_HTTP_HTTPCONTEXT_H

#include "copyable.h"
#include "HttpRequest.h"

class Buffer;

class HttpContext : public copyable
{
    public:
        enum HttpRequestParseState
        {
            kExpectRequestLine,
            kExpectHeaders,
            kExpectBody,
            kGotAll
        };

        HttpContext()
            : state_(kExpectRequestLine)
        {   }

        bool parseRequest(Buffer* buf, Timestamp receiveTime);

        bool gotAll() const
        {
            return state_ == kGotAll;
        }

        void reset()
        {
            state_ = kExpectRequestLine;
            HttpRequest dummy;
            request_.swap(dummy);
        }

        const HttpRequest& request() const
        {
            return request_;
        }

        HttpRequest& request()
        {
            return request_;
        }

    private:
        bool processRequestLine(const char* begin, const char* end);

        HttpRequestParseState state_;
        HttpRequest request_;
};


#endif // end _NET_HTTP_HTTPCONTEXT_H