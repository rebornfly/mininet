#  轻量级epoll网络框架

### 特点：
* 智能指针管理链接生命周期， 确保跨线程链接正确资源回收
* 使用bind+function做回调，松耦合
* 工作线程回包采用event_fd通知网络线程
* zlog日志输出，协议采用protobuf
* 支持多种线程模型:  
  * 单线程模型 TcpServer+RequestMfcMap  
  * 多线程，网络线程和多个工作线程，网路线程收发请求管理epoll，工作线程处理请求 TcpServer + AsyncRequestMfcMap  
  * 多线程，网路线程负责监听listen socket连接事件，其他工作线程负责socket读写事件， 网络线程只负责listen和connect并通过管道派发给工作线程，TcpServerMt + RequestMfcMap  
         
###  依赖第三方库:  
  [zlog 高性能日志库](https://github.com/HardySimpson/zlog)  
  [protobuf 传输协议](https://github.com/google/protobuf)  
 
### 代码结构说明  
* event_source: epoll监听的事件源，每一个listen套接字或者链接套接字都**拥有**一个事件源非继承  
* net_epoll: 管理了epoll的所有操作，创建，以及修改删除事件
* request_dispatch: 单线程消息派遣
* async_request_dispatch:  多线程消息派遣
* auto_timer, timer: 基于epoll的定时器  
* globals:  epoll全局变量
* net_conn:  链接类，封装了该链接上的read和write
* net_handler:  链接数据处理
* socket_buffer:  链接的读写缓冲区
* socket_help:  常用socket相关函数
* tcpserer:  普通tcpserver
* tcpservermt:  多线程的tcpserer
* thread:  线程，互斥量相关
 
### 多线程模型  
* tcpserver+AsyncRequestMfcMap: 由网络线程和工作线程组成，工作线程不处理网络事件，工作线程的回报通过event_fd唤醒网络主线程：
 ```
 void CEpoll::pushFuctor( Functor& fun)
 {
    {
        MutexGuard lock(m_mutex);
        m_vecPendingFunctors.push_back(fun);
    }

    if (!m_bcallingPendingFunctors)
    {
        wakeup();
    }
 }
 
 void CEpoll::wakeup()
 {
    uint64_t u = 1;

    ssize_t flag = ::write(m_eventFd, &u, sizeof(uint64_t));
    if(flag != sizeof(uint64_t))
    {
        log(Error, "[CEpoll::wakeup] flag:%lu, err:%s", flag ,strerror(errno));
    }
 }
 ```
 * tcpservermt+RequestMfcMap: 由网络线程和工作线程组成，网络线程只负责listen和connect，工作线程负责链接上的read， write， close事件以及处理请求
   工作线程和网络线程都有自己的epoll，工作线程和网络线程通信采用管道触发读写，每个工作线程创建时候都会创建一个管道  
   ```
    if (pipe(m_pipefd) == -1)
    {
        throw Socket_Exception("pipe");
    }

    // 监控管道的读端
    pipecaller.reset(new PipeCaller(m_pipefd[0], &m_pE));
    
    pipecaller->setnewConnectionCallback(boost::bind(&TcpServerMt::Worker::handleRead, this));

   ```
### 链接生命周期  
链接 的生命周期管理是链接跨线程处理的难点，本框架通过shared_ptr管理链接生命周期，好处是当进入worker子线程里面处理消息时，如果网络线程收到该链接关闭请求，只会将其移除epoll，tcpserver的connections中移除，但是还存在子线程的回调函数中，所以此时并不会析够TcpConn，子线程依然可以调用该指针，等子线程回包时发现链接状态已经不是connected返回时，才会真正析够
 
### 用法
* 初始化日志`initLog`
* 初始化`mainLoop`  
* 定义数据处理器`dataHandler`
* 初始化消息回调映射对象`AsyncRequestMfcMap`, `RequestMfcMap`
* 初始化server `TcpServer`,`TcpServerMt`
* 启动server开始listen
* 启动主循环mainLoop，epoll开始run
```C++
class CTest{
public:
    CTest(RequestMfcMap& reqCtx)
     {
         reqCtx.addRequestMap<CTest, TestReq>(this,1 , &CTest::onTest);
     }
     ~CTest()
     {
     }

     void onTest(TestReq* msg, uint32_t cmd, const TcpConnPtr& conn)
     {
         TestRsp rsp;
         rsp.set_res("hello, i'm server");
         conn->sendResponse(rsp, 1);
     }
     
};
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
