#include "base_game.h"
#include "link.h"
#include "net/socket.h"
#include "net/conn.h"
#include "common/pack.h"
#include "common/log.h"
#include "common/time_tools.h"

namespace ctm
{
    CBaseGame::CBaseGame() :
    m_id(0),
    m_type(0),
    m_name(""), 
    m_timerHandler(NULL),
    m_protoMap(NULL),
    m_protoMapEx(NULL),
    m_linkManger(NULL),
    m_HBTimerId(0),
    m_HBInterval(HB_IDLE_SECOND),
    m_maxPackLen(MAX_PACK_LEN)
    {
    }

    CBaseGame::~CBaseGame()
    {
        DELETE(m_timerHandler);
        DELETE(m_protoMap);
        DELETE(m_protoMapEx);
        DELETE(m_linkManger);
    }

    int CBaseGame::Init()
    {
        if (CConnector::Init() == -1)
        {
            ERROR("CConnector init failed");
            return -1;
        }

        m_timerHandler = new CTimerHandler;
        if (m_timerHandler == NULL)
        {
            ERROR("Create timer failed");
            return -1;
        }

        m_protoMap = new ProtoMap;
        if (m_protoMap == NULL)
        {
            ERROR("Create protoMap failed");
            return -1;
        }

        m_protoMapEx = new ProtoMap;
        if (m_protoMapEx == NULL)
        {
            ERROR("Create protoMapEx failed");
            return -1;
        }

        m_linkManger = new CLinkManger;
        if (m_linkManger == NULL)
        {
            ERROR("Create linkManger failed");
            return -1;
        }

        RegProtoHandle(ECHO_PROTO_REQ_ID, &CBaseGame::EchoReq, this);
        RegProtoHandle(ECHO_PROTO_RSP_ID, &CBaseGame::EchoRsp, this);
        RegProtoHandle(HB_PROTO_REQ_ID, &CBaseGame::HeartBeatReq, this);
        RegProtoHandle(HB_PROTO_RSP_ID, &CBaseGame::HeartBeatRsp, this);

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
                    ERROR("Recv len overload max:%d len:%d [%s]", 
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
            HandleRecvData(conn, buf->data, buf->len);
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

    int CBaseGame::TestEcho(const string& ip, uint32 port, char* buf)
    {
        CConn* conn = Connect(ip, port);
        if (conn == NULL)
        {
            ERROR("Connect falied ip:%s, port:%d", ip.c_str(), port);
            return -1;
        }

        Send(conn, 0, 0, ECHO_PROTO_REQ_ID, buf, strlen(buf));

        return 0;
    }

    void CBaseGame::HandleRecvData(CConn* conn, void* buf, uint32 len)
    {
        static struct CMsgHeader headinfo;
        headinfo.Decode((char*)buf, len);
        headinfo.recvTime = MilliTimestamp();
        char* pData = (char*)buf + sizeof(headinfo);
        DEBUG("Recv common msg conn=[%s], len=%d", conn->ToString().c_str(), len);
        DEBUG("headinfo:[%s]", headinfo.ToString().c_str());
        Dispatch(conn, headinfo, pData, headinfo.dataLen);
    }

    uint32 CBaseGame::Uid() const
    {
        return UUID(m_id, m_type);
    }

    void CBaseGame::Dispatch(CConn* conn, const CMsgHeader& head, const void* data, uint32 len)
    {
        if (head.duid == 0 || head.duid == Uid() || (GET_TYPE(head.duid) == Type() && GET_ID(head.duid) == 0))
        {
            HandleProtoMSG(conn, head, (char*)data, len);
        }
        else if (GET_TYPE(head.duid) && GET_ID(head.duid))
        {
            // 转发
            CLinkConn* link = m_linkManger->GetLink(head.duid);
            if (link)
            {
                Send(link->conn, head.duid, head.suid, head.protoId, (char*)data, len);
            }
            else
            {
                DEBUG("Not find link uid:%u", head.duid);
            }
        }
        else if (GET_TYPE(head.duid) && GET_ID(head.duid) == 0)
        {   
            // 广播
            LinkIdMAP& linkMap = m_linkManger->GetLinkIdMap(GET_TYPE(head.duid));
            LinkIdMAP::iterator it = linkMap.begin();
            for (; it != linkMap.end(); it++)
            {
                Send(it->second->conn, head.duid, head.suid, head.protoId, (char*)data, len);
            }
        }
    }

    void CBaseGame::RegProtoHandle(uint32 protoId, ProtoHandle handle, CBaseGame* object)
    {
        (*m_protoMap)[protoId].handle = handle;
        (*m_protoMap)[protoId].object = object;
        (*m_protoMap)[protoId].func = NULL;
    }

    void CBaseGame::RegProtoHandle(uint32 protoId, CallBackFunc func, void* param)
    {
        (*m_protoMap)[protoId].handle = NULL;
        (*m_protoMap)[protoId].object = NULL;
        (*m_protoMap)[protoId].func = func;
        (*m_protoMap)[protoId].param = param;
    }

    int CBaseGame::Send(CConn* conn, const char* buf, uint32 len)
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

    int CBaseGame::Send(CConn* conn, uint32 duid, uint32 protoId, const char* buf, uint32 len)
    {
        return Send(conn, duid, Uid(), protoId, buf, len);
    }

    int CBaseGame::Send(CConn* conn, uint32 duid, uint32 suid, uint32 protoId, const char* buf, uint32 len)
    {
        static struct CMsgHeader head;
        static char tmpBuf[MAX_PACK_LEN];

        if (!conn)
        {
            ERROR("Not find conn NULL");
            return -1;
        }

        if (len + sizeof(head) > MAX_PACK_LEN)
        {
            ERROR("Send len overload max:%d len:%d ", MAX_PACK_LEN, len + sizeof(head));
            return -1;
        }

        head.duid = duid;
        head.suid = suid;
        head.protoId = protoId;
        head.sendTime = MilliTimestamp();
        head.dataLen = len;

        uint32 tmplen = MAX_PACK_LEN;
        head.Encode(tmpBuf, tmplen);
        memmove(tmpBuf + sizeof(head), buf, len);

        return this->Send(conn, tmpBuf, len + sizeof(head));
    }

    int CBaseGame::Send(CLinkConn* link, uint32 protoId, const char* buf, uint32 len)
    {
        return this->Send(link->conn, link->Uid(), Uid(), protoId, buf, len);
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

    void CBaseGame::StartHeartBeats(uint32 interval)
    {
        m_HBInterval = interval;
        m_HBTimerId = StartTimer(interval * 1000, -1, (TimerCallBack)&CBaseGame::HandleHeartBeat);
    }

    void CBaseGame::StopHeartBeats()
    {
        StopTimer(m_HBTimerId);
        m_HBTimerId = 0;
    }

    void CBaseGame::TestEchoTimer(uint32 timerId, uint32 remindCount, void* param, void* param1)
    {
        DEBUG("CBaseGame::TestEchoTimer timerId:%d remindCount:%d", timerId, remindCount);
        
        Send((CConn*)param, 0, Uid(), ECHO_PROTO_REQ_ID, (char*)param1, strlen((const char*)param1));
    }

    void CBaseGame::EchoReq(CConn* conn, const CMsgHeader& head, char* buf, int len)
    {
        Send(conn, 0, Uid(), ECHO_PROTO_RSP_ID, buf, len);
    }

    void CBaseGame::EchoRsp(CConn* conn, const CMsgHeader& head, char* buf, int len)
    {
        DEBUG("%s", buf);
    }

    void CBaseGame::Unknown(CConn* conn, const CMsgHeader& head, char* buf, int len)
    {
        DEBUG("Recv un known data [%s]", conn->ToString().c_str());
    }

    void CBaseGame::HeartBeatReq(CConn* conn, const CMsgHeader& head, char* buf, int len)
    {
        Send(conn, 0, Uid(), HB_PROTO_RSP_ID, buf, len);
    }

    void CBaseGame::HeartBeatRsp(CConn* conn, const CMsgHeader& head, char* buf, int len)
    {
        DEBUG("Recv heart beat [%s]", conn->ToString().c_str());
    }

    void CBaseGame::HandleProtoMSG(CConn* conn, const CMsgHeader& head, char* buf, int len)
    {
        if (m_protoMap->find(head.protoId) != m_protoMap->end())
        {
            if ((*m_protoMap)[head.protoId].func)
            {
                CallBackFunc func = (*m_protoMap)[head.protoId].func;
                func((*m_protoMap)[head.protoId].param, conn, head, buf, len);
            }
            else
            {
                CBaseGame* object  = (*m_protoMap)[head.protoId].object;
                ProtoHandle handle = (*m_protoMap)[head.protoId].handle;

                if (object == NULL) object = this;
                (object->*handle)(conn, head, buf, len);
            }
        }
        else
        {
            Unknown(conn, head, buf, len);
        }
    }

    void CBaseGame::HandleHeartBeat(uint32 timerId, uint32 remindCount, void* param)
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
                    DEBUG("Not active %us close conn [%s]", 3 * m_HBInterval, (*it1)->ToString().c_str());
                    OnClose(*it1);
                }
                else if (now - (*it1)->lastActive >= m_HBInterval)
                {
                    Send(*it1, 0, Uid(), HB_PROTO_REQ_ID, (char*)&now, sizeof(time_t));
                }
            }
        }
    }
}