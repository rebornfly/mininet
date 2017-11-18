#include "../../net/async_request_dispatch.h"
#include "../../net/mainloop.h"
#include "../../net/globals.h"
#include "../../net/net_epoll.h"
#include "../../net/net_handler.h"
#include "../../net/tcpserver.h"
#include "../../net/auto_timer.h"
#include "../test.pb.h"
#include<stdio.h>
#include<thread>

using namespace std;
using namespace znb;

const int work_size = 100;


class CTest
{
 public:
     CTest(RequestMfcMap& reqCtx, CNetDataHandler* dataHandler, TcpServer* server)
     {
         reqCtx.addRequestMap<CTest, TestRsp>(this,1 , &CTest::onTest);

         for(int i = 0; i < work_size; i++)
         {
             conn[i] = server->clientConnection("127.0.0.1", 11111);
         }
         m_Timer.Init(this, &CTest::test);
         m_Timer.Start(1000);
     }
     ~CTest()
     {

     }

     void onTest(TestRsp* msg, uint32_t cmd, const TcpConnPtr& conn )
     {
         count++;
         log(Info, " %u  %s ", count, msg->res().c_str());
         TestReq req;
         req.set_name("i'm client");
         conn->sendResponse(req, 1);
     }

     void test(){
         for(int i =0; i < work_size; i++){
            TestReq req;
            req.set_name("i'm client");

            conn[i]->sendResponse(req, 1);
         }
         m_Timer.Stop();
     }


private:
    CAutoTimer m_Timer;
    int count ;

    TcpConnPtr conn[work_size];

};
int  main(int sz, char* argc[])
{
    initLog("../conf/log.conf", "client");

    CMainLoop mainLoop;
    mainLoop.Init();

    CNetDataHandler dataHandler;

    AsyncRequestMfcMap  m_reqContext ;

    dataHandler.setRequestCtx(&m_reqContext);
    TcpServer server(11112);
    server.setDataHandler(&dataHandler);
    server.startServer();

    CTest objTest(m_reqContext, &dataHandler, &server);

    mainLoop.Start();
    return 0;
}
