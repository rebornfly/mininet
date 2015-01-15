
#include "NetHander.h"

using namespace server::net;

CNetDataHander::CNetDataHander()
{

}

CNetDataHander::~CNetDataHander()
{
	
}

void CNetDataHander::OnData(char* data, uint32_t len)
{
	printf("%s, size:%u", data, len);
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

void CNetLinkHander::OnPeerClosed()
{
	
}