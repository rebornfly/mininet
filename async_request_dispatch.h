#ifndef _ASYNC_REQUESTDISPATCH_H_
#define _ASYNC_REQUESTDISPATCH_H_
#include <google/protobuf/message.h>
#include "request_dispatch.h"
#include "../common/znb_thread.h"
#include "../common/znb_message_queue.h"
#include "../common/znb_mysql.h"

using namespace google::protobuf;

const int WorkerSize = 10;

namespace znb
{
	class AsyncRequestMfcMap : public RequestMfcMap
	{
	public:
		AsyncRequestMfcMap(DBConf& dbConf);
		~AsyncRequestMfcMap();	

		void start();		
		
		void stop();		

		struct RequestType
		{	
			RequestType(IConn* pConn, Message* pMsg, uint32_t uCmd, uint32_t uRequestId, uint64_t uUid, BaseEntry* pEntry):
			conn(pConn),
			msg(pMsg),
			cmd(uCmd),
			requestId(uRequestId),
            uid(uUid),
			entry(pEntry)
			{
			}
			RequestType():conn(NULL),msg(NULL),cmd(0),requestId(0),uid(0),entry(NULL)
			{
			}
			void operator= (volatile RequestType& r) 			
			{
				conn      = r.conn;	
				msg       = r.msg;		
				cmd       = r.cmd;	
				requestId = r.requestId;
                uid       = r.uid;
				entry     = r.entry;				
			}
			void operator= (const RequestType& r) volatile			
			{
				conn      = r.conn;	
				msg       = r.msg;		
				cmd       = r.cmd;	
				requestId = r.requestId;	
                uid       = r.uid;
				entry     = r.entry;				
			}
			IConn*		 conn;
			Message* 	 msg;
			uint32_t 	 cmd;
			uint32_t	 requestId;
            uint64_t     uid;
			BaseEntry*	 entry;
		};	
		
		class Worker : public Thread
		{
			friend class AsyncRequestMfcMap;
		
		public:
		   	Worker(DBConf& dbConf);
			~Worker();

			virtual void process();
			
			bool pushRequest(const RequestType& req)
			{
				return m_queue.pushElement(req);	
			}
				
			Mutex getMutex()
			{
				return mutex;
			}
			
			void setMutex()
			{
				m_cond.setMutex(mutex);
			}
				
			Condition getCond()
			{
				return m_cond;
			}
		
			void notify()
			{
				m_cond.notify();
			}
			
			typedef TMessageQueue<RequestType> Queue;
	
			Queue m_queue;

			bool _bTerminate;

			Mutex mutex;
			
			Condition m_cond;		
			
			MysqlDao mysql;	
		}; 
		
		
		void dispatcherToWorkers(google::protobuf::Message* msg,
                        BaseEntry* entry, uint32_t cmd, uint32_t requestId, uint64_t uid,  IConn* conn);
		
		virtual	void requestDispatch(std::string &strMsg, uint32_t cmd, uint32_t requestId, uint64_t uid64, IConn* conn);
		
		typedef std::vector<Worker*> Workers;
		
		Workers m_workers;
		
		int m_nDispIndex;
		
	};
}

#endif
