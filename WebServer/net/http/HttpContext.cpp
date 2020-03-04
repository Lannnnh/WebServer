#include "Buffer.h"
#include "HttpContext.h"

bool HttpContext::processRequestLine(const char* begin, const char* end)
{
    // 比如请求行为：GET http://lannnnh.com/index.htm [?query] HTTP/1.1
    bool succeed = false;
    const char* start = begin;
    // 找到第一个空格，把请求动作GET提取出来
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space))
    {
        start = space+1;
        space = std::find(start, end, ' ');
        if (space != end)
        {
            // 查看是否有query参数，给动态网页传递参数，暂不支持访问动态资源
            const char* question = std::find(start, space, '?');
            if (question != space)
            {
                // 设置路径 http://lannnnh.com/index.htm
                request_.setPath(start, question);
                request_.setQuery(question, space);
            }
            else
            {
                request_.setPath(start, space);
            }
            // 最后找到HTTP版本参数，1.1或者1.0
            start = space+1;
            succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
            if (succeed)
            {
                if (*(end-1) == '1')
                {
                    request_.setVersion(HttpRequest::kHttp11);
                }
                else if (*(end-1) == '0')
                {
                    request_.setVersion(HttpRequest::kHttp10);
                }
                else
                {
                    succeed = false;
                }
            }
        }
    }
    
    return succeed;
}

bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
{
    bool ok = true;
    bool hasMore = true;
    while (hasMore)
    {
        if (state_ == kExpectRequestLine)
        {
            const char* crlf = buf->findCRLF();
            // 解析请求报文的请求行（\r\n结尾）
            if (crlf)
            {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok)
                {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf+2);
                    state_ = kExpectHeaders;
                }
                else
                {
                    hasMore = false;
                }
            }
            else
            {
                hasMore = false;
            }
        }
        // 解析请求报文的首部，比如Connection: Keep-Alive\r\n
        else if (state_ == kExpectHeaders)
        {
            const char* crlf = buf->findCRLF();
            if (crlf)
            {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf)
                {
                    request_.addHeader(buf->peek(), colon, crlf);
                }
                else
                {
                    // 报文首部解析完毕，遇到了空行\r\n跳出解析
                    state_ = kGotAll;
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2);
            }
            else 
            {
                hasMore = false;
            }
        }
        else if (state_ == kExpectBody)
        {
            // need process
        }
    }

    return ok;
}