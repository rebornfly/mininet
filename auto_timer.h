#ifndef AUTOTIMER_H_
#define AUTOTIMER_H_

#include "timer.h"
#include "globals.h"

struct ICallBack
{
public:
    virtual ~ICallBack()
    {
    }
    virtual void Callback() = 0;
};

template <typename TClass>
struct TCallBack0 : public ICallBack
{
    typedef void (TClass::*TMethod)();
    TClass &_class;
    TMethod _method;

    TCallBack0(TClass &clz, TMethod &mf)
    : _class(clz), _method(mf)
    {
    }

    virtual void Callback()
    {
        (_class.*_method)();
    }
};

template <typename TClass, typename TArg1>
struct TCallBack1: public ICallBack
{
    typedef void (TClass::*TMethod)(const TArg1 &);
    TClass &_class;
    TMethod _method;
    TArg1 _arg1;

    TCallBack1(TClass &clz, TMethod &mf, const TArg1 &arg1)
    : _class(clz), _method(mf), _arg1(arg1)
    {
    }

    virtual void Callback()
    {
        (_class.*_method)(_arg1);
    }
};

class CAutoTimer : public znb::ITimer
{
public:
    CAutoTimer()
    : m_bStop(true), m_bInCallback(false), m_pCallBack(0)
    {
    }

    ~CAutoTimer()
    {
        if (m_pCallBack)
        {
            delete m_pCallBack;
            m_pCallBack = NULL;
        }
    }

public:
    void Start(int nTimeout)
    {
        if (m_pCallBack != NULL)
        {
            if (!m_bStop)
            {
                // 定时器已经启动，需要先移除
                znb::Globals::GetTimerMgr()->RemoveTimeout(this);
            }

            m_bStop = false;
            m_nTimeout = nTimeout;

            // 如果我们在OnTimer的回调中，则无需设置定时
            // 因OnTimer会自动重设定时器，此处若不加判断，则导致重复定时器
            if (!m_bInCallback)
            {
                znb::Globals::GetTimerMgr()->AddTimeout(this, nTimeout);
            }
        }
    }

    virtual void OnTimer()
    {
        if (!m_bStop && m_pCallBack)
        {
            m_bInCallback = true;
            m_pCallBack->Callback();
            m_bInCallback = false;

            // 需要再次检查停止标记，因应用程序可能在回调中停止定时器
            if (!m_bStop)
            {
                znb::Globals::GetTimerMgr()->AddTimeout(this, m_nTimeout);
            }
        }
    }

    void Stop()
    {
        m_bStop = true;
    }

    template <typename TClass>
    void Init(TClass *clz, void (TClass::*mf)())
    {
        if (m_pCallBack)
        {
            delete m_pCallBack;
            m_pCallBack = NULL;
        }
        m_pCallBack = new TCallBack0<TClass>(*clz, mf);
    }

    template <typename TClass, typename TArg1>
    void Init(TClass *clz, void (TClass::*mf)(const TArg1 &), const TArg1 &arg1)
    {
        if(m_pCallBack)
        {
            delete m_pCallBack;
            m_pCallBack = NULL;
        }
        m_pCallBack = new TCallBack1<TClass, TArg1>(*clz, mf, arg1);
    }

private:
    int m_nTimeout;
    bool m_bStop;
    bool m_bInCallback;
    ICallBack *m_pCallBack;
};

#endif // AUTOTIMER_H_
