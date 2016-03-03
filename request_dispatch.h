#ifndef _REQUEST_DISPATCH_H_
#define _REQUEST_DISPATCH_H_

#include <map>
#include <iostream>
#include<google/protobuf/message.h>
#include "net_conn.h"
#include "../common/znb_mysql.h"

using namespace znb;
using namespace std;

class IAppContextEx
{
public:
	virtual ~IAppContextEx() {};
	virtual void requestDispatch(std::string &strMsg, uint32_t cmd, uint32_t requestId, uint64_t uid, IConn* conn) = 0;
};

class BaseEntry
{
public:
	virtual ~BaseEntry() {}	
public:	

	virtual void handleRequest(std::string &strMsg, uint32_t cmd, uint32_t requestId, IConn* conn, uint64_t uid64, MysqlDao* mysql) = 0;	
	
	virtual google::protobuf::Message* unPack(std::string &strMsg, uint32_t cmd, uint32_t requestId, IConn* conn) = 0;	
	//多线程使用
	virtual void handleMessage(google::protobuf::Message* msg, uint32_t cmd, uint32_t requestId, IConn* conn, uint64_t uid64, MysqlDao* mysql) = 0;
};

template<class ObjClass, class MsgType>
class RequestEntry : public BaseEntry
{
public:
	typedef void (ObjClass::*PFUN)(MsgType* msg, uint32_t cmd, uint32_t requestId, IConn* conn, uint64_t uid64, MysqlDao* mysql);

	RequestEntry(ObjClass* obj, PFUN pf):
	pObj(obj)
	,m_pfun(pf)
	{
	}

	~RequestEntry(){}

	void handleRequest(std::string &strMsg, uint32_t cmd, uint32_t requestId, IConn* conn, uint64_t uid64, MysqlDao* mysql)
	{
		
		MsgType msg;
	
		try
		{	
			log(Info, "handleRequest cmd:%u, size:%u", cmd, (unsigned int)strMsg.size());
			msg.ParseFromString(strMsg);
			(pObj->*m_pfun)(&msg, cmd, requestId, conn,  uid64, mysql);
		}
		catch(...)
		{
			log(Error, "[handleRequest] decode msg error cmd:%u, raw string:%s", cmd, strMsg.c_str());
		
			conn->sendError(cmd, requestId, 1);
		}
	}

	virtual google::protobuf::Message* unPack(std::string &strMsg, uint32_t cmd, uint32_t requestId,  IConn* conn) 
	{
		MsgType* msg = new MsgType;
		
		try
                {
                        msg->ParseFromString(strMsg);
			return msg;
                }
                catch(...)
                {
                        log(Error, "[unPack] decode msg error cmd:%u, raw string:%s", cmd, strMsg.c_str());

                        conn->sendError(cmd, requestId, 1);
			
			return NULL;
                }
	}	

	void  handleMessage(google::protobuf::Message* msg, uint32_t cmd, uint32_t requestId, IConn* conn, uint64_t uid64, MysqlDao* mysql)
	{
		MsgType* pMsg = static_cast<MsgType *>(msg);
		(pObj->*m_pfun)(pMsg, cmd, requestId, conn,  uid64, mysql);
	}


private:
	ObjClass* pObj;
	MsgType m_msgType;
	PFUN m_pfun;
};

class RequestMfcMap : public IAppContextEx
{
public:
	RequestMfcMap();
	~RequestMfcMap();
	//
	template<class ObjClass, class MsgType>
	void addRequestMap(ObjClass* pObj, uint32_t cmd, void (ObjClass::*pfun)(MsgType* msg, uint32_t cmd, uint32_t requestId, IConn* conn, uint64_t uid64, MysqlDao* m_mysql))
	{
		std::map<uint32_t ,BaseEntry*>::iterator it = m_mapCmdToEntry.find(cmd);
		if(it != m_mapCmdToEntry.end())
		{
			log(Error, "duplicate msg entry cmd:%u", cmd);
			return;
		}
		
		BaseEntry* msgEntry = new RequestEntry<ObjClass, MsgType>(pObj, pfun);
		m_mapCmdToEntry[cmd] = msgEntry;
	}
	
	virtual	void requestDispatch(std::string &strMsg, uint32_t cmd, uint32_t requestId, uint64_t uid64, IConn* conn);
protected:

	std::map<uint32_t, BaseEntry *> m_mapCmdToEntry;
	
};
#endif
