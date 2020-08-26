#ifndef CTM_SERVER_BASE_GMAE_H__
#define CTM_SERVER_BASE_GAME_H__

#include "connector.h"
#include "timer/timer.h"

#define MAX_PACK_LEN (32 * 1024)
#define HB_IDLE_SECOND 30

#define ECHO_PROTO_REQ_ID 0
#define ECHO_PROTO_RSP_ID 1

#define HB_PROTO_REQ_ID 2
#define HB_PROTO_RSP_ID 3

namespace ctm
{
    class CPack;
    class CBaseGame;

    typedef void (CBaseGame::*ProtoHandle)(void* data, char* buf, int len);

    class CBaseGame : public CTimerApi, public CConnector
    {
        struct ProtoInfo
        {
            CBaseGame* object;
            ProtoHandle handle;
        };
        typedef std::map<unsigned int, ProtoInfo> ProtoMap;

    public:

        CBaseGame();
        virtual ~CBaseGame();

        virtual int Init(CLog* log = NULL);
        virtual int LoopRun();

        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnReadClose(CConn* conn);
        virtual void OnWriteClose(CConn* conn);
        virtual void OnClose(CConn* conn);

        int StartListen(int port);
        int TestEcho(const string& ip, unsigned int port, char* buf);

        // 设置打包器
        void SetPacker(CPack* pack)
        {
            m_pack = pack;
        }

        // 设置心跳间隔
        void SetHBInterval(unsigned int idleSecond)
        {
            m_idleSecond = idleSecond;
        }

        // 设置心跳启停
        void SetHeartBeat(bool bStart)
        {
            m_bHeartBeat = bStart;
        }
        
    public:
    
        void RegProtoHandle(unsigned int protoId, ProtoHandle handle, CBaseGame* object = NULL);
        int StartTimer(int milliSecond, int count, TimerCallBack cb, void* param = NULL, void* param1 = NULL, void* param2 = NULL);
        int Send(CConn* conn, char* buf, unsigned int len);
        int Send(CConn* conn, unsigned int protoId, char* buf, unsigned int len);

        void Second_1(unsigned int timerId, unsigned int remindCount, void* param);
        void Second_2(unsigned int timerId, unsigned int remindCount, void* param);

        void TestEchoTimer(unsigned int timerId, unsigned int remindCount, void* param, void* param1);

        void EchoReq(void* data, char* buf, int len);
        void EchoRsp(void* data, char* buf, int len);
        void Unknown(void* data, char* buf, int len);

        void HeartBeatReq(void* data, char* buf, int len);
        void HeartBeatRsp(void* data, char* buf, int len);

    protected:

        void HandleMSG();
        void HandleTimerMSG(CTimerMessage* msg);
        void HandleProtoMSG(CConn* conn, unsigned int protoId, char* buf, int len);

        void HandleHeartBeat();

    protected:
        CTimer* m_timer;
        SafeyMsgQueue* m_msgQueue;
        ProtoMap* m_protoMap;
        CPack* m_pack;
        CConn* m_echoServConn;
        short  m_millTimeOut;
        unsigned int m_idleSecond;
        bool   m_bHeartBeat;
        unsigned int m_maxPackLen;
    };
}

#endif