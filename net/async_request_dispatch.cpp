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

void AsyncRequestMfcMap::requestDispatch(const char* pData, uint32_t size, uint32_t cmd, const TcpConnPtr& conn)
{
    std::map<uint32_t, BaseEntry*>::iterator it = m_mapCmdToEntry.find(cmd);
    if(it != m_mapCmdToEntry.end())
    {
        BaseEntry* entry = it->second;
        google::protobuf::Message* msg = entry->unPack( pData, size, cmd , conn);
        dispatcherToWorkers(msg, entry, cmd, conn);
    }
    else
    {
        log(Error, "[AsrequestDispatch] : can't find request entry:%u", cmd);
        conn->sendError(cmd, 100);
        return;
    }
}

void AsyncRequestMfcMap::dispatcherToWorkers(google::protobuf::Message* msg, 
            BaseEntry* entry, uint32_t cmd,  const  TcpConnPtr& conn)
{
    RequestType req(conn, msg, cmd, entry);
    
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
            //    log(Info, "[AsyncRequestMfcMap::dispatcherToWorkers] message queue size:[%lu] flow tid:%u", pWorker->getSize(), (unsigned int)pthread_self());
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
        req = m_queue.front();
        m_queue.pop_front();

        req.entry->handleMessage(req.msg, req.cmd, req.conn );
        
        delete req.msg;
    }
    catch(exception& e)
    {
        log(Info,"[Worker::process] GetThread::process ERROR: %s", e.what());
    }
}
