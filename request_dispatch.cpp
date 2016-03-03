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

void RequestMfcMap::requestDispatch(std::string &strMsg, uint32_t cmd, uint32_t requestId, uint64_t uid64, IConn* conn)
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
		entry->handleRequest(strMsg, cmd, requestId, conn, uid64, NULL);			
	}	
}
