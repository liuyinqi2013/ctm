#ifndef CTM_SERVER_TEST_SERVER_H__
#define CTM_SERVER_TEST_SERVER_H__

#include "base_server.h"

namespace ctm
{
    class CTestServer : public CBaseServer
    {
    public:
        CTestServer();
        virtual ~CTestServer();
        
        virtual int Init(CLog* log = NULL);

        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        virtual void OnException(CConn* conn);

        CConn* m_pipeConn[2];
    };
}

#endif