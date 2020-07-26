#ifndef CTM_NET_TCPCLIENT_H__
#define CTM_NET_TCPCLIENT_H__

#include "thread/thread.h"
#include "module/module.h"
#include "netpacket.h"

namespace ctm
{
    class CTcpClient : public CModule, protected CThread
    {
    public:
        enum EConnStatus
        {
            UnConnect = 0,
            Connecting = 1,
            Connected = 2,
            ConnectError = 3,
        };

        static const unsigned int default_try_reconnect_count = 3;

        CTcpClient(const string& ip, int port);
        ~CTcpClient();

        virtual int Init();
        virtual int UnInit();
        virtual int OnRunning();

        int SendData(char* data, int len);
        int SyncSendData(char* data, int len);
        int Connect();
        int ReConnect();
        void SetAutoReConnect(bool flag) { m_autoReconnect = flag; }
        void SetTryReconnectCount(unsigned int count) { m_tryReconnectCount = count; }
        void ReadClose();
        void WriteClose();

    protected:
        virtual int Run();
        void ConnOptNotify(int opt);
        int  Read(SOCKET_T fd);
        int  Write();
        Conn GetConn();
        bool CanAutoReconnect();
        void Close();
    private:
        string m_serverIp;
        unsigned int m_serverPort;
        SOCKET_T m_connFd;
        int m_connStatus;
        CMessgaeQueue *m_sendQueue;
        CContextCache m_readCache;
        bool m_autoReconnect;
        int m_tryReconnectCount;
        int m_connectCount;
    };
}

#endif