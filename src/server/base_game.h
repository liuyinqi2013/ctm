#ifndef CTM_SERVER_BASE_GMAE_H__
#define CTM_SERVER_BASE_GAME_H__

#include "units.h"
#include "connector.h"
#include "timer/timerhandler.h"

#define HB_IDLE_SECOND 30

namespace ctm
{
    class CPack;
    class CBaseGame;
    class CConnManger;

    typedef void (CBaseGame::*ProtoHandle)(void* data, char* buf, int len);
    typedef void (*CallBackFunc)(void* param, void* data, char* buf, int len);

    class CBaseGame : public CTimerApi, public CConnector
    {
        struct ProtoInfo
        {
            CBaseGame* object;
            ProtoHandle handle;
            CallBackFunc func;
            void* param;
        };

        typedef std::map<unsigned int, ProtoInfo> ProtoMap;

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

        virtual int Init(CLog* log = NULL);
        virtual int Run();

        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnReadClose(CConn* conn);
        virtual void OnWriteClose(CConn* conn);
        virtual void OnClose(CConn* conn);

        virtual unsigned int GetServerId();
        virtual unsigned int GetServerType();

        int StartListen(int port);
        int TestEcho(const string& ip, unsigned int port, char* buf);

        void SetPacker(CPack* pack) { m_pack = pack; }
        void SetMaxPackLen(unsigned int maxLen) { m_maxPackLen = maxLen; }

        virtual void Dispatch(CConn* conn, const CMsgHeader& head, const void* data, unsigned int len);

    public:

        void RegProtoHandle(unsigned int protoId, ProtoHandle handle, CBaseGame* object = NULL);
        void RegProtoHandle(unsigned int protoId, CallBackFunc func, void* param = NULL);

        int StartTimer(int milliSecond, int count, TimerCallBack cb, void* param = NULL, void* param1 = NULL, void* param2 = NULL);
        int StartTimer(int milliSecond, int count, TimerCallBackEx cb, void* param = NULL, void* param1 = NULL, void* param2 = NULL);
        int StopTimer(int timerId);

        void StartHeartBeats(unsigned int interval);
        void StopHeartBeats();

        int Send(CConn* conn, char* buf, unsigned int len);
        int Send(CConn* conn, unsigned int protoId, char* buf, unsigned int len);
        int Send(CConn* conn, unsigned int dstId, unsigned int srcId, unsigned int uid, unsigned int protoId, char* buf, unsigned int len);

        int SendEx(unsigned int dstId, unsigned int protoId, char* buf, unsigned int len);
        
        void TestEchoTimer(unsigned int timerId, unsigned int remindCount, void* param, void* param1);

    protected:
        void EchoReq(void* data, char* buf, int len);
        void EchoRsp(void* data, char* buf, int len);
        void Unknown(void* data, char* buf, int len);

        void HeartBeatReq(void* data, char* buf, int len);
        void HeartBeatRsp(void* data, char* buf, int len);
        void OnCommonMsg(void* data, char* buf, int len);

        void HandleProtoMSG(CConn* conn, unsigned int protoId, char* buf, int len);
        void HandleHeartBeat(unsigned int timerId, unsigned int remindCount, void* param);

    protected:
        CTimerHandler* m_timerHandler;
        ProtoMap* m_protoMap;
        CPack* m_pack;
        CConn* m_echoServConn;
        CConnManger* m_connManger;
        int m_HBTimerId;
        unsigned int m_HBInterval;
        unsigned int m_maxPackLen;
    };
}

#endif