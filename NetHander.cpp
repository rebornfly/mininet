
#include "NetHander.h"
#include "EvSource.h"

using namespace server::net;

CNetDataHander::CNetDataHander()
{

}

CNetDataHander::~CNetDataHander()
{
	
}

void CNetDataHander::OnData(char* data, uint32_t len)
{
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