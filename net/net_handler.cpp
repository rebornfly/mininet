#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include "net_handler.h"
#include "event_source.h"
#include "net_conn.h"

using namespace znb;

CNetDataHandler::CNetDataHandler()
{

}

CNetDataHandler::~CNetDataHandler()
{
    
}

int CNetDataHandler::onData(const TcpConnPtr& conn, char* data, uint32_t len)
{
    int nProcess = 0;
    while(len > 0)
    {
        if(len < HEADER_SIZE)
        {
            break;
        }

        uint32_t size = XHTONL( *((uint32_t*)(data)));
        if(len - HEADER_SIZE < size)
        {
            break;
        }

        data += sizeof(uint32_t);

        uint32_t cmd = XHTONL( *((uint32_t*)data));
        data += sizeof(uint32_t);

        //log(Info, "[onData]-----[size:%u, padding:%u, length:%u, cmd:%u, requestId:%u, reverse:%u, ret:%u, uid:%lu]", size, padding, length, cmd, requestId, reverse, ret, uid);

        if(m_pRequestCtx)
        {
            m_pRequestCtx->requestDispatch(data, size, cmd, conn);
        }
        len -= HEADER_SIZE;
        len -= size;
        data += size;
        nProcess += size + HEADER_SIZE;
    }

    return nProcess;
}

