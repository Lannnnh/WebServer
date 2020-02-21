# A Simple C++ Web Server

## 简介

本项目为C++编写的Web服务器（使用了部分的C++11新特性）。

## 开发平台

Linux： 18.04.2

Complier： g++ 7.4.0

## 安装与使用

在WebServer目录下

	cmake . && make

在http目录下（threadNum为可选参数，设置线程数）

```
./http_test [-threadNum]
```



