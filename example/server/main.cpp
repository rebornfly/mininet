#include "../../net/async_request_dispatch.h"
#include "../../net/request_dispatch.h"
#include "../../net/mainloop.h"
#include "../../net/globals.h"
#include "../../net/net_epoll.h"
#include "../../net/net_handler.h"
#include "../../net/tcpservermt.h"
#include "../test.pb.h"
using namespace std;

class CTest
{
 public:
     CTest(RequestMfcMap& reqCtx)
         :count(0)
     {
         reqCtx.addRequestMap<CTest, TestReq>(this,1 , &CTest::onTest);
     }
     ~CTest()
     {

     }

     void onTest(TestReq* msg, uint32_t cmd, const TcpConnPtr& conn )
     {
         count++;
         if(count % 1000 == 0)
            log(Info, " %d ", count);
         TestRsp rsp;
         rsp.set_res("hello, i'm server");
         conn->sendResponse(rsp, 1);
     }

private:
    int count ;

};
int  main(int sz, char* argc[])
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
