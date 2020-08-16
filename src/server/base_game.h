#ifndef CTM_SERVER_BASE_GMAE_H__
#define CTM_SERVER_BASE_GAME_H__

#include "conn_server.h"
#include "timer/timer.h"

namespace ctm
{
    class CBaseGame;

    typedef void (CBaseGame::*ProtoHandle)(void* data, char* buf, int len);

    struct ProtoInfo
    {
        CBaseGame* object;
        ProtoHandle handle;
    };

    typedef std::map<int, ProtoInfo> ProtoMap;

    class CBaseGame : public CTimerApi, public CConnServer
    {
    public:
        CBaseGame();
        virtual ~CBaseGame();

        virtual int Init(CLog* log = NULL);
        virtual int LoopRun();

        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnReadClose(CConn* conn);
        
    public:
        void RegProtoHandle(int protoId, ProtoHandle handle, CBaseGame* object = NULL);
        int StartTimer(int milliSecond, int count, TimerCallBack cb, void* param = NULL, void* param1 = NULL, void* param2 = NULL);

        void Second_1(unsigned int timerId, unsigned int remindCount, void* param);
        void Second_2(unsigned int timerId, unsigned int remindCount, void* param);

    protected:

        void HandleMSG();
        void HandleTimerMSG(CTimerMessage* msg);

    protected:
        CTimer* m_timer;
        SafeyMsgQueue* m_msgQueue;
        ProtoMap* m_protoMap;
    };
}

#endif