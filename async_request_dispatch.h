#ifndef _ASYNC_REQUESTDISPATCH_H_
#define _ASYNC_REQUESTDISPATCH_H_
#include <google/protobuf/message.h>
#include "request_dispatch.h"
#include <deque>
#include "net_conn.h"
#include "../common/znb_thread.h"

using namespace google::protobuf;

const int WorkerSize = 10;

namespace znb
{
    class AsyncRequestMfcMap : public RequestMfcMap
    {
    public:
        AsyncRequestMfcMap();
        ~AsyncRequestMfcMap();    

        void start();        
        
        void stop();        

        struct RequestType
        {    
            RequestType(TcpConnPtr pConn, Message* pMsg, uint32_t uCmd, uint32_t uRequestId, uint64_t uUid, BaseEntry* pEntry):
            conn(pConn),
            msg(pMsg),
            cmd(uCmd),
            requestId(uRequestId),
            uid(uUid),
            entry(pEntry)
            {
            }
            RequestType():msg(NULL),cmd(0),requestId(0),uid(0),entry(NULL)
            {
            }
            void operator= (RequestType& r)             
            {
                conn      = r.conn;    
                msg       = r.msg;        
                cmd       = r.cmd;    
                requestId = r.requestId;
                uid       = r.uid;
                entry     = r.entry;                
            }
            void operator= (const RequestType& r) 
            {
                conn      = r.conn;    
                msg       = r.msg;        
                cmd       = r.cmd;    
                requestId = r.requestId;    
                uid       = r.uid;
                entry     = r.entry;                
            }
            TcpConnPtr     conn;
            Message*      msg;
            uint32_t      cmd;
            uint32_t     requestId;
            uint64_t     uid;
            BaseEntry*     entry;
        };    
        
        class Worker : public Thread
        {
            friend class AsyncRequestMfcMap;
        
        public:
               Worker();
            ~Worker();

            virtual void process();
            
            void  pushRequest(const RequestType& req)
            {
                m_queue.push_back(req);    
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

            size_t getSize()
            {
                return m_queue.size();
            }
            
            typedef std::deque<RequestType> Queue;
    
            Queue m_queue;

            bool _bTerminate;

            Mutex mutex;
            
            Condition m_cond;        
                
        }; 
        
        
        void dispatcherToWorkers(google::protobuf::Message* msg,
                        BaseEntry* entry, uint32_t cmd, uint32_t requestId, uint64_t uid,  const TcpConnPtr& conn);
        
        virtual    void requestDispatch(const char* pData, uint32_t size, uint32_t cmd, uint32_t requestId, uint64_t uid64, const TcpConnPtr& conn);
        
        typedef std::vector<Worker*> Workers;
        
        Workers m_workers;
        
        int m_nDispIndex;
        
    };
}

#endif
