#include "request_dispatch.h"

using namespace znb;

RequestMfcMap::RequestMfcMap()
{
}
RequestMfcMap::~RequestMfcMap()	
{		
	for (std::map<uint32_t, BaseEntry* >::iterator it = m_mapCmdToEntry.begin(); it != m_mapCmdToEntry.end(); ++it)		
	{
		delete it->second;			
	}
}

void RequestMfcMap::requestDispatch(const char* data, uint32_t size, uint32_t cmd, uint32_t requestId, uint64_t uid64, const TcpConnPtr& conn)
{
	std::map<uint32_t, BaseEntry*>::iterator it = m_mapCmdToEntry.find(cmd);	
	if(it == m_mapCmdToEntry.end())
	{		
		log(Error, "can't find request entry cmd:%u, request:%u", cmd, requestId);
		return;		
	}		
	else		
	{			
		BaseEntry* entry = it->second;		
		entry->handleRequest(data,size, cmd, requestId, conn, uid64);			
	}	
}
