# A Simple C++ Web Server

## 简介

本项目为C++编写的多线程Web服务器（使用了C++11新特性），在应用层实现了Http服务器，其中Http服务器实现了解析Get、Post、Head等请求，目前支持访问静态资源、Http长连接、管线化请求，加入了异步日志记录状态诊断故障。

1.  [日志Log设计与实现](https://github.com/Lannnnh/WebServer/blob/master/日志Log设计与实现.md)
2.  [TcpServer设计与实现](https://github.com/Lannnnh/WebServer/blob/master/TcpServer设计与实现.md)
3.  [Http Server设计与实现](https://github.com/Lannnnh/WebServer/blob/master/Http%20Server设计与实现.md)

## 开发平台

Linux： 18.04.2

Complier： g++ 7.4.0

## 安装与使用

在WebServer目录下

	cmake . && make

在WebServer/net/http目录下（threadNum为可选参数，设置线程数）

```
./httpserver_test [-threadNum]
```

服务器运行起来之后在浏览器访问服务器http://127.0.0.1:8000 之后的显示效果
![html](https://github.com/Lannnnh/WebServer/blob/master/photo/html.png)

## 总代码量

![codenums](https://github.com/Lannnnh/WebServer/blob/master/photo/codenums.png)