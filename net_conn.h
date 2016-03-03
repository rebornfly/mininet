#ifndef _NET_CONN_H_
#define _NET_CONN_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <google/protobuf/message.h>
#include <boost/bind.hpp>
#include "event_source.h"
#include "socket_buffer.h"

#define MAX_BUFFER_SIZE 4096
namespace znb
{
	/** @defgroup 封装连接模块
		* @author reborn-lys
		* @version 0.1
		* @date 2015.4.2
		* @{
	*/

	//网络字节序的转换
	inline uint16_t XHTONS(uint16_t i16) {	
		return ((i16 << 8) | (i16 >> 8));	
	}	
	inline uint32_t XHTONL(uint32_t i32) {	
		return ((uint32_t(XHTONS(i32)) << 16) | XHTONS(i32>>16));
	}	
	inline uint64_t XHTONLL(uint64_t i64) {	
		return ((uint64_t(XHTONL((uint32_t)i64)) << 32) |XHTONL((uint32_t(i64>>32))));
	}
	
			
	inline std::string addr_ntoa(u_long ip)
	{ 
		struct in_addr addr;
		memcpy(&addr, &ip, 4);
		return std::string(::inet_ntoa(addr)); 
	}

    inline std::string str2Hex(char* data, int len)
    {
        char hex[17];
        memset(hex, 0 ,17);
        int n = 0;
        for(int i = 0; i < len; i++)
        {
            n += sprintf(hex + n, "%02x ", data[i]);
        }
        return hex;
    }


	class CNetConn : public IConn
	{
		public:
			
			CNetConn(uint32_t so, uint32_t ip, uint32_t port, ILinkHandler* linkHandler, IDataHandler* dataHandler):
			IConn(so),
			m_pLinkHandler(linkHandler),
			m_pDataHandler(dataHandler),
			m_uPort(port),
			m_uIp(ip),
			m_input(4*1024, 2*1024),
			m_output(4*1024, 2*1024),
			m_uLastWarn(0),
			m_uDropCount(0),
			m_bOverflow(false)
			{
					
			}
			virtual ~CNetConn();

			virtual uint32_t getPeerPort() 
			{
				return m_uPort;
			}

			virtual uint32_t getPeerIp() 
			{
				return m_uIp;
			}

			virtual void filterRead(char* data, size_t size)
			{
			}

			virtual void onRead();
			virtual void onWrite();
			virtual void onError(){}

            virtual void decryptData(){};

			void send(const char* data, uint32_t len);

			void send(std::string& strMsg, uint32_t cmd, uint32_t requestId, uint64_t uid64);
			/*客户端连接服务端回包接口*/
			virtual void sendResponse(google::protobuf::Message& msg, uint32_t cmd, uint32_t requestId, uint64_t uid64);
			
			void sendError(uint32_t cmd, uint32_t requestId, uint8_t error);	
		private:
				
			int sendHelper(const char* data, size_t size);

			ILinkHandler* m_pLinkHandler;
			IDataHandler* m_pDataHandler;
	
			uint32_t m_uPort;
			uint32_t m_uIp;
				
			SocketBuffer m_input;
			SocketBuffer m_output;

			uint32_t m_uLastWarn;
			uint32_t m_uDropCount;

			bool m_bOverflow;
					
	};
}
#endif
