#ifndef CTM_SERVER_CCONNECTOR_H__
#define CTM_SERVER_CCONNECTOR_H__

#include <set>
#include "net/conn.h"

namespace ctm
{
    class CLog;
    class CEventHandler;
    class CEventMonitor;

    class CConnector : public Action
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

        CConnector();
        virtual ~CConnector();

        virtual int Init(CLog* log = NULL);
        virtual int Execute();

    public:
        virtual void OnAccept(CConn* conn);
        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnReadClose(CConn* conn);
        virtual void OnWriteClose(CConn* conn);
        virtual void OnClose(CConn* conn);
        virtual void OnAsynConnOk(CConn* conn);
        virtual void OnAsynConnTimeOut(CConn* conn);
        virtual void OnHangUp(CConn* conn);
        virtual void OnReady(CConn* conn);
        virtual void OnError(CConn* conn, int error);
    
        CConn* Listen(const string& ip, unsigned int port);
        CConn* Connect(const string& ip, unsigned int port);
        CConn* CreateConn(int fd, int events, int status = CConn::ACTIVE, bool listen = false, int family = 0);
        int Pipe(CConn* ConnArr[2]);
        
    protected:

        int InitBase();

        int HandleIOEvent();
        int HandleReadyCConns();
        CConn* HandleAccpet(CConn* listenConn);

    protected:
        int m_status;
        CLog* m_log;
        CConnPool* m_connPool;
        CEventHandler* m_eventHandler;
        CEventMonitor* m_eventMonitor;
        std::list<CConn*> m_readyCConns;
        std::set<CConn*>  m_readyCConnsSet;
    };
}

#endif