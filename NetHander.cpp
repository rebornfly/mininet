
#include "NetHander.h"
#include "EvSource.h"

using namespace server::net;

CNetDataHander::CNetDataHander()
{

}

CNetDataHander::~CNetDataHander()
{
	
}
//@brief 根据发送数据协议解析
void CNetDataHander::OnData(char* data, uint32_t len)
{
	//TODO 进行消息解析并由DataHander进行消息派遣
	Log(Info, "%s, size:%u", data, len);
}

///////////////////////////


CNetLinkHander::CNetLinkHander()
{
	
}

CNetLinkHander::~CNetLinkHander()
{
	
}

void CNetLinkHander::OnConnected()
{
	
}

void CNetLinkHander::OnError()
{
	
}

void CNetLinkHander::OnPeerClosed(IConn* conn)
{
	CEpoll* pEpoll = conn->GetEpoll();
	Log(Info, "OnPeerClosed pEpoll %0x", pEpoll);
	pEpoll->remove(conn);
}
