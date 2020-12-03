#include "base_game.h"
#include "net/socket.h"
#include "net/conn.h"
#include "common/pack.h"
#include "manger.h"

namespace ctm
{
    static CFixedPack gFixedPack;

    CBaseGame::CBaseGame() : 
    m_timerHandler(NULL),
    m_protoMap(NULL),
    m_pack(NULL),
    m_echoServConn(NULL),
    m_connManger(NULL),
    m_HBTimerId(0),
    m_HBInterval(HB_IDLE_SECOND),
    m_maxPackLen(MAX_PACK_LEN)
    {

    }

    CBaseGame::~CBaseGame()
    {
        DELETE(m_timerHandler);
        DELETE(m_protoMap);
        DELETE(m_connManger);
    }

    int CBaseGame::Init(CLog* log)
    {
        if (CConnector::Init(log) == -1)
        {
            CTM_DEBUG_LOG(m_log, "CConnector init failed");
            return -1;
        }

        m_timerHandler = new CTimerHandler;
        if (m_timerHandler == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create timer failed");
            return -1;
        }

        m_protoMap = new ProtoMap;
        if (m_protoMap == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create protoMap failed");
            return -1;
        }

        m_connManger = new CConnManger;
        if (m_connManger == NULL)
        {
            CTM_DEBUG_LOG(m_log, "Create connManger failed");
            return -1;
        }

        m_pack = &gFixedPack;

        RegProtoHandle(ECHO_PROTO_REQ_ID, &CBaseGame::EchoReq, this);
        RegProtoHandle(ECHO_PROTO_RSP_ID, &CBaseGame::EchoRsp, this);
        RegProtoHandle(HB_PROTO_REQ_ID, &CBaseGame::HeartBeatReq, this);
        RegProtoHandle(HB_PROTO_RSP_ID, &CBaseGame::HeartBeatRsp, this);
        RegProtoHandle(COM_MSG_PROTO_ID, &CBaseGame::OnCommonMsg, this);

        return 0;
    }

