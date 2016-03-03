#include<sys/time.h>
#include"async_request_dispatch.h"

using namespace znb;

AsyncRequestMfcMap::AsyncRequestMfcMap(DBConf& dbConf)
{
	for(int i = 0; i < WorkerSize; i++)
	{
		Worker* pWorker = new Worker(dbConf);
		pWorker->setMutex();
		m_workers.push_back(pWorker);
	}

	m_nDispIndex = 0;	

	start();	
}

AsyncRequestMfcMap::~AsyncRequestMfcMap()
{
	stop();
}

//启动线程
void AsyncRequestMfcMap::start()
{
	
    std::vector<Worker*>::iterator it = m_workers.begin();
    while(it != m_workers.end())
    {	
	    (*it)->start();
	    ++it;
    }
}

void AsyncRequestMfcMap::stop()
{
    for(int i = 0; i < WorkerSize; i++)
    {
        delete m_workers[i];
    }
}

void AsyncRequestMfcMap::requestDispatch(std::string &strMsg, uint32_t cmd, uint32_t requestId, uint64_t uid64, IConn* conn)
{
	std::map<uint32_t, BaseEntry*>::iterator it = m_mapCmdToEntry.find(cmd);
	if(it != m_mapCmdToEntry.end())
	{
		BaseEntry* entry = it->second;		
		google::protobuf::Message* msg = entry->unPack(strMsg, cmd, requestId, conn);
		dispatcherToWorkers(msg, entry, cmd, requestId, uid64, conn);
	}
	else
	{
		log(Error, "[AsrequestDispatch] : can't find request entry:%u", cmd);
        conn->sendError(cmd, requestId, 100);
		return;
	}
}

void AsyncRequestMfcMap::dispatcherToWorkers(google::protobuf::Message* msg, 
			BaseEntry* entry, uint32_t cmd, uint32_t requestId, uint64_t uid,  IConn* conn)
{
	RequestType req(conn, msg, cmd, requestId, uid, entry);
	
	if(conn != NULL)
	{
		int nCurrentIndex = m_nDispIndex;
		do	
		{
			// 尝试向Workers[m_nDispIndex]派遣消息	
			Worker* pWorker = m_workers[m_nDispIndex++];
			if (m_nDispIndex == WorkerSize)	
			{	
				m_nDispIndex = 0;
			}
			if (pWorker->pushRequest(req))	
			{	
				//conn->addConnUser();
				Mutex m_mutex = pWorker->getMutex();			
				MutexGuard lock(m_mutex);	
				pWorker->notify();
				return;	
			}
			else
			{
				log(Info, "message queue over flow tid:%u", (unsigned int)pthread_self());
				conn->sendError(cmd, requestId,  2);
				return;
			}
				
		} while (m_nDispIndex != nCurrentIndex);
	}
}

AsyncRequestMfcMap::Worker::Worker(DBConf& dbConf):
m_queue(10000),
_bTerminate(false),
mysql(dbConf)
{
	
}

AsyncRequestMfcMap::Worker::~Worker()
{

}

void AsyncRequestMfcMap::Worker::process()
{

	try
	{
		
		RequestType req;
		MutexGuard lock(m_cond.getMutex());
		if(m_queue.GetElementSize() == 0)
		{
			// 等待5毫秒后重试		
			m_cond.wait();	
		}
		
		if(!m_queue.popElement(req))
			return;

		struct timeval tpstart,tpend; 
		gettimeofday(&tpstart,NULL); //记录开始时间戳

		req.entry->handleMessage(req.msg, req.cmd, req.requestId, req.conn, req.uid, &mysql);
		
		delete req.msg;
		gettimeofday(&tpend,NULL); //记录结束时间戳
		uint32_t  timeuse = 1000000*(tpend.tv_sec-tpstart.tv_sec)+ tpend.tv_usec-tpstart.tv_usec; //计算差值
		log(Info, "[handleMessage] ok cmd: %u thread:%u, mysql:%p ------>>cost:%ums", req.cmd, (unsigned int)pthread_self(), &mysql, timeuse/1000);
	}
	catch(exception& e)
	{
		log(Info,"[Worker::process] GetThread::process ERROR: %s", e.what());
	}
}
