#ifndef _NET_DATA_HANDER_
#define _NET_DATA_HANDER_
#include <stdio.h>
#include "EvSource.h"


namespace server
{
	namespace net
	{
		class CNetDataHander : public IDataHander
		{
			public:
				CNetDataHander();
				~CNetDataHander();
				virtual void OnData(char* data, uint32_t len);
		};


	////////////////////////////////
		class IConn;
		class CNetLinkHander : public ILinkHander
		{
			public:

				CNetLinkHander();
				~CNetLinkHander();

				virtual void OnError();
				virtual void OnPeerClosed(IConn* conn);
				virtual void OnConnected();
		};
	}
}
#endif