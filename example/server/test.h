#include "../test.pb.h"
#include "../tinynet/request_dispatch.h"
using namespace std;

class CTest
{
public:
	CTest(RequestMfcMap& reqCtx)
    {

	    reqCtx.addRequestMap<CTest, TestReq>(this,1 , &CTest::onTest);
    }
	~CTest()
    {

    }

	void onTest(TestReq* msg, uint32_t cmd, uint32_t requestId, const TcpConnPtr& conn, uint64_t uid64 )
    {
        count++;
        TestRsp rsp;
        rsp.set_res("hello, i'm server");
    }

private:

    int count ;
};

