#ifndef CTM_NET_TCPMOD_H__
#define CTM_NET_TCPMOD_H__

#include "thread/thread.h"
#include "module/module.h"
#include "netpacket.h"
#include "tcpinterface.h"

namespace ctm
{
    class CMutex;
    class CTcpServer;

    typedef std::unordered_map<SOCKET_T, Conn*> ConnMap;

    class CTcpMod : public CModule, protected CThread
    {
    public:
        CTcpMod(CTcpInterface* tcpInterface = NULL);
        ~CTcpMod();

        virtual int Init();
        virtual int UnInit();
        virtual int OnRunning();

        int Connect(const char* ip, int port);
        int Listen(const char* ip, int port);
        void Send(Conn* conn, char* data, int len);
        bool IsValidConn(const Conn* conn);
        void SetInterface(CTcpInterface* tcpInterface);
        void Close(Conn* conn);
        void CloseRead(Conn* conn);
        void CloseWrite(Conn* conn);

    protected:
        virtual int Run();
        int AddConn(Conn* conn);
        int DelConn(Conn* conn);
        int AddEpollEvent(Conn* conn, uint32_t events);
        int DelEpollEvent(Conn* conn);
        int UpdateEpollEvent(Conn* conn, uint32_t events);
        int Read(Conn* conn);
        int Accept(Conn* conn);
        void ConnOptNotify(const Conn* conn, int opt);
        void Clear();
    private:
        int m_epollFd;
        ConnMap m_connMap;
        ConnMap m_listenConnMap;
        CTcpInterface* m_tcpInterface;
        CMutex* m_mutex;
    };
}

#endif