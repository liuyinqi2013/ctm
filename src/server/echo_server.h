#ifndef CTM_SERVER_ECHO_SERVER_H__
#define CTM_SERVER_ECHO_SERVER_H__

#include "connector.h"

namespace ctm
{
    class CEchoServer : public CConnector
    {
    public:
        CEchoServer();
        virtual ~CEchoServer();
        
        virtual int Init(const string& ip, unsigned int port, CLog* log = NULL);

        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnReadClose(CConn* conn);
    };
}

#endif