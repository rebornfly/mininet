#include "thread.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>

using namespace std;
using namespace znb;

void Thread::start()
{
    int iRet;
    iRet = pthread_create(&m_idThread, NULL, Thread::run, this);
    if(iRet != 0){
        cout << "ERROR:" << strerror(errno) << endl;
        exit(0);
    }
}


void* Thread::run(void* args)
{
    
    Thread* pThis = static_cast<Thread*>(args);

     pThis->m_idThread = pthread_self();

    int iRet = pthread_detach(pThis->m_idThread);
    if(iRet != 0)
    {
        cout << "ERROR:" << strerror(errno) << endl;
        pthread_exit(NULL);
    }
    
    while(true)
    {
        pThis->process();
    }
}

void Thread::process()
{
    sleep(1);
    cout << "hello" << endl;
}
