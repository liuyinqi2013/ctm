#include "base_game.h"
#include "net/socket.h"
#include "net/conn.h"

namespace ctm
{
    CBaseGame::CBaseGame() : 
    m_timer(NULL),
    m_msgQueue(NULL),
    m_protoMap(NULL)
    {

    }

    CBaseGame::~CBaseGame()
    {
        DELETE(m_timer);
        DELETE(m_msgQueue);
        DELETE(m_protoMap);
    }

    int CBaseGame::Init(CLog* log)
    {
        if (CConnServer::Init(log) == -1)
        {
            CTM_DEBUG_LOG(m_log, "CConnServer init failed");
            return -1;
        }

        m_timer = new CTimer;
        if (m_timer == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create timer failed");
            return -1;
        }

        m_msgQueue = new SafeyMsgQueue;
        if (m_timer == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create queue failed");
            return -1;
        }

        m_protoMap = new ProtoMap;
        if (m_timer == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create protoMap failed");
            return -1;
        }

        m_timer->Start();

        return 0;
    }

    int CBaseGame::LoopRun()
    {
        while(1)
        {
            HandleMSG();
            if (-1 == Execute())
            {
                break;
            }
        }

        return 0;
    }
        
    void CBaseGame::OnRead(CConn* conn)
    {
        int ret = 0;
        Buffer buf(4096 * 2);

        for (int i = 0; i < 3; i++)
        {
            buf.offset = 0;
            ret = conn->Recv(&buf);
            if (buf.offset > 0) 
            {
                conn->AsynSend(buf.data, buf.offset);
            }

            if (ret != IO_RD_OK)
            {
                break;
            }
        }
        OnError(conn, ret);
    }

    void CBaseGame::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();

        if (conn->status == CConn::RDCLOSED && conn->sendCache.size() == 0)
        {
            CTM_DEBUG_LOG(m_log, "XXX OnWrite:[%s]", conn->ToString().c_str());
            OnClose(conn);
        }
    }

    void CBaseGame::OnReadClose(CConn* conn)
    {
        if (conn->sendCache.size() == 0) 
        {
            CTM_DEBUG_LOG(m_log, "XXX IO_RD_CLOSE:[%s]", conn->ToString().c_str());
            OnClose(conn);
        }
    }

    void CBaseGame::RegProtoHandle(int protoId, ProtoHandle handle, CBaseGame* object)
    {
        (*m_protoMap)[protoId].handle = handle;
        (*m_protoMap)[protoId].object = object;
    }

    int CBaseGame::StartTimer(int milliSecond, int count, TimerCallBack cb, void* param, void* param1, void* param2)
    {
        printf("StartTimer %x\n", this);
        return m_timer->AddTimer(milliSecond, count, cb, this, param, param1, param2, true, m_msgQueue);
    }

    void CBaseGame::Second_1(unsigned int timerId, unsigned int remindCount, void* param)
    {
        printf("Second_1 %x\n", this);
        CTM_DEBUG_LOG(m_log, "CBaseGame::Second_1 timerId:%d remindCount:%d", timerId, remindCount);
    }

    void CBaseGame::Second_2(unsigned int timerId, unsigned int remindCount, void* param)
    {
        printf("Second_2 %x\n", this);
        CTM_DEBUG_LOG(m_log, "CBaseGame::Second_2 timerId:%d remindCount:%d", timerId, remindCount);
    }

    void CBaseGame::HandleMSG()
    {
        CMessage* msg = NULL;
        m_msgQueue->GetPopFront(msg, 1);
        if (msg)
        {
          HandleTimerMSG((CTimerMessage*)msg);  
        }
    }

    void CBaseGame::HandleTimerMSG(CTimerMessage* msg)
    {
        printf("HandleTimerMSG %x\n", msg->m_object);
        CTimerApi* object  = msg->m_object;
        TimerCallBack func = msg->m_cbFunction;
        (object->*func)(msg->m_timerId, msg->m_remindCount, msg->m_param, msg->m_param1, msg->m_param2);
    }
}