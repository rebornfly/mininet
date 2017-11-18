#ifndef _EVENT_SOURCE_H_
#define _EVENT_SOURCE_H_

#include <google/protobuf/message.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "net_epoll.h"
#include "logger.h"
#include "thread.h"

#define HEADER_SIZE 8
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
        ENUM_TYPE_TIMER = 3,     /**事件类型为定时器**/
        ENUM_TYPE_LOGGER = 4,     /**事件类型为zlog**/
        ENUM_TYPE_EVENTFD = 5,
        ENUM_TYPE_UNKNOWN = 6     /**未知类型**/
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

    //    void onRead() ;
    //    void onWrite() ;
    //    void onError();

    protected:

        int m_fd;
    private:
    
        uint32_t m_uMask;

        CEpoll* m_pEpoll;

        EventType m_type;

        Mutex mutex;
    };
    
    /*
     *   唤醒I/O线程事件源
    */
    class WakeupEvent
    {
    public:
        WakeupEvent()
        {
        }

        WakeupEvent(int so, CEpoll* ep):ev(new CEvSource(so, ep))
        {
        }

        virtual ~WakeupEvent()
        {

        }
        void handleRead()
        {
            uint64_t one = 1;
            ssize_t n = ::read(ev->getFd(), &one, sizeof one);
            if (n != sizeof one)
            {
                 log(Error, "WakeupEvent::onRead read %lu bytes instead of 8", n);
            }
            //log(Error, "WakeupEvent::onRead read %lu bytes instead of 8", n);
        }

        void readyReading()
        {
            ev->getEpoll()->netEpollAdd(ev.get(), EPOLL_READABLE);
        }

        void setReadCallback()
        {
            ev->setReadCallback(boost::bind(&WakeupEvent::handleRead, this));
        }
    private:
        boost::scoped_ptr<CEvSource> ev;

    };

    enum ConnectStat
    {
        ENUM_STATE_NONE,
        ENUM_STATE_CONNECTING,
        ENUM_STATE_CONNECTED
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
