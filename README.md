# A Simple C++ Web Server

## 简介

本项目为C++编写的多线程Web服务器（使用了C++11新特性），在应用层实现了Http服务器，其中Http服务器实现了解析Get、Post、Head等请求，目前支持访问静态资源、Http长连接、管线化请求，加入了异步日志记录状态诊断故障。

服务器编程采用“no-blocking IO+IO multiplexing”（Reactor）模式，在这个模型下每个IO线程都有一个事件循环（event loop），通过事件循环分发任务来实现用于处理读写和（单次或者周期性）定时事件。

1.  [日志Log设计与实现](https://github.com/Lannnnh/WebServer/blob/master/日志Log设计与实现.md)
2.  [TcpServer设计与实现](https://github.com/Lannnnh/WebServer/blob/master/TcpServer设计与实现.md)
3.  [Http Server设计与实现](https://github.com/Lannnnh/WebServer/blob/master/Http%20Server设计与实现.md)
4.  [测试与总结](https://github.com/Lannnnh/WebServer/blob/master/测试与总结.md)

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

## 技术

1.  使用智能指针管理对象资源
2.  使用Eventfd实现线程异步唤醒
3.  支持Http长连接、短连接
4.  实现了异步日志
5.  基于Poll的IO多路复用实现了Reactor模式（有待改进成Epoll）
6.  支持优雅关闭连接
7.  支持线程池实现多线程IO（线程轮转采用round-robin轮询调用）

## 总代码量

![codenums](https://github.com/Lannnnh/WebServer/blob/master/photo/codenums.png)