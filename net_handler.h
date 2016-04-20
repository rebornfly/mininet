#ifndef _NET_DATA_HANDER_
#define _NET_DATA_HANDER_
#include <stdio.h>
#include "event_source.h"
#include "request_dispatch.h"



namespace znb
{
	/**datahandler 处理连接上收到的数据*/
	class CNetDataHandler : public IDataHandler
	{
		public:
			CNetDataHandler();
			~CNetDataHandler();
				
			void setRequestCtx(RequestMfcMap* reqCtx)
			{
				m_pRequestCtx = reqCtx;
			}

			virtual int onData(IConn* conn, char* data, uint32_t len);

		private:

			RequestMfcMap* m_pRequestCtx;
	};


	/**linkhandler 处理连接事件*/
	class IConn;
	class CNetLinkHandler : public ILinkHandler
	{
		public:

			CNetLinkHandler();
			~CNetLinkHandler();

			virtual void onError(IConn* conn);
			virtual void onPeerClosed(IConn* conn);
			virtual void onAccept(CEpoll*, uint32_t, IDataHandler*);
	};
}

#endif
