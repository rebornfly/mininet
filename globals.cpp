#include "globals.h"

using namespace znb;

CEpoll* Globals::m_pSelector = 0;
CTimerMgr* Globals::m_pTimerMgr = 0;

CEnvTimer* Globals::m_pEvnTimer = 0;


void Globals::SetSelector(CEpoll* pSelector)
{
    m_pSelector = pSelector;
}

void Globals::SetTimerMgr(CTimerMgr* pTimerMgr)
{
    m_pTimerMgr = pTimerMgr;
}

void Globals::SetEnvTimer(CEnvTimer* pEnvTimer)
{
    m_pEvnTimer = pEnvTimer;
}

CEnvTimer::CEnvTimer()
: m_tmNow(time(NULL))
{
    Globals::GetTimerMgr()->AddTimeout(this, 1000);
}

CEnvTimer::~CEnvTimer()
{
    Globals::GetTimerMgr()->RemoveTimeout(this);
}

void CEnvTimer::OnTimer()
{
    m_tmNow = time(NULL);
    Globals::GetTimerMgr()->AddTimeout(this, 1000);
}
