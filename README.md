##mininet 是一个基于epoll的高性能网络框架库

##一下是一个 接入服务器模型，并且在实际中得到了应用


>
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

