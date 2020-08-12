#ifndef CTM_SERVER_ECHO_SERVER_H__
#define CTM_SERVER_ECHO_SERVER_H__

#include "base_server.h"

namespace ctm
{
    class CEchoServer : public CBaseServer
    {
    public:
        CEchoServer();
        virtual ~CEchoServer();
        
        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnException(CConn* conn);
    };
}

#endif