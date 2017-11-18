#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include <stdexcept>
#include <iostream>
#include <string.h>
#include "logger.h"
using namespace std;
namespace znb{

class Mutex{

public:
    Mutex(){
        if(0 != pthread_mutex_init(&m_mutex, NULL)){
            throw runtime_error( strerror(errno) );
        }
    }

    virtual ~Mutex(){
        pthread_mutex_destroy(&m_mutex);
    }

    void lock(){
        int stat = pthread_mutex_lock(&m_mutex);
        if(0 != stat)
        {
            log(Info,"[pthread_mutex_lock] mutex:%s errno:%u stat:%d", strerror(errno), errno, stat);
                    throw runtime_error( strerror(errno) );
        }
    }

    void unlock(){
        if( 0 != pthread_mutex_unlock(&m_mutex))
            throw runtime_error( strerror(errno) );
    }
        
    bool trylock()
    {
        if(0 == pthread_mutex_trylock(&m_mutex))
            return true;
        else
        {
            return false;
        }
    }

    pthread_mutex_t * getMutexPtr(){
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;    
};


class MutexGuard{
public:
    explicit MutexGuard(Mutex& mutex , bool btrylock = false):m_mutex(mutex) {
        if(btrylock)
        {
            m_mutex.trylock();
        }
        else
        {
            m_mutex.lock();
        }
    }
    
    ~MutexGuard(){
        m_mutex.unlock();
    }

private:
    Mutex& m_mutex;
};

class Condition{
public:
        
    Condition()  {
        if(0 != pthread_cond_init(&m_cond, NULL)){
            throw runtime_error( strerror(errno) );
        }
    }

    Condition(Mutex& mutex) : m_mutex(mutex) {
        if(0 != pthread_cond_init(&m_cond, NULL)){
            throw runtime_error( strerror(errno) );
        }
    }

    virtual ~Condition(){
        pthread_cond_destroy(&m_cond);
    }
    
    void setMutex(Mutex& mutex)
    {
        m_mutex = mutex;
    }
    
    Mutex& getMutex(){
        return m_mutex;
    }


    void wait(){
        if(0 != pthread_cond_wait(&m_cond, m_mutex.getMutexPtr()))
    {
       log(Info,"[pthread_cond_wait] error:%s",strerror(errno));    
            throw runtime_error(strerror(errno));
    }
    }

    void notify(){
        if(0 != pthread_cond_signal(&m_cond))
            throw runtime_error(strerror(errno));
    }

private:
    pthread_cond_t m_cond;
    Mutex m_mutex; 
};

#define ThreadLockGuard(x) assert("ThreadLockGuard NEED initialize one object!");

class Thread{

public:
    Thread(){}
    virtual ~Thread(){} 

    void start();

    virtual void process();

private:
    static void* run(void* args);

protected:
    pthread_t m_idThread;
};
}
#endif
