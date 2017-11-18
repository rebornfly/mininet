#ifndef _NET_CONN_H_
#define _NET_CONN_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <google/protobuf/message.h>
#include <execinfo.h>
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
    class Socket_Exception : public exception
    {
    public:
        Socket_Exception(const string &sBuffer) : _buffer(sBuffer){}

        const char* what() const throw()
        {
             return _buffer.c_str();
        }
        void getBacktrace()
        {
            void * array[64];
            int nSize = backtrace(array, 64);
            char ** symbols = backtrace_symbols(array, nSize);

            for (int i = 0; i < nSize; i++)
            {
                _buffer += symbols[i];
                _buffer += "\n";
            }
            free(symbols);
        }

        ~Socket_Exception() throw(){}    
    private:
        string  _buffer;
    };
    
            
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

    class CTcpConn : boost::noncopyable,
                      public boost::enable_shared_from_this<CTcpConn>
    {
        public:
            
            typedef boost::shared_ptr<CTcpConn> TcpConnPtr;
            typedef boost::function<void (const TcpConnPtr&)> CloseCallback;

            CTcpConn(CEpoll* ep, int so, char* ip, uint32_t port,  IDataHandler* dataHandler):
            m_pDataHandler(dataHandler),
            m_uPort(port),
            m_strIp(ip),
            m_input(4*1024, 2*1024),
            m_output(4*1024, 2*1024),
            m_uLastWarn(0),
            m_uDropCount(0),
            m_bOverflow(false),
            fd(so),
            ev(new CEvSource(so, ep))
            {
                ep->netEpollAdd(ev.get(), EPOLL_READABLE);
                ev->setReadCallback(boost::bind(&CTcpConn::onRead, this));
                ev->setWriteCallback(boost::bind(&CTcpConn::onWrite, this));
            }
            virtual ~CTcpConn();

            void setConnStat(ConnectStat stat)
            {
                m_stat = stat;
            }

            ConnectStat getConnStat()
            {
                return m_stat;
            }

            void setCloseCallback(const CloseCallback& cb)
            { 
                closeCallback = cb; 
            }

            uint32_t getPeerPort() 
            {
                return m_uPort;
            }

            string getPeerIp() 
            {
                return m_strIp;
            }

            int getFd()
            {
                return fd;
            }

            void filterRead(char* data, size_t size)
            {
            }
            void setBlocking(bool bBlock);

            void onRead();
            void onWrite();
            void onError(){}
            
            void handleClose(TcpConnPtr conn);

            //此处可以解密数据， 如果是空的话就是传输明文， 可以重新解密规则，https？ 各位爷
            virtual void decryptData(){};

            void sendBin(const char* data, uint32_t len);

            void send(std::string& strMsg, uint32_t cmd );
            /*客户端连接服务端回包接口*/
            void sendResponse(google::protobuf::Message& msg, uint32_t cmd );
            
            void sendError(uint32_t cmd, uint8_t error);    

        private:
                
            int sendHelper(const char* data, size_t size);

            IDataHandler* m_pDataHandler;
    
            uint32_t m_uPort;
            string   m_strIp;
                
            SocketBuffer m_input;
            SocketBuffer m_output;

            uint32_t m_uLastWarn;
            uint32_t m_uDropCount;

            bool m_bOverflow;

            ConnectStat m_stat;

            CloseCallback closeCallback;

            int fd;

            boost::scoped_ptr<CEvSource> ev;
    };

    typedef boost::shared_ptr<CTcpConn> TcpConnPtr;
}
#endif
