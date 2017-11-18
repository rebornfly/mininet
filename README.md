#  轻量级epoll网络框架

## 特点：
* 智能指针管理链接生命周期， 确保跨线程链接正确资源回收
* 使用bind+function做回调，松耦合
* 工作线程回包采用，event_fd通知网络线程
* 采用zlog作为日志输出，格式丰富，性能高
* 支持多种线程模型：
  1. 单线程模型 TcpServer+RequestMfcMap
  2. 多线程，网络线程和多个工作线程，网路线程收发请求管理epoll，工作线程处理请求 TcpServer + AsyncRequestMfcMap
  3. 多线程，网路线程负责侦听和多个工作线程， 网络线程只负责listen和connect，将连接派送给工作线程，由工作线程去收发关闭连接，每个工作线程管理一个   
     epoll ， TcpServerMt + RequestMfcMap
  
  ## 依赖第三方库：
  * !(zlog)[https://github.com/HardySimpson/zlog]
  * !(protobuf)[https://github.com/google/protobuf]




```C++
int main(int sz, char* argc[])
{
    initLog("../conf/log.conf", "server_lb", "", "");

    CMainLoop mainLoop;
    mainLoop.Init();

    //初始化服务器配置
    INIT_SERVER_CONFIG;

    //数据处理器
    CNetDataHandler dataHandler;
    //http数据处理
    CNetHttpDataHandler httpHandler;
    
    //面向后台服务端的连接管理，会创建普通的netconn
    CNetLinkHandler linkHandler;

    //面向客户端的连接管理，创建可加密的clientconn
    CConnectionManager connManager(&dataHandler);

    //消息派遣
    RequestMfcMap  m_reqContex;
    
    dataHandler.setRequestCtx(&m_reqContex);
    
    /**************业**务**逻**辑**部**分*************/
    CDispatcher dispatcherObj;
    CTest testObj(m_reqContex, &dispatcherObj, &connManager);

    /************************************************/
    //面向后端server配置
    TcpServer server;
    server.setEpoll(znb::Globals::GetEpoll());
    server.setDataHandler(&dataHandler);
	  server.setLinkHandler(&linkHandler);
    server.startServer(CServerConfigExt::getInstance()->getEndPort());	  


    //面向客户端server配置
	  TcpServer clientServer;
    clientServer.setEpoll(znb::Globals::GetEpoll());
    clientServer.setDataHandler(&httpHandler);
	  clientServer.setLinkHandler(&connManager);
    clientServer.startServer(CServerConfigExt::getInstance()->getFrontPort());

    mainLoop.Start();
    return 0;
}
```
