#ifndef CTM_SERVER_BASE_GMAE_H__
#define CTM_SERVER_BASE_GAME_H__

#include "units.h"
#include "connector.h"
#include "timer/timerhandler.h"
#include "common/macro.h"

#define HB_IDLE_SECOND 30

namespace ctm
{
    class CLinkConn;
    class CLinkManger;
    class CBaseGame;
    class CTimerHandler;

    typedef void (CBaseGame::*ProtoHandle)(CConn* conn, const CMsgHeader& head, char* data, int len);
    typedef void (*CallBackFunc)(void* param, CConn* conn, const CMsgHeader& head, char* data, int len);

    class CBaseGame : public CTimerApi, public CConnector
    {
        struct ProtoInfo
        {
            CBaseGame* object;
            ProtoHandle handle;
            CallBackFunc func;
            void* param;
        };

        typedef std::unordered_map<uint32, ProtoInfo> ProtoMap;

    public:

        enum InnerProtoId
        {
            ECHO_PROTO_REQ_ID = 0,
            ECHO_PROTO_RSP_ID = 1,
            HB_PROTO_REQ_ID   = 2,
            HB_PROTO_RSP_ID   = 3,
            COM_MSG_PROTO_ID  = 4, // 通用消息
        };

        CBaseGame();
        virtual ~CBaseGame();

        virtual int Init();
        virtual int Run();

        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnReadClose(CConn* conn);
        virtual void OnWriteClose(CConn* conn);
        virtual void OnClose(CConn* conn);
        
        uint32 Uid() const;
        uint32 Id() { return m_id; } const
        uint32 Type() { return m_type; } const
        string Name() { return m_name; } const

        void SetId(uint32 id) { m_id = id; } 
        void SetType(uint32 type) { m_type = type; }
        void SetName(const string& name) { m_name = name; }
        void SetMaxPackLen(uint32 maxLen) { m_maxPackLen = maxLen; }

        int StartListen(int port);
        int TestEcho(const string& ip, uint32 port, char* buf);

        virtual void HandleRecvData(CConn* conn, void* buf, uint32 len);
        virtual void Dispatch(CConn* conn, const CMsgHeader& head, const void* data, uint32 len);

    public:

        void RegProtoHandle(uint32 protoId, ProtoHandle handle, CBaseGame* object = NULL);
        void RegProtoHandle(uint32 protoId, CallBackFunc func, void* param = NULL);

        int StartTimer(int milliSecond, int count, TimerCallBack cb, void* param = NULL, void* param1 = NULL, void* param2 = NULL);
        int StartTimer(int milliSecond, int count, TimerCallBackEx cb, void* param = NULL, void* param1 = NULL, void* param2 = NULL);
        int StopTimer(int timerId);

        void StartHeartBeats(uint32 interval);
        void StopHeartBeats();

        int Send(CConn* conn, const char* buf, uint32 len);
        int Send(CConn* conn, uint32 duid, uint32 protoId, const char* buf, uint32 len);
        int Send(CConn* conn, uint32 duid, uint32 suid, uint32 protoId, const char* buf, uint32 len);
        int Send(CLinkConn* link, uint32 protoId, const char* buf, uint32 len);
        int Send(uint32 duid, uint32 suid, uint32 protoId, const char* buf, uint32 len);

        void TestEchoTimer(uint32 timerId, uint32 remindCount, void* param, void* param1);

    protected:
        void EchoReq(CConn* conn, const CMsgHeader& head, char* buf, int len);
        void EchoRsp(CConn* conn, const CMsgHeader& head, char* buf, int len);
        void Unknown(CConn* conn, const CMsgHeader& head, char* buf, int len);

        void HeartBeatReq(CConn* conn, const CMsgHeader& head, char* buf, int len);
        void HeartBeatRsp(CConn* conn, const CMsgHeader& head, char* buf, int len);

        void HandleProtoMSG(CConn* conn, const CMsgHeader& head, char* buf, int len);
        void HandleHeartBeat(uint32 timerId, uint32 remindCount, void* param);

    protected:
        uint32 m_id;
        uint32 m_type;
        string m_name;
        CTimerHandler* m_timerHandler;
        ProtoMap* m_protoMap;
        ProtoMap* m_protoMapEx;
        CLinkManger* m_linkManger;
        int m_HBTimerId;
        uint32 m_HBInterval;
        uint32 m_maxPackLen;
    };
}

#endif