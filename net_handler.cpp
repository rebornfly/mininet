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
            //need more
            break;
        }
        
        uint16_t padding = XHTONS(*((uint16_t*)data));

        uint8_t length = *((uint8_t*)(data + 2));

        uint32_t size = XHTONL( *((uint32_t*)(data + 3)));
        if(len - HEADER_SIZE < size)
        {
            //need more
            break;
        }

        data += sizeof(uint16_t);
        data += sizeof(uint8_t);
        data += sizeof(uint32_t);

        uint32_t cmd = XHTONL( *((uint32_t*)data));
        data += sizeof(uint32_t);

        uint32_t requestId = XHTONL( *((uint32_t*)data));
        data += sizeof(uint32_t);

            
        uint8_t ret = *((uint8_t*)data)    ;
        data += sizeof(uint8_t);
        

        uint64_t uid = XHTONLL( *((uint64_t*)data));
        data += sizeof(uint64_t);
    
        uint32_t reverse = length + 3 - HEADER_SIZE;
        if(reverse > 0)
        {
            data += reverse;
        }
    
        log(Info, "[onData]-----[size:%u, padding:%u, length:%u, cmd:%u, requestId:%u, reverse:%u, ret:%u, uid:%lu]", size, padding, length, cmd, requestId, reverse, ret, uid);

        if(m_pRequestCtx)
        {
            m_pRequestCtx->requestDispatch(data, size, cmd, requestId, uid, conn);
        }
        
        len -= HEADER_SIZE;    
        len -= size;
        len -= reverse;
        data += size;
        
        nProcess += size + HEADER_SIZE;
        
    }

    return nProcess;
}

