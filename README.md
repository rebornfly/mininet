#  轻量级epoll网络框架

### 特点：
* 智能指针管理链接生命周期， 确保跨线程链接正确资源回收
* 使用bind+function做回调，松耦合
* 工作线程回包采用，event_fd通知网络线程
* 采用zlog作为日志输出，格式丰富，性能高
* 支持多种线程模型:  
  * 单线程模型 TcpServer+RequestMfcMap  
  * 多线程，网络线程和多个工作线程，网路线程收发请求管理epoll，工作线程处理请求 TcpServer + AsyncRequestMfcMap  
  * 多线程，网路线程负责侦听和多个工作线程， 网络线程只负责listen和connect，将连接派送给工作线程，由工作线程去收发关闭连接，每个工作线程管理一个 
      epoll ， TcpServerMt + RequestMfcMap  
         
###  依赖第三方库:  
  [zlog 高性能日志库](https://github.com/HardySimpson/zlog)  
  [prootobuf 传输协议](https://github.com/google/protobuf)  
  
###  用法
* 初始化日志`initLog`
* 初始化`mainLoop`
* 定义数据处理器`dataHandler`
* 初始化消息回调映射对象`AsyncRequestMfcMap`, `RequestMfcMap`
* 初始化server `TcpServer`,`TcpServerMt`
* 启动server开始listen
* 启动主循环mainLoop，epoll开始run
```C++
int main(int sz, char* argc[])
{
    initLog("../conf/log.conf", "server");
    CMainLoop mainLoop;
    mainLoop.Init();

    CNetDataHandler dataHandler;

    //AsyncRequestMfcMap  m_reqContex ;
    RequestMfcMap  m_reqContex ;
    CTest objTest(m_reqContex);

    dataHandler.setRequestCtx(&m_reqContex);
    TcpServerMt server(11111);
    server.setDataHandler(&dataHandler);
    server.startServer();	

    mainLoop.Start();
    return 0;
}
```
