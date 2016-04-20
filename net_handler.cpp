#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include "net_handler.h"
#include "event_source.h"
#include "net_conn.h"

using namespace znb;

CNetDataHandler::CNetDataHandler()
{

}

CNetDataHandler::~CNetDataHandler()
{
	
}

int CNetDataHandler::onData(IConn* conn, char* data, uint32_t len)
{
	int nProcess = 0;
	while(len > 0)
	{
		if(len < HEADER_SIZE)
		{
			//need more
			break;
		}
	    
		uint16_t padding = XHTONS(*((uint16_t*)data));

        uint8_t length = *((uint8_t*)(data + 2));

		uint32_t size = XHTONL( *((uint32_t*)(data + 3)));

		if(len - HEADER_SIZE < size)
		{
			//need more
			break;
		}

		data += sizeof(uint16_t);
        data += sizeof(uint8_t);
		data += sizeof(uint32_t);

		uint32_t cmd = XHTONL( *((uint32_t*)data));
		data += sizeof(uint32_t);

		uint32_t requestId = XHTONL( *((uint32_t*)data));
		data += sizeof(uint32_t);

			
		uint8_t ret = *((uint8_t*)data)	;
		data += sizeof(uint8_t);
		

		uint64_t uid = XHTONLL( *((uint64_t*)data));
		data += sizeof(uint64_t);
	
        uint32_t reverse = length + 3 - HEADER_SIZE;
        if(reverse > 0)
        {
            data += reverse;
        }
	
        log(Info, "[onData]-----[size:%u, padding:%u, length:%u, cmd:%u, requestId:%u, reverse:%u, ret:%u, uid:%lu]", size, padding, length, cmd, requestId, reverse, ret, uid);

		if(m_pRequestCtx)
		{
			std::string strMsg(data, size);
			m_pRequestCtx->requestDispatch(strMsg, cmd, requestId, uid, conn);
		}
		
		len -= HEADER_SIZE;	
		len -= size;
        len -= reverse;
		data += size;
		
		nProcess += size + HEADER_SIZE;
		
	}

	return nProcess;
}

///////////////////////////


CNetLinkHandler::CNetLinkHandler()
{
	
}

CNetLinkHandler::~CNetLinkHandler()
{
	
}

void CNetLinkHandler::onAccept(CEpoll* ep, uint32_t fd, IDataHandler* dataHandler)
{

	char addr[256];

	socklen_t len = 256;

	int m_so = ::accept(fd, (struct sockaddr*)&addr, &len);

	if(m_so == -1)
	{
		log(Error, "pid:%u  accept fd:so %u:%u,err:%s", getpid(), fd,m_so, strerror(errno));
		return;
	}

	if( fcntl(m_so, F_SETFL, fcntl(m_so, F_GETFD, 0)|O_NONBLOCK) == -1 )
	{
		log(Error, "[onAccept] pid :%u, set nonblock err:%s", getpid(), strerror(errno));
		return;
	}
	
	struct sockaddr_in* addrsock = (struct sockaddr_in*)addr;

	CNetConn* conn = new CNetConn(m_so, addrsock->sin_addr.s_addr, ntohs(addrsock->sin_port), this, dataHandler);
	
	conn->setEpoll(ep);

	ep->netEpollAdd(conn, EPOLL_READABLE);

	conn->setConnStat(ENUM_STATE_CONNECTED);

	conn->setEventType(ENUM_TYPE_CONN);

	log(Info, "[onAccept] pid:%d Accept from:%s : %u sockid:%u conn:%p", getpid(), inet_ntoa(addrsock->sin_addr), addrsock->sin_port, m_so, conn);
	
}

void CNetLinkHandler::onError(IConn* conn)
{
	CEpoll* pEpoll = conn->getEpoll();

	pEpoll->remove(conn);

	conn->setConnStat(ENUM_STATE_NONE);

	log(Info, "[OnError] pEpoll %p", pEpoll);
}

void CNetLinkHandler::onPeerClosed(IConn* conn)
{
	CEpoll* pEpoll = conn->getEpoll();
	
	pEpoll->remove(conn);

	conn->setConnStat(ENUM_STATE_NONE);

	log(Info, "[OnPeerClosed] sockfd:%u ",conn->getFd());
}
