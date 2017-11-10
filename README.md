# ACEFileService  ACE文件服务器

## 简介
基于ACE实现的文件服务器，支持SOCKET/FTP方式，同时提供`文件服务器查看器`，是一个很好的实例;


   **子项目模块:**
1. ACE_FILE_SERVER_SRC:       文件服务器/客户端源码  (可根据条件编译成服务器端和客户端[windows/linux均支持],开发语言:C++,库:ACE)
1. FTP_Connecter:             FTP客户端连接库 (开发语言：C++)
1. ACE_FILE_SERVER_VIEW_SRC:  文件查看器源码  (可连接FTP和Socket文件服务器,开发语言:c#)

很早之前项目中的关键模块，最近整理历史资料中翻出来的，开源出来以供人学习之用;
