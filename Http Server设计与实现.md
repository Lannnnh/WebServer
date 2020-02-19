# Http Server设计与实现


Http Server就是套了一个TcpServer的壳子，然后增加了接收并解析Http请求报文，根据请求报文再发送Http响应报文的一个Class。

Http Server包含如下这些模块（除TcpServer外）。

## HttpRequest Class

HttpRequest Class是一个Http请求状态描述集合，包含请求方法、Http协议版本、URL等。

## HttpContext Class

HttpContext Class处理Http请求报文，解析请求行和首部字段。parseRequest()函数用来解析请求报文，根据state状态判断解析的是请求行（processRequestLine()函数被调用用来解析请求行，将解析结果保存在request状态集合里面）还是首部报文。

## HttpResponse Class

HttpResponse Class是一个http响应状态描述集合，它用appendToBuffer()把响应报文添加到OutputBuffer里面发送给Http客户端。