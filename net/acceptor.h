#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>
#include "globals.h"

const in_addr_t kInaddrAny = INADDR_ANY;
const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

namespace znb
{
    inline uint32_t hostToNetwork32(uint32_t host32)
    {  
        return htobe32(host32); 
    }
    inline uint16_t hostToNetwork16(uint16_t host16)
    {  
        return htobe16(host16); 
    }
    struct InetAddress
    {
        InetAddress(uint16_t port = 0, bool loopbackOnly = false)
        {
            bzero(&addr_, sizeof addr_);
            addr_.sin_family = AF_INET;
            in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
            addr_.sin_addr.s_addr = hostToNetwork32(ip);
            addr_.sin_port = hostToNetwork16(port);
        }

        InetAddress(uint16_t port , string ip)
        {
            addr_.sin_family = AF_INET;
            addr_.sin_port = hostToNetwork16(port);
            ::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
        }
        void setSockAddrInet(const struct sockaddr_in& addr)
        {
            addr_ = addr;
        }
        string toIpPort() const
        {
            const int size = 64;
            char buf[size] = {0};
            ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(size));
            size_t end = ::strlen(buf);
            uint16_t port = be16toh(addr_.sin_port);
            snprintf(buf+end, size-end, ":%u", port);
            return buf;
        }

        struct sockaddr_in addr_;
    };

    class Acceptor
    {
    public:
        typedef boost::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

        explicit Acceptor(int sockfd):socketfd_(sockfd),ev(new CEvSource(sockfd))
        {
            ev->setReadCallback(boost::bind(&Acceptor::handleRead, this));

        }
        ~Acceptor()
        {
        }

        void setnewConnectionCallback(NewConnectionCallback cb)
        {
            newConnectionCallback = cb;
        }

        void handleRead();

        void listen()
        {
            Globals::GetEpoll()->netEpollAdd(ev.get(), EPOLL_READABLE);
        }

    private:

        int socketfd_;

        NewConnectionCallback newConnectionCallback;

        boost::scoped_ptr<CEvSource> ev; 

    };
}

#endif
