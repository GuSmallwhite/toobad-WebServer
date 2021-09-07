# WebServer
## 多线程的web服务器，可以解析http请求
* 基于半同步半反应堆模式，建立线程池，实现高并发
* 利用string相关函数进行http请求解析和响应报文生成
* 利用共享内存和io向量机制实现页面返回
* redis保存用户数据

### 线程安全手段（类）
* mutex
* sem

### 线程池类
* 维护线程列表和请求列表
* 主线程用于监听套接字，读取（创建http对象并且入队）和写入响应报文
* 工作线程用于接受连接，处理请求
* 请求列表中为http请求对象，各个线程竞争请求列表中的锁

### http请求类
* 维护连接，用读写套接字来初始化
* 实现报文解析，支持GET和POST
* 通过共享内存获得指向请求文件资源的指针，通过io向量机制，将响应行和内容聚集写入，返回响应报文

### main函数
* 注册epoll的可读、可写事件，当出现有效事件时，将请求压入请求列表，
* 创建连接，创建线程池，创建主线程用于监听连接

### 数据库
* 使用redis键值对来储存用户名和密码
* 单例模式创建redis客户端类，用于用户信息查询,包装了常用的查询命令，并对结果进行解析
* 函数getReply(向redis服务端发送请求)进行了加锁保护，防止同一块区域被同时改写而导致返回结果异常
* 需要拥有redis环境
    + 安装redis `sudo apt-get install redis-server`
    + 安装C++的hiredis库  `sudo apt-get install libhiredis-dev`

### 定时器
* 使用定时器清理非活跃连接
* 创建管道，信号（定时触发或按键触发）被触发时，向管道内写入
* epoll监听管道一端的读事件
* 根据读出的信号不同，进行定时清理或关闭服务的操作
* 信号handler仅向管道中写数据，用于更新标志位，并不进行真正的清理操作，保证处理足够快速
* 主函数循环中根据标志位的变化来执行具体的操作

### 网页页面
* 具有登录、注册、错误、欢迎、帮助五个页面

### 致谢
* 《Linux高性能服务器编程》游双著.

