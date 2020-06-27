#include <assert.h>
#include "net/tcpserver.h"
#include "common/message.h"
#include "common/macro.h"
#include "module/timer.h"
#include "server.h"

namespace ctm
{
    CServer::CServer(const char* host, unsigned int port)
    {
        m_queue = new CCommonQueue();
        m_timer = new CTimer();
        m_tcpServer = new CTcpServer(host, port);
    }

    CServer::~CServer()
    {
        DELETE(m_queue);
        DELETE(m_tcpServer);
        DELETE(m_timer);
    }

    int CServer::Init()
    {
        if (m_tcpServer->Init() == -1)
        {
            return -1;
        }
        m_tcpServer->SetOutMessageQueue(m_queue);
        return 0;
    }

    int CServer::Start()
    {
        m_timer->Start();
        m_timer->Detach();
        m_tcpServer->OnRunning();
        CThread::Start();
        CThread::Detach();
        return 0;
    }

	int CServer::Stop()
    {
        m_timer->Stop();
        m_tcpServer->UnInit();
        CThread::Stop();
        return 0;
    }

    int CServer::Run()
    {
        while(1)
        {
            shared_ptr<CMessage> message = m_queue->GetPopFront(10);
            if (message.get() == NULL){
                continue;
            }

            switch (message->m_type)
            {
            case MSG_SYS_NULL:
                break;
            case MSG_SYS_COMMON:
                break;
            case MSG_SYS_TIMER:
            {
                shared_ptr<CTimerMessage> msg = dynamic_pointer_cast<CTimerMessage>(message);
                OnTimerMessage(msg);
            }
                break;
            case MSG_SYS_NET_DATA:
            {
                shared_ptr<CNetDataMessage> msg = dynamic_pointer_cast<CNetDataMessage>(message);
                OnNetDataMessage(msg);
            }
                break;
            case MSG_SYS_NET_CONN:
            {
                shared_ptr<CNetConnMessage> msg = dynamic_pointer_cast<CNetConnMessage>(message);
                OnNetConnMessage(msg);
            }
                break;
            default:
            {
                OnMessage(message);
            }
                break;
            }
        }
        return 0;
    }

    void CServer::OnTimerMessage(shared_ptr<CTimerMessage>& msg)
    {
        TimerCallBack cb = msg->m_cbFunction;
        CModule* object = msg->m_object;
        (object->*cb)(msg->m_timerId, msg->m_remindCount, msg->m_param);
    }

    void CServer::OnNetConnMessage(shared_ptr<CNetConnMessage>& msg)
    {


    }

    void CServer::OnNetDataMessage(shared_ptr<CNetDataMessage>& msg)
    {

    }

    void CServer::OnMessage(shared_ptr<CMessage>& msg)
    {

    }
}
