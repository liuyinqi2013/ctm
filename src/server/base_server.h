#ifndef CTM_SERVER_BASE_SERVER_H__
#define CTM_SERVER_BASE_SERVER_H__

#include "net/conn.h"

namespace ctm
{
    class CLog;
    class CEventHandler;
    class CEventMonitor;

    class CBaseServer : public Action
    {
    public:
        enum
        {
            CREATE  = 0,
            INIT    = 1,
            RUNNING = 2,
            EXIT    = 3,
            EXCEPT  = 4,
        };

        CBaseServer();
        virtual ~CBaseServer();

        virtual int Init(const string& ip, unsigned int port, CLog* log = NULL);
        virtual int RunProc();

        virtual void OnAccept(CConn* conn);
        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnReadClose(CConn* conn);
        virtual void OnWriteClose(CConn* conn);
        virtual void OnClose(CConn* conn);
        virtual void OnAsynConnOk(CConn* conn);
        virtual void OnAsynConnTimeOut(CConn* conn);
        virtual void OnException(CConn* conn);

    protected:
        int InitBase();
        CConn* ListenConn(const string& ip, unsigned int port);
        CConn* ConnectConn(const string& ip, unsigned int port);

        CConn* HandleAccpet(CConn* listenConn);
        int Read(CConn* conn, char* buf, int len);

    protected:
        int m_status;
        CLog* m_log;
        CConnPool* m_connPool;
        CEventHandler* m_eventHandler;
        CEventMonitor* m_eventMonitor;
    };
}

#endif