#ifndef CTM_NET_TCPCONN_H__
#define CTM_NET_TCPCONN_H__

#include <memory>
#include "common/macro.h"
#include "socket.h"
#include "io/file.h"
#include "resolve.h"

namespace ctm 
{
    class TcpConn;
    class AsynTcpConnector;
    class Acceptor;

    std::shared_ptr<TcpConn> Accept(int fd);
    std::shared_ptr<TcpConn> TcpConnect(const char* endpoint, int port);

    class TcpConn : public CFile {
        DISABLE_COPY_ASSIGN(TcpConn)
    public:
        virtual ~TcpConn() { }

        void SetNoDelay() 
        { ctm::SetNoDelay(GetFd()); }
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

        friend AsynTcpConnector;
        friend Acceptor;
        friend std::shared_ptr<TcpConn> Accept(int fd);
        friend std::shared_ptr<TcpConn> TcpConnect(const char* endpoint, int port);
    };

    std::shared_ptr<TcpConn> Accept(int fd);
    int BaseConnect(int fd, const CAddr& addr);
    std::shared_ptr<TcpConn> TcpConnect(const char* endpoint, int port);

    typedef void (*ConnCallBack)(int code, const char* message, std::shared_ptr<TcpConn> conn);

    class AsynTcpConnector : public CFile::CHandler
    {
        DISABLE_COPY_ASSIGN(AsynTcpConnector)
    public:
        AsynTcpConnector(const char* endpoint, uint16_t port, ConnCallBack cb, CPoller* poller, uint32_t timeOut = 20);
        ~AsynTcpConnector();

    private:
        void Start(); 
        void Next();

        virtual void OnRead();
        virtual void OnWrite();
        virtual void OnError();

        static void OnTimer(uint64_t timerId, uint32_t remind, void* param);

    private:
        std::string m_endpoint;
        uint16_t m_port;
        
        int m_idx;
        std::vector<CAddr> m_addrs;
        ConnCallBack m_cb;

        CPoller* m_poller;
        uint32_t m_timeout;

        std::shared_ptr<TcpConn> m_conn;
        int m_timerId;
    };
}

#endif