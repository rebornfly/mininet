#ifndef MAIN_LOOP_H_
#define MAIN_LOOP_H_

#include "net_epoll.h"

namespace znb
{
    class CMainLoop
    {
    public:
        CMainLoop();
        ~CMainLoop();

    public:
        void Init();
        void Start();
        void Stop();
    };
}

#endif // MAIN_LOOP_H_
