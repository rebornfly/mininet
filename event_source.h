#ifndef _EVENT_SOURCE_H_
#define _EVENT_SOURCE_H_

#include <google/protobuf/message.h>
#include "net_epoll.h"
#include "logger.h"
#include "../common/znb_thread.h"

#define HEADER_SIZE 24
namespace znb 
{
	/** @defgroup 网络框架库
		* @author reborn-lys
		* @version 0.1
		* @date 2015.4.2
		* @{
	*/

	enum EventType
	{
		ENUM_TYPE_CONN = 2,      /**事件类型为连接**/
		ENUM_TYPE_TIMER = 3,	 /**事件类型为定时器**/
		ENUM_TYPE_LOGGER = 4,	 /**事件类型为zlog**/
        ENUM_TYPE_EVENTFD = 5,
		ENUM_TYPE_UNKNOWN = 6	 /**未知类型**/
	};
	class CEvSource
	{
	public:
		CEvSource():m_fd(-1),m_uMask(0), m_type(ENUM_TYPE_UNKNOWN),userCount(0)
		{
				
		}

		explicit CEvSource(uint32_t fd):m_fd(fd),m_type(ENUM_TYPE_UNKNOWN),userCount(0)
		{
				
		}

		virtual ~CEvSource()
		{
			if(-1 != m_fd)
			{
				::close(m_fd);
			}
		}

		void setEpoll(CEpoll* pE)
		{
			m_pEpoll = pE;
		}
			
		CEpoll* getEpoll()
		{	
			return m_pEpoll;
		}	

		int getFd()
		{	
			return m_fd;
		}

		uint32_t getCurrentMask()
		{
			return m_uMask;
		}

		void setCurrentMask(uint32_t uMask)
		{
			m_uMask = uMask;
		}

		void setEventType(EventType type)
		{
			m_type = type;
		}
		EventType getEventType()
		{
			return m_type;
		}

		virtual void onRead() = 0;
		virtual void onWrite() = 0;
		virtual void onError() = 0;

	protected:

		int m_fd;
	private:
	
		uint32_t m_uMask;

		CEpoll* m_pEpoll;

		EventType m_type;

		uint32_t userCount;

		Mutex mutex;
	};
	
    /*
     *   唤醒I/O线程事件源
    */
    class WakeupEvent : public CEvSource
    {
    public:
        WakeupEvent()
        {
        }
        
        explicit WakeupEvent(int m_so):CEvSource(m_so)
        {
        }

        virtual ~WakeupEvent()
        {
            
        }
        virtual void onRead() 
        {
            uint64_t one = 1;
            ssize_t n = ::read(getFd(), &one, sizeof one);
            if (n != sizeof one)
            {
                 log(Error, "WakeupEvent::onRead read %lu bytes instead of 8", n);
            }
        }
		virtual void onWrite() 
        {
        }
		virtual void onError() 
        {
        }

    };

	enum ConnectStat
	{
		ENUM_STATE_NONE,
		ENUM_STATE_CONNECTING,
		ENUM_STATE_CONNECTED
	};

	class IConn : public CEvSource
	{
	public:
		IConn()
		{

		}
		explicit IConn(uint32_t m_so):CEvSource(m_so)
		{

		}
		virtual ~IConn()
		{
		}
		void setConnStat(ConnectStat stat)
		{
			m_stat = stat;
		}

		ConnectStat getConnStat()
		{
			return m_stat;
		}

		virtual void onRead() = 0;
		virtual void onWrite() = 0;
		virtual void onError() = 0;
		
        virtual uint32_t getPeerPort() = 0;
        virtual uint32_t getPeerIp() = 0;

		virtual void send(std::string& strMsg, uint32_t cmd, uint32_t requestId, uint64_t uid64) = 0;
		virtual void sendResponse(google::protobuf::Message& msg, uint32_t cmd, uint32_t requestId, uint64_t uid64) = 0;
		virtual void sendError(uint32_t cmd, uint32_t requestId, uint8_t error) = 0;
	protected:

		uint32_t m_uConnId;

		ConnectStat m_stat;
	};

	class IDataHandler
	{
	public:

		~IDataHandler(){}

		virtual int onData(IConn* conn, char* data, uint32_t len) = 0;

	};

	class ILinkHandler
	{
	public:

		~ILinkHandler(){}

		virtual void onPeerClosed(IConn* conn) = 0;
		virtual void onError(IConn* conn) = 0;
		virtual void onAccept(CEpoll* ep, uint32_t fd, IDataHandler* dataHandler) = 0;
	};

	class IDataHandlerAware
	{
	public:
		virtual IDataHandler* getDataHandler() const
		{
			return m_pDataHandler;
		}
				
		virtual void setDataHandler(IDataHandler* handler)
		{
			m_pDataHandler = handler;
		}
	private:
		IDataHandler* m_pDataHandler;
	};

	class ILinkHandlerAware
	{
	public:
		ILinkHandler* getLinkHandler() const
		{			
			return m_pLinkHandler;
		}
		virtual void setLinkHandler(ILinkHandler* handler)
		{
			m_pLinkHandler = handler;
		}
	private:

		ILinkHandler* m_pLinkHandler;
	};

}

#endif
