#include "HttpResponse.h"
#include "Buffer.h"

#include <stdio.h>

// Http响应报文响应行格式：HTTP/1.1 200 OK
void HttpResponse::appendToBuffer(Buffer* output) const
{
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_);
    output->append("\r\n");

    // HTTP短连接
    if (closeConnection_)
    {
        output->append("Connection: close\r\n");
    }
    else
    {
        // 设置实体部分大小，HTTP长连接
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n");
    }

    // 添加响应报文首部
    for (const auto& header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    output->append(body_);
}