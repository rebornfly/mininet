#ifndef _EVENT_SOURCE_H_
#define _EVENT_SOURCE_H_

#include <google/protobuf/message.h>
#include <boost/function.hpp>
#include "net_epoll.h"
#include "logger.h"
#include "../common/znb_thread.h"

#define HEADER_SIZE 24
namespace znb 
{
	/** @defgroup �����ܿ�
		* @author reborn-lys
		* @version 0.1
		* @date 2015.4.2
		* @{
	*/

	enum EventType
	{
		ENUM_TYPE_CONN = 2,      /**�¼�����Ϊ����**/
		ENUM_TYPE_TIMER = 3,	 /**�¼�����Ϊ��ʱ��**/
		ENUM_TYPE_LOGGER = 4,	 /**�¼�����Ϊzlog**/
        ENUM_TYPE_EVENTFD = 5,
		ENUM_TYPE_UNKNOWN = 6	 /**δ֪����**/
	};
	class CEvSource
	{
	public:

        typedef boost::function<void()> EventCallback;

		CEvSource():m_fd(-1),m_uMask(0), m_type(ENUM_TYPE_UNKNOWN)
		{
				
		}

		explicit CEvSource(uint32_t fd):m_fd(fd),m_type(ENUM_TYPE_UNKNOWN)
        {

        }
		CEvSource(uint32_t fd, CEpoll* ep):m_fd(fd),m_pEpoll(ep),m_type(ENUM_TYPE_UNKNOWN)
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

        void setReadCallback(const EventCallback& cb)
        { readCallback_ = cb; }
        void setWriteCallback(const EventCallback& cb)
        { writeCallback_ = cb; }
        void setErrorCallback(const EventCallback& cb)
        { errorCallback_ = cb; }

        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;

	//	void onRead() ;
	//	void onWrite() ;
	//	void onError();

	protected:

		int m_fd;
	private:
	
		uint32_t m_uMask;

		CEpoll* m_pEpoll;

		EventType m_type;

		Mutex mutex;
	};
	
    /*
     *   ����I/O�߳��¼�Դ
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
            log(Error, "WakeupEvent::onRead read %lu bytes instead of 8", n);
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
    class CTcpConn;
	class IDataHandler
	{
	public:

		~IDataHandler(){}

		virtual int onData(const boost::shared_ptr<CTcpConn>& conn, char* data, uint32_t len) = 0;

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
}

#endif
