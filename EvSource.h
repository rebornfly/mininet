#pragma once
#include "NetEpoll.h"

namespace server
{
	namespace net 
	{
		class CEvSource
		{
		public:
			CEvSource():m_fd(-1),mask(0)
			{
				
			}

			CEvSource(uint32_t fd):m_fd(fd)
			{
				
			}

			~CEvSource()
			{

			}

			void SetEpoll(CEpoll* pE)
			{
				pEpoll = pE;
			}
			
			CEpoll* GetEpoll()
			{
				return pEpoll;
			}

			uint32_t getFd()
			{
				return m_fd;
			}

			uint32_t getCurrentMask()
			{
				return mask;
			}

			void setCurrentMask(uint32_t uMask)
			{
				mask = uMask;
			}
			virtual void OnRead() = 0;
			virtual void OnWrite() = 0;
			virtual void OnError() = 0;
		private:

			uint32_t m_fd;
			uint32_t mask;

			CEpoll* pEpoll;
		};

		class IDataHander
		{
			public:

				~IDataHander(){}

				virtual void OnData(char* data, uint32_t len) = 0;

		};

		class ILinkHander
		{
			public:

				~ILinkHander(){}

				virtual void OnPeerClosed() = 0;
				virtual void OnError() = 0;
				virtual void OnConnected() = 0;
		};

		class IDataHanderAware
		{
			public:
				virtual IDataHander* getDataHander() const
				{
					return dataHander;
				}
				
				virtual void setDataHander(IDataHander* hander)
				{
					dataHander = hander;
				}
			private:
				IDataHander* dataHander;
		};

		class ILinkHanderAware
		{
			public:
				virtual ILinkHander* getLinkHander() const
				{
					return linkHander;
				}
				virtual void setLinkHander(ILinkHander* hander)
				{
					linkHander = hander;
				}
			private:

				ILinkHander* linkHander;
		};

	}
}