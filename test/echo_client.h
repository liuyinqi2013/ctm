#ifndef CTM_SERVER_ECHO_CLIENT_H__
#define CTM_SERVER_ECHO_CLIENT_H__

#include "connector.h"

namespace ctm
{
    class CEchoClient : public CConnector
    {
    public:
        CEchoClient();
        virtual ~CEchoClient();
        
        virtual int Init(const string& ip, unsigned int port, CLog* log = NULL);
        virtual void OnAsynConnTimeOut(CConn* conn);
        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnReadClose(CConn* conn);
        virtual void OnClose(CConn* conn);

    private:
        CConn* m_stdin;
        CConn* m_conn;
        
    public:
        int m_sendLen;
        int m_recvLen;
    };
}

#endif