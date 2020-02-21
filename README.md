# A Simple C++ Web Server

## 简介

本项目为C++编写的Web服务器（使用了部分的C++11新特性）。

1.  [日志Log设计与实现](https://github.com/Lannnnh/WebServer/blob/master/日志Log设计与实现.md)
2.  [TcpServer设计与实现](https://github.com/Lannnnh/WebServer/blob/master/TcpServer设计与实现.md)
3.  [HttpServer设计与实现](https://github.com/Lannnnh/WebServer/blob/master/HttpServer设计与实现.md)

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

服务器运行起来之后在浏览器访问服务器http://127.0.0.1:8000之后的显示效果![html](G:\beap\Webserver\photo\html.png)
