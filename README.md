# MyWebserver
Linux下C++轻量级Web服务器

- 使用 **线程池 + 非阻塞socket + epoll(ET和LT均实现)** 的并发模型
- 使用**状态机**解析HTTP请求报文，支持解析**GET和POST**请求
- 访问服务器数据库实现web端用户**注册、登录**功能，可以请求服务器**图片和视频文件**
- 实现**同步/异步日志系统**，记录服务器运行状态



## 快速运行

- 服务器测试环境

  - MySQL版本8.0.35

- 浏览器测试环境

  - Windows、Linux均可
  - Chrome
  - FireFox
  - Edge

- 测试前确认已安装MySQL数据库

  ```
  // 建立yourdb库
  create database yourdb;
  
  // 创建user表
  USE yourdb;
  CREATE TABLE user(
      username char(50) NULL,
      passwd char(50) NULL
  )ENGINE=InnoDB;
  
  // 添加数据
  INSERT INTO user(username, passwd) VALUES('root', '1234');
  ```

  
- 修改config.ini中的数据库初始化信息

  ```
	[web message]
	webIp=your linux IP
	webPort=your Port

	[sql message]
	sqlHost=0
	sqlPort=0
	sqlUserName=root
	sqlPassword=1234

  ```


- build

  ```
  sh ./build.sh
  ```

  

- 启动server

  ```
  ./server
  ```

  

- 浏览器端

  ```
  ip:Port
  ```
