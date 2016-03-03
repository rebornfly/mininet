#include<iostream>
#include "net_conn.h"

using namespace znb;
using namespace std;


CNetConn::~CNetConn()
{
}

void CNetConn::onRead()
{
	m_input.checkSpace();

	size_t uBytesToRead = m_input.free();
	if (uBytesToRead == 0)
	{
		throw buffer_overflow("Input buffer overflow");
	}

	char* buf = m_input.tail();

	int rc;
Retry:
	rc = ::recv(m_fd, buf, uBytesToRead, 0);
	if (rc == -1)
	{
		if (errno == EINTR)
		{
			// 系统调用被信号中断
			goto Retry;
		}
		else if (errno == EAGAIN)
		{
			// 非阻塞套接字上无任何数据可读
			return ;
		}
		else
		{
			log(Error, "%d  recv err:%s ", getpid(), strerror(errno));
		}
	}
	else if (rc == 0)
	{
		// 远程主机主动关闭连接
		m_pLinkHandler->onPeerClosed(this);
		return ;
	}
	else
	{
		filterRead(m_input.tail(), rc);

		// 读入数据
		m_input.inCrement(rc);
		
		
		int nBytesProcessed = m_pDataHandler->onData(this, m_input.data(), m_input.size());

		if (nBytesProcessed > 0)
		{
			m_input.erase(nBytesProcessed);
		}

	}
}
void CNetConn::onWrite()
{
	
	if (m_output.size() == 0)
	{
		getEpoll()->netEpollDel(this, EPOLL_WRITEABLE);
		return ;
	}

	int rc = 0;

	rc = sendHelper(m_output.data(), m_output.size());
	
	if(rc == -1)
	{
		log(Error, "[InnerConn::__OnWrite] failed with error conn id:%u", getFd());
		m_pLinkHandler->onError(this);
		return ;
	}

	if (rc > 0)
	{
		m_output.erase(rc);
		if (m_output.size() == 0)
		{
			// 停止监视该连接上的可写事件
			getEpoll()->netEpollDel(this, EPOLL_WRITEABLE);
		}

		if (m_output.size() < m_uLastWarn)
		{
			m_uLastWarn = m_output.size();
		}
	}
}

int CNetConn::sendHelper(const char* data, size_t size)
{
	int rc = 0;

Retry:
	rc = ::send(m_fd, data, size, 0);

	if (rc == -1)
	{
		if (errno == EINTR)
		{
			// 系统调用被信号中断
			goto Retry;
		}
		else if (errno == EAGAIN)
		{
			// 非阻塞套接字,写会导致阻塞
			return 0;
		}
		else
		{
			log(Error, "send err : %s", strerror(errno));
		}
	}

	return rc;
}

void CNetConn::sendResponse(google::protobuf::Message& msg, uint32_t cmd, uint32_t requestId, uint64_t uid64)
{
	std::string strMsg;
	
    msg.SerializeToString(&strMsg);

    CEpoll::Functor f = boost::bind(&CNetConn::send, this, strMsg, cmd, requestId, uid64);

    getEpoll()->pushFuctor(f);

//	sendResponse(strMsg, cmd, requestId, uid64);
}

void CNetConn::send(std::string& strMsg, uint32_t cmd, uint32_t requestId, uint64_t uid64)
{
	if(getConnStat() == ENUM_STATE_NONE)
	{
		log(Error ,"[sendResponse] fd:%u is closed or Error", getFd());
		return;
	}
	int len = strMsg.size();

	char data[HEADER_SIZE  + len];
	
	uint32_t size = XHTONL(len);
	
	uint32_t cmdNet = XHTONL(cmd);
	
	uint32_t requestIdNet = XHTONL(requestId);

    uint64_t uidNet = XHTONLL(uid64);
	
    uint32_t count = 0;

	data[count++] =  0;   //0
	
	data[count++] =  255; //1
	
    data[count++] = 21;   //2

	memcpy(data + count, (const char*)(&size), 4);  //3  
    count += 4;                                       

	memcpy(data + count, (const char*)(&cmdNet), 4); //7
	count += 4;

	memcpy(data + count, (const char*)(&requestIdNet), 4); //11
	count += 4;

	data[count++] = 0;   //15
	
	memcpy(data + count, (const char*)(&uidNet), 8);  //24
    count += 8;

	memcpy(data + count, strMsg.c_str(), strMsg.size());

	send(data, HEADER_SIZE + len);

}

void CNetConn::sendError(uint32_t cmd, uint32_t requestId,  uint8_t error)
{
	if(getConnStat() == ENUM_STATE_NONE)
	{
		log(Error ,"[sendResponse] fd:%u is closed or Error", getFd());
		return;
	}
	int len = 0;

    char data[HEADER_SIZE];


    uint32_t cmdNet = XHTONL(cmd);

    uint32_t requestIdNet = XHTONL(requestId);
	

    data[0] =  0;

    data[1] =  255;

    memcpy(data + 2, (const char*)(&len), 4);

    memcpy(data + 6, (const char*)(&cmdNet), 4);

    memcpy(data + 10, (const char*)(&requestIdNet), 4);

    data[14] = error;

    memcpy(data + 15, (const char*)(&requestIdNet), 4);

    send(data, HEADER_SIZE);

}

void CNetConn::send(const char* data, uint32_t size)
{
	if (m_output.size() == 0)
	{
		// 发送缓冲区无数据，尝试直接发送
		
		int nSent = sendHelper(data, size);
		if (size_t(nSent) == size)
		{
            // 发送完毕
			return ;
		}
		else
		{
            // 发送了一部分，其余部分需要append到发送缓冲区，等待后续发送
			data += nSent;
			size -= nSent;
			goto Append;
		}
	
		if(nSent == -1 ) 
		{
			log(Error, "[InnerConn::SendBin] failed with error conn id:%u",   getFd());
			m_pLinkHandler->onError(this);
			return ;
		}
	}

Append:
	if (!m_output.append(data, size))
	{
		m_uDropCount += size;
		if (!m_bOverflow || 0 == m_uDropCount % 1024){ //first time...
			log(Error, "buffer overflow, conn id:%d ip:%s port:%u drop:%u", 
			 getFd(), addr_ntoa(m_uIp).c_str(), m_uPort, m_uDropCount);
			m_bOverflow = true;
		}
	}
	else
	{
		if (m_bOverflow)
		{
			log(Warn, "buffer overflow recovered, conn id:%u ip:%s port:%u drop:%u", 
				 getFd(), addr_ntoa(m_uIp).c_str(), m_uPort, m_uDropCount);
			m_bOverflow = false;
			m_uDropCount = 0;
		}
	}

	// 每累计1K报警，避免过多报警
	if (m_output.size() - m_uLastWarn > 1024)
	{
		log(Warn, "send blocked output size: %u connid: %u ip:%s port:%u", 
			 uint32_t(m_output.size()),getFd(), addr_ntoa(m_uIp).c_str(), m_uPort);
		m_uLastWarn = m_output.size();
	}

	// 监视该连接上的可写事件
	getEpoll()->netEpollAdd(this, EPOLL_WRITEABLE);
	
}