    int CBaseGame::Run()
    {
        while(1)
        {
            if (-1 == Execute()) break;
            m_timeOut = m_timerHandler->HandleTimeOuts();
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
                    ret = IO_EXCEPT;
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

    unsigned int CBaseGame::GetServerId()
    {
        return 0;
    }
    
    unsigned int CBaseGame::GetServerType()
    {
        return 0;
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

    void CBaseGame::Dispatch(CConn* conn, const CMsgHeader& head, const void* data, unsigned int len)
    {

    }

    void CBaseGame::RegProtoHandle(unsigned int protoId, ProtoHandle handle, CBaseGame* object)
    {
        (*m_protoMap)[protoId].handle = handle;
        (*m_protoMap)[protoId].object = object;
        (*m_protoMap)[protoId].func = NULL;
    }

    void CBaseGame::RegProtoHandle(unsigned int protoId, CallBackFunc func, void* param)
    {
        (*m_protoMap)[protoId].handle = NULL;
        (*m_protoMap)[protoId].object = NULL;
        (*m_protoMap)[protoId].func = func;
        (*m_protoMap)[protoId].param = param;
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
        if (conn == NULL || conn->status != CConn::ACTIVE)
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

    int CBaseGame::Send(CConn* conn, unsigned int dstId, unsigned int srcId, unsigned int uid, unsigned int protoId, char* buf, unsigned int len)
    {
        static struct CMsgHeader head;
        static char tmpBuf[MAX_PACK_LEN];

        if (!conn)
        {
            CTM_DEBUG_LOG(m_log, "Not find conn NULL");
            return -1;
        }

        if (len + sizeof(head) > MAX_PACK_LEN)
        {
            CTM_DEBUG_LOG(m_log, "Send len overload max:%d len:%d ", MAX_PACK_LEN, len + sizeof(head));
            return -1;
        }

        head.dstId = dstId;
        head.srcId = srcId;
        head.uid = uid;
        head.msgId = protoId;
        head.dataLen = len;

        unsigned int tmplen = MAX_PACK_LEN;
        head.Encode(tmpBuf, tmplen);
        memmove(tmpBuf + sizeof(head), buf, len);

        return this->Send(conn, COM_MSG_PROTO_ID, tmpBuf, len + sizeof(head));
    }

    int CBaseGame::SendEx(unsigned int dstId, unsigned int protoId, char* buf, unsigned int len)
    {
        CConn* conn = m_connManger->GetConnById(dstId);
        return this->Send(conn, dstId, GetServerId(), 0, protoId, buf, len);
    }
    
    int CBaseGame::StartTimer(int milliSecond, int count, TimerCallBack cb, void* param, void* param1, void* param2)
    {
        return m_timerHandler->AddTimer(milliSecond, count, cb, this, param, param1, param2, false, NULL);
    }

    int CBaseGame::StartTimer(int milliSecond, int count, TimerCallBackEx cb, void* param, void* param1, void* param2)
    {
        return m_timerHandler->AddTimer(milliSecond, count, cb, param, param1, param2, false, NULL);
    }

    int CBaseGame::StopTimer(int timerId)
    {
        return m_timerHandler->StopTimer(timerId);
    }

    void CBaseGame::StartHeartBeats(unsigned int interval)
    {
        m_HBInterval = interval;
        m_HBTimerId = StartTimer(interval * 1000, -1, (TimerCallBack)&CBaseGame::HandleHeartBeat);
    }

    void CBaseGame::StopHeartBeats()
    {
        StopTimer(m_HBTimerId);
        m_HBTimerId = 0;
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

    void CBaseGame::OnCommonMsg(void* data, char* buf, int len)
    {
        CConn* conn = (CConn*)data;
        static struct CMsgHeader headinfo;
        headinfo.Decode(buf, len);
        char* pData = buf + sizeof(headinfo);
        CTM_DEBUG_LOG(m_log, "Recv common msg conn=[%s], len=%d", conn->ToString().c_str(), len);
        CTM_DEBUG_LOG(m_log, "headinfo:[%s]", headinfo.ToString().c_str());
        Dispatch(conn, headinfo, pData, headinfo.dataLen);
    }

    void CBaseGame::HandleProtoMSG(CConn* conn, unsigned int protoId, char* buf, int len)
    {
        if (m_protoMap->find(protoId) != m_protoMap->end())
        {
            if ((*m_protoMap)[protoId].func)
            {
                CallBackFunc func = (*m_protoMap)[protoId].func;
                func((*m_protoMap)[protoId].param, conn, buf, len);
            }
            else
            {
                CBaseGame* object  = (*m_protoMap)[protoId].object;
                ProtoHandle handle = (*m_protoMap)[protoId].handle;

                if (object == NULL) object = this;
                (object->*handle)(conn, buf, len);
            }
        }
        else
        {
            Unknown(conn, buf, len);
        }
    }

    void CBaseGame::HandleHeartBeat(unsigned int timerId, unsigned int remindCount, void* param)
    {
        time_t now = time(NULL);
        set<CConn*>::iterator it = m_connPool->m_connSet.begin();
        for (;it != m_connPool->m_connSet.end();)
        {
            set<CConn*>::iterator it1 = it++;
            if((*it1)->isListen == false && (*it1)->status == CConn::ACTIVE)
            {
                if (now - (*it1)->lastActive >= 3 * m_HBInterval)
                {
                    CTM_DEBUG_LOG(m_log, "Not active %us close conn [%s]", 3 * m_HBInterval, (*it1)->ToString().c_str());
                    OnClose(*it1);
                }
                else if (now - (*it1)->lastActive >= m_HBInterval)
                {
                    Send(*it1, HB_PROTO_REQ_ID, (char*)&now, sizeof(time_t));
                }
            }
        }
    }
}