#ifndef CTM_SERVER_TEST_SERVER_H__
#define CTM_SERVER_TEST_SERVER_H__

#include "connector.h"

namespace ctm
{
    class CTestServer : public CConnector
    {
    public:
        CTestServer();
        virtual ~CTestServer();
        
        virtual int Init(CLog* log = NULL);

        virtual void OnRead(CConn* conn);
        virtual void OnWrite(CConn* conn);
        
        CConn* m_pipeConn[2];
    };
}

#endif