#ifndef CTM_SERVER_SERVER_H__
#define CTM_SERVER_SERVER_H__
#include "thread/thread.h"
#include "module/module.h"
#include "net/netpacket.h"

namespace ctm
{
    class CTcpServer;
    
    class CServer : public CModule, public CThread
    {
    public:
        CServer(const char* host, unsigned int port);
        virtual ~CServer();
        virtual int Init();
        virtual int Start();
		virtual int Stop();
    protected:
        virtual int Run();
        virtual void OnTimerMessage(shared_ptr<CTimerMessage>& msg);
        virtual void OnNetConnMessage(shared_ptr<CNetConnMessage>& msg);
        virtual void OnNetDataMessage(shared_ptr<CNetDataMessage>& msg);
        virtual void OnMessage(shared_ptr<CMessage>& msg);
    protected:
        CTimer *m_timer;
        CCommonQueue *m_queue;
        CTcpServer *m_tcpServer;
    };
}

#endif