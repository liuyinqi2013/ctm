#include "base_game.h"
#include "net/socket.h"
#include "net/conn.h"
#include "common/pack.h"

namespace ctm
{
    static CFixedPack gFixedPack;

    CBaseGame::CBaseGame() : 
    m_timer(NULL),
    m_msgQueue(NULL),
    m_protoMap(NULL),
    m_pack(NULL),
    m_echoServConn(NULL),
    m_millTimeOut(0),
    m_idleSecond(HB_IDLE_SECOND),
    m_bHeartBeat(false),
    m_maxPackLen(MAX_PACK_LEN)
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
        if (CConnector::Init(log) == -1)
        {
            CTM_DEBUG_LOG(m_log, "CConnector init failed");
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

        m_pack = &gFixedPack;

        m_timer->Start();

        RegProtoHandle(ECHO_PROTO_REQ_ID, &CBaseGame::EchoReq, this);
        RegProtoHandle(ECHO_PROTO_RSP_ID, &CBaseGame::EchoRsp, this);
        RegProtoHandle(HB_PROTO_REQ_ID, &CBaseGame::HeartBeatReq, this);
        RegProtoHandle(HB_PROTO_RSP_ID, &CBaseGame::HeartBeatRsp, this);

        return 0;
    }

    int CBaseGame::GoRun()
    {
        while(1)
        {
            HandleMSG();
            if (-1 == Execute())
            {
                break;
            }

            if (m_bHeartBeat)
            {
                HandleHeartBeat();
            }
        }

        return 0;
    }
        
    void CBaseGame::OnRead(CConn* conn)
    {
        int ret = 0;
        uint len = 0;
        Buffer* buf = NULL;

        if (conn->recvBuff == NULL)
        {
            if (conn->head)
            {
                buf = (Buffer*)conn->head;
                conn->head = NULL;
            }
            else
            {
                buf = new Buffer(4);
            }
            
            ret = conn->Recv(buf);
            if (ret == IO_RD_OK)
            {
                memcpy(&len, buf->data, 4);
                len = htonl(len);
                delete buf;

                if (len > m_maxPackLen)
                {
                    CTM_ERROR_LOG(m_log, "Recv len overload max:%d len:%d [%s]", 
                                  m_maxPackLen, len, conn->ToString().c_str());
                    goto err;
                }

                buf = new Buffer(len);

                goto read_data;
            }
            else if (ret == IO_RD_AGAIN)
            {
                conn->head = buf;
            }
            else
            {
                delete buf;
            }

            goto err;
        }
        else 
        {
            buf = conn->recvBuff;
            conn->recvBuff = NULL;
        }

    read_data:

        ret = conn->Recv(buf);
        if (ret == IO_RD_OK)
        {
            unsigned int protoId = 0;
            int ret = m_pack->UnPack(buf, protoId, buf);
            if (ret != -1 )
            {
                buf->data[buf->len] = 0;

                HandleProtoMSG(conn, protoId, buf->data, buf->len);
            }
            else
            {
                CTM_DEBUG_LOG(m_log, "Recv can not UnPack data");
            }

            delete buf;
        }
        else if (ret == IO_RD_AGAIN)
        {
            conn->recvBuff = buf;
        }
        else
        {
            delete buf;
        }
        
    err:
        OnError(conn, ret);
    }

    void CBaseGame::OnWrite(CConn* conn)
    {
        conn->OnAsynWrite();
    }

    void CBaseGame::OnReadClose(CConn* conn)
    {
        OnClose(conn);
    }

    void CBaseGame::OnWriteClose(CConn* conn)
    {
        OnClose(conn);
    }

    void CBaseGame::OnClose(CConn* conn)
    {
        /*
            do something;
        */
       
        CConnector::OnClose(conn);
    }

    int CBaseGame::StartListen(int port)
    {
        CConn* conn = Listen("0.0.0.0", port);
        if (conn == NULL)
        {
            return -1;
        }

        return 0;
    }

    int CBaseGame::TestEcho(const string& ip, unsigned int port, char* buf)
    {
        CConn* conn = Connect(ip, port);
        if (conn == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Connect falied ip:%s, port:%d", ip.c_str(), port);
            return -1;
        }

        Send(conn, ECHO_PROTO_REQ_ID, buf, strlen(buf));

        return 0;
    }

    void CBaseGame::RegProtoHandle(unsigned int protoId, ProtoHandle handle, CBaseGame* object)
    {
        (*m_protoMap)[protoId].handle = handle;
        (*m_protoMap)[protoId].object = object;
    }

    int CBaseGame::Send(CConn* conn, char* buf, unsigned int len)
    {
        if (conn->status != CConn::ACTIVE)
        {
            return -1;
        }

        Buffer* dst = new Buffer(len + 4);
        int nlen = htonl(len);
        memmove(dst->data, &nlen, 4);
        memmove(dst->data + 4, buf, len);

        return conn->AsynSend(dst);
    }

    int CBaseGame::Send(CConn* conn, unsigned int protoId, char* buf, unsigned int len)
    {
        if (conn->status != CConn::ACTIVE)
        {
            return -1;
        }

        unsigned int retlen = 0;
        retlen = m_pack->Pack(protoId, buf, len, (void*)NULL, retlen);

        Buffer* dst = new Buffer(retlen + 4);
        retlen = htonl(retlen);
        memmove(dst->data, &retlen, 4);

        retlen = dst->len - 4;
        m_pack->Pack(protoId, buf, len, dst->data + 4, retlen);
        dst->offset = 0;

        return conn->AsynSend(dst);
    }

    int CBaseGame::StartTimer(int milliSecond, int count, TimerCallBack cb, void* param, void* param1, void* param2)
    {
        return m_timer->AddTimer(milliSecond, count, cb, this, param, param1, param2, true, m_msgQueue);
    }

    void CBaseGame::TestEchoTimer(unsigned int timerId, unsigned int remindCount, void* param, void* param1)
    {
        CTM_DEBUG_LOG(m_log, "CBaseGame::TestEchoTimer timerId:%d remindCount:%d", timerId, remindCount);
        
        CConn* conn = (CConn*)param;
        Send(conn, ECHO_PROTO_REQ_ID, (char*)param1, strlen((const char*)param1));
    }

    void CBaseGame::EchoReq(void* data, char* buf, int len)
    {
        CConn* conn = (CConn*)data;
        Send(conn, ECHO_PROTO_RSP_ID, buf, len);
    }

    void CBaseGame::EchoRsp(void* data, char* buf, int len)
    {
        CTM_DEBUG_LOG(m_log, "%s", buf);
    }

    void CBaseGame::Unknown(void* data, char* buf, int len)
    {
        CConn* conn = (CConn*)data;
        CTM_DEBUG_LOG(m_log, "Recv un known data [%s]", conn->ToString().c_str());
    }

    void CBaseGame::HeartBeatReq(void* data, char* buf, int len)
    {
        CConn* conn = (CConn*)data;
        Send(conn, HB_PROTO_RSP_ID, buf, len);
    }

    void CBaseGame::HeartBeatRsp(void* data, char* buf, int len)
    {
        CConn* conn = (CConn*)data;
        CTM_DEBUG_LOG(m_log, "Recv heart beat [%s]", conn->ToString().c_str());
    }

    void CBaseGame::HandleMSG()
    {
        CMessage* msg = NULL;

        m_msgQueue->GetPopFront(msg, m_millTimeOut);

        if (msg == NULL)
        {
            m_millTimeOut = 1;
            return; 
        }

        m_millTimeOut = 0;

        switch (msg->m_type)
        {
        case MSG_SYS_TIMER:
            HandleTimerMSG(dynamic_cast<CTimerMessage*>(msg));
            break;
        default:
            break;
        }

        if (msg->m_delete)
        {
            delete msg;
        }
    }

    void CBaseGame::HandleTimerMSG(CTimerMessage* msg)
    {
        CTimerApi* object  = msg->m_object;
        TimerCallBack func = msg->m_cbFunction;
        (object->*func)(msg->m_timerId, msg->m_remindCount, msg->m_param, msg->m_param1, msg->m_param2);
    }

    void CBaseGame::HandleProtoMSG(CConn* conn, unsigned int protoId, char* buf, int len)
    {
        if (m_protoMap->find(protoId) != m_protoMap->end())
        {
            CBaseGame* object  = (*m_protoMap)[protoId].object;
            ProtoHandle handle = (*m_protoMap)[protoId].handle;

            if (object == NULL) object = this;

            (object->*handle)(conn, buf, len);
        }
        else
        {
            Unknown(conn, buf, len);
        }
    }

    void CBaseGame::HandleHeartBeat()
    {
        time_t now = time(NULL);
        
        set<CConn*>::iterator it = m_connPool->m_connSet.begin();
        for (;it != m_connPool->m_connSet.end();)
        {
            if( (*it)->isListen == false &&
                (*it)->status == CConn::ACTIVE && 
                now - (*it)->lastActive > m_idleSecond)
            {
                set<CConn*>::iterator it1 = it++;
                Send(*it1, HB_PROTO_REQ_ID, (char*)&now, sizeof(time_t));
            }
            else
            {
                it++;
            }
        }
    }
}