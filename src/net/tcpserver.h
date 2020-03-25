#ifndef CTM_NET_TCPSERVER_H__
#define CTM_NET_TCPSERVER_H__

#include "thread/thread.h"
#include "module/module.h"
#include "netpacket.h"

namespace ctm
{
    class CTcpSend : public CModule, protected CThread
    {
    public:
        CTcpSend();
        ~CTcpSend();

        virtual int Init();
        virtual int UnInit();
        virtual int OnRunning();

        void SendData(shared_ptr<CNetDataMessage>& netMessage) 
        {
            m_queue.PutMessage(netMessage);
        }

    protected:
        virtual int Run();

    private:
        CCommonQueue m_queue;
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
    };
}

#endif