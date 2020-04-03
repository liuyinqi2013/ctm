#ifndef CTM_NET_TCPSERVER_H__
#define CTM_NET_TCPSERVER_H__

#include "thread/thread.h"
#include "module/module.h"
#include "netpacket.h"

namespace ctm
{
    class CMutex;
    class CTcpServer;

    class CTcpSend : public CModule, protected CThread
    {
    public:
        CTcpSend(CTcpServer* tcpserver = NULL);
        ~CTcpSend();

        virtual int Init();
        virtual int UnInit();
        virtual int OnRunning();

        void SendData(shared_ptr<CNetDataMessage>& netMessage) 
        {
            m_queue.PushBack(netMessage);
        }

    protected:
        virtual int Run();

    private:
        int m_epollFd;
        CCommonQueue m_queue;
        CTcpServer* m_tcpServr;
    };

    class CTcpServer : public CModule, protected CThread
    {
    public:
        CTcpServer(const string& ip, unsigned int port);
        ~CTcpServer();

        virtual int Init();
        virtual int UnInit();
        virtual int OnRunning();

        void SendData(const Conn& conn, char* data, int len);
        bool IsValidConn(const Conn& conn);

    protected:
        virtual int Run();
        bool AddConn(Conn* conn);
        void AddEpollEvent(SOCKET_T fd);
        void DeleteClinetConn(SOCKET_T fd);
        void DelConn(SOCKET_T fd);
        void DelEpollEvent(SOCKET_T fd);
        int  Read(SOCKET_T fd);
        void ConnOptNotify(const Conn& conn, int opt);

    private:
        typedef std::unordered_map<SOCKET_T, Conn*> ConnMap;

        string m_ip;
        unsigned int m_port;
        SOCKET_T m_acceptFd;
        int m_epollFd;
        CContextCache m_contextCache;
        ConnMap m_connMap;
        CTcpSend* m_sendModule;
        CMutex* m_mutex;
    };
}

#endif