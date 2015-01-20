#include "NetConn.h"

using namespace server::net;

CNetConn::CNetConn():
uPort(-1),
uIp(-1),
m_input(4*1024, 2048),
m_output(4*1024, 2048)
{
	
}

CNetConn::~CNetConn()
{

}

void CNetConn::OnRead()
{
	if(m_input.Size() == MAX_BUFFER_SIZE)
	{
		Log(Error, "input owerflow");
		return;
	}

	char* pStart = m_input.Tail();

Retry:
	int rc = ::recv(m_fd, pStart, MAX_BUFFER_SIZE - m_input.Size(), 0);

	if(rc == 0)
	{
		Log(Info, "OnRead pEpoll:%0x", GetEpoll());
		linkHander->OnPeerClosed(this);
	}
	else if(rc == -1)
	{
		if (errno == EINTR)
		{
			// 系统调用被信号中断
			goto Retry;
		}
		else if (errno == EAGAIN)
		{
			// 非阻塞套接字上无任何数据可读
			return;
		}
		else
		{
			Log(Error ,"recv err fd:%u:%s", m_fd, strerror(errno));
			return;
		}
	}
	else
	{
		m_input.ResetSize(m_input.Size() + rc);
		dataHander->OnData(m_input.Data(), m_input.Size());
	}
}

void CNetConn::OnWrite()
{
	if(m_output.Size() == MAX_BUFFER_SIZE)
	{
		Log(Error, "output buffer owerflow");
		return;
	}
	
	if(m_output.Size() == 0)  //水平触发，移除
	{

		GetEpoll()->NetEpollDel(this, EPOLL_WRITEABLE);
	}

Retry:
	int rc = ::send(m_fd, m_output.Data(), m_output.Size(), -1);
	if(rc == -1)
	{
		if(errno == EINTR)
			goto Retry;
		else
		{
			Log(Error ,"recv err:%s", strerror(errno));
			return;
		}
	}
	if(rc > 0)
	{
		if(rc == m_output.Size())
		{
			m_output.ResetSize(0) ;
			GetEpoll()->NetEpollDel(this, EPOLL_WRITEABLE);
		}
		else
		{
			m_output.Resize(rc);
		}
	}

}

void CNetConn::Send(const char* data, uint32_t len)
{
	if(m_output.Size() == 0)
	{
		int rc = ::send(m_fd, data, len, 0);
		if(rc > 0)
		{
			if(rc == len)
				return;
			if(rc < len)
			{
				const char* pStart = data + rc;
				m_output.AppendData(pStart, len - rc);      //此处不检查空间，默认一次发送数据小于8M
				GetEpoll()->NetEpollAdd(this, EPOLL_WRITEABLE);
			}
			return;
		}
	}

	if(!m_output.CheckCapacity(len))
	{
		Log(Error, "buffer overflow drop:%u", len);
		return;
	}
	else
	{
		m_output.AppendData(data, len);
		GetEpoll()->NetEpollAdd(this, EPOLL_WRITEABLE);
	}
}