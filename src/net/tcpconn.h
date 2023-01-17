#ifndef CTM_NET_TCPCONN_H__
#define CTM_NET_TCPCONN_H__

#include "common/macro.h"
#include "socket.h"
#include "io/file.h"
#include "resolve.h"

namespace ctm 
{
    class TcpConn;
    class TcpConnector;
    class TcpAcceptor;
    class TcpConnHandler;
    class TcpListener;

    class TcpConn : public CFile {
        DISABLE_COPY_ASSIGN(TcpConn)
    public:
        virtual ~TcpConn() { }

        void SetNoDelay() { ctm::SetNoDelay(GetFd()); }
        void SetKeepAlive(int interval) { ctm::SetKeepAlive(GetFd(), interval); }

        int GetRecvLowat() { return ctm::GetRecvLowat(GetFd()); }
	    int GetSendLowat() { return ctm::GetSendLowat(GetFd()); }
        int GetTcpState() { return GetTCPState(GetFd()); }

        int SetRecvLowat(int val) { return ctm::SetRecvLowat(GetFd(), val); }
	    int SetSendLowat(int val) { return ctm::SetSendLowat(GetFd(), val); }

	    int GetRecvBuf() { return ctm::GetRecvBuf(GetFd()); }
	    int GetSendBuf() { return ctm::GetSendBuf(GetFd()); }
	    int SetRecvBuf(int val) { return ctm::SetRecvBuf(GetFd(), val); }
	    int SetSendBuf(int val) { return ctm::SetSendBuf(GetFd(), val); }

        int GetLocalAddr(std::string& outIp, int& outPort) { return ctm::GetSockName(GetFd(), outIp, outPort); }
	    int GetRemoteAddr(std::string& outIp, int& outPort) { return ctm::GetPeerName(GetFd(), outIp, outPort); }

        std::string GetLocalAddr();
	    std::string GetRemoteAddr();
        
        void Show();
        void ShutDown(int row) { shutdown(GetFd(), row); }

    protected:
        TcpConn(int fd, CPoller* poller = NULL) : CFile(fd, poller) {}

        friend TcpConnector;
        friend TcpAcceptor;
        friend TcpListener;
    };

    class TcpConnHandler
    {
    public:
    virtual void OnConnect(int code, const char* message, TcpConn* conn);
    };
    
    class TcpConnector : public CFile::CHandler
    {
        DISABLE_COPY_ASSIGN(TcpConnector);
    public:
        TcpConnector(CPoller* poller, TcpConnHandler* handler, const CAddr& addr,  uint32_t timeOut = 20);
        TcpConnector(CPoller* poller, TcpConnHandler* handler, const char* endpoint, uint16_t port,  uint32_t timeOut = 20);
        virtual ~TcpConnector();

    private:
        int Connect(int fd, const CAddr& addr);
        void Resolve(const char* endpoint, uint16_t port);
        void Start(); 
        void Next();
        
        virtual void OnRead(CFile* file);
        virtual void OnWrite(CFile* file);
        virtual void OnError(CFile* file);
        static void OnTimer(uint64_t timerId, uint32_t remind, void* param);

    private:
        CPoller* m_poller;
        TcpConnHandler *m_handler;
        
        int m_idx;
        std::vector<CAddr> m_addrs;
        uint32_t m_timeout;

        TcpConn* m_conn;
        int m_timerId;
    };

    class TcpListener
    {
        DISABLE_COPY_ASSIGN(TcpListener)

        class TcpAcceptor : public CFile, public CFile::CHandler
        {
        public:
            TcpAcceptor(CPoller* poller, int fd, TcpConnHandler* handler);
            virtual ~TcpAcceptor() {}
        protected:
            virtual void OnRead(CFile* file);
            TcpConnHandler* m_handler;
        };

    public:
        TcpListener(CPoller* poller, TcpConnHandler* handler, const CAddr& addr);
        virtual ~TcpListener();

        int Listen();
    protected:
        CPoller* m_poller;
        TcpConnHandler* m_handler;
        TcpAcceptor* m_acceptor;
        CAddr m_addr;
    };
}

#endif