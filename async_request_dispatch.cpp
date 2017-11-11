#include<sys/time.h>
#include"async_request_dispatch.h"

using namespace znb;

AsyncRequestMfcMap::AsyncRequestMfcMap()
{
    for(int i = 0; i < WorkerSize; i++)
    {
        Worker* pWorker = new Worker();
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

void AsyncRequestMfcMap::requestDispatch(const char* pData, uint32_t size, uint32_t cmd, uint32_t requestId, uint64_t uid64, const TcpConnPtr& conn)
{
    std::map<uint32_t, BaseEntry*>::iterator it = m_mapCmdToEntry.find(cmd);
    if(it != m_mapCmdToEntry.end())
    {
        BaseEntry* entry = it->second;
        google::protobuf::Message* msg = entry->unPack( pData, size, cmd, requestId, conn);
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
            BaseEntry* entry, uint32_t cmd, uint32_t requestId, uint64_t uid,  const  TcpConnPtr& conn)
{
    RequestType req(conn, msg, cmd, requestId, uid, entry);
    
    if(conn.get())
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
            pWorker->pushRequest(req);
            {
                Mutex m_mutex = pWorker->getMutex();            
                MutexGuard lock(m_mutex);    
                pWorker->notify();
                log(Info, "[AsyncRequestMfcMap::dispatcherToWorkers] message queue size:[%lu] flow tid:%u", pWorker->getSize(), (unsigned int)pthread_self());
                return;    
            }
                
        } while (m_nDispIndex != nCurrentIndex);
    }
}

AsyncRequestMfcMap::Worker::Worker():
_bTerminate(false)
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
        if(m_queue.size() == 0)
        {
            m_cond.wait();    
        }
        log(Info, "[Work::process] queue size:%lu", m_queue.size());
    //    if(!m_queue.popElement(req))
    //        return;
        req = m_queue.front();
        m_queue.pop_front();

        struct timeval tpstart,tpend; 
        gettimeofday(&tpstart,NULL); //记录开始时间戳

        req.entry->handleMessage(req.msg, req.cmd, req.requestId, req.conn, req.uid);
        //req.entry->handleMessage(req.msg, req.cmd, req.requestId, req.conn, req.uid, &mysql);
        
        delete req.msg;
        gettimeofday(&tpend,NULL); //记录结束时间戳
        uint32_t  timeuse = 1000000*(tpend.tv_sec-tpstart.tv_sec)+ tpend.tv_usec-tpstart.tv_usec; //计算差值
        log(Info, "[handleMessage] ok cmd: %u thread:%u ------>>cost:%ums", req.cmd, (unsigned int)pthread_self(), timeuse/1000);
    }
    catch(exception& e)
    {
        log(Info,"[Worker::process] GetThread::process ERROR: %s", e.what());
    }
}
