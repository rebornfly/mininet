#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include "EvSource.h"
#include "NetSocketBuffer.h"

#define MAX_BUFFER_SIZE 4096
namespace server
{
	namespace net
	{

		class CNetConn : public IConn
		{
			public:
				
				CNetConn();
				CNetConn(uint32_t so, uint32_t ip, uint32_t port, ILinkHander* linkHander, IDataHander* dataHander):
				IConn(so),
				linkHander(linkHander),
				dataHander(dataHander),
				uPort(port),
				uIp(ip),
				m_input(4*1024, 2*1024),
				m_output(4*1024, 2*1024)
				{
					
				}
				~CNetConn();

				uint32_t getPeerPort() const
				{
					return uPort;
				}

				uint32_t getPeerIp() const
				{
					return uIp;
				}

				virtual void OnRead();
				virtual void OnWrite();
				virtual void OnError(){}

				void Send(const char* data, uint32_t len);

			private:

				ILinkHander* linkHander;
				IDataHander* dataHander;
	
				uint32_t uPort;
				uint32_t uIp;
				
				CNetSockBuff m_input;
				CNetSockBuff m_output;

		};
	}
}