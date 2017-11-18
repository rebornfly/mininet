#include <signal.h>
#include "timer.h"
#include "globals.h"
#include "mainloop.h"
#include "logger.h"

using namespace znb;

static void sig_pipe(int signo)
{
    log(Info, "SIGPIPE...");
}

CMainLoop::CMainLoop()
{
}

CMainLoop::~CMainLoop()
{
}

void CMainLoop::Init()
{
    CEpoll* pSelector = new CEpoll();
    znb::Globals::SetSelector(pSelector);

    CTimerMgr* pTimerMgr = new CTimerMgr();
    pTimerMgr->Init(pSelector);
    znb::Globals::SetTimerMgr(pTimerMgr);

    // ȫ��ʱ�����
    CEnvTimer* pEnvTimer = new CEnvTimer();
    znb::Globals::SetEnvTimer(pEnvTimer);

    // ��׽SIGPIPE�ź�
    signal(SIGPIPE, sig_pipe);
}

void CMainLoop::Start()
{
    znb::Globals::GetEpoll()->netEpollRun();
}

void CMainLoop::Stop()
{
    znb::Globals::GetEpoll()->netEpollStop();
}
