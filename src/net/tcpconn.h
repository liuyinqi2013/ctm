#ifndef CTM_NET_TCPCONN_H__
#define CTM_NET_TCPCONN_H__

#include <memory>
#include "common/macro.h"
#include "socket.h"
#include "conn.h"
#include "resolve.h"

namespace ctm 
{
    class Event;
    class CEventMonitor;

    class TcpConn;
    class AsynTcpConnector;
    class Acceptor;

    std::shared_ptr<TcpConn> Accept(int fd);
    std::shared_ptr<TcpConn> TcpConnect(const char* endpoint, int port);

    class TcpConn : public BaseConn {
    public:
        virtual ~TcpConn() { }

        void SetNoDelay() { ctm::SetNoDelay(m_fd); }
        void SetKeepAlive(int interval) { ctm::SetKeepAlive(m_fd, interval); }

        int GetRecvLowat() { return ctm::GetRecvLowat(m_fd); }
	    int GetSendLowat() { return ctm::GetSendLowat(m_fd); }
        int GetTcpState() { return GetTCPState(m_fd); }

        int SetRecvLowat(int val) { return ctm::SetRecvLowat(m_fd, val); }
	    int SetSendLowat(int val) { return ctm::SetSendLowat(m_fd, val); }

	    int GetRecvBuf() { return ctm::GetRecvBuf(m_fd); }
	    int GetSendBuf() { return ctm::GetSendBuf(m_fd); }
	    int SetRecvBuf(int val) { return ctm::SetRecvBuf(m_fd, val); }
	    int SetSendBuf(int val) { return ctm::SetSendBuf(m_fd, val); }

        int GetLocalAddr(std::string& outIp, int& outPort) { return ctm::GetSockName(m_fd, outIp, outPort); }
	    int GetRemoteAddr(std::string& outIp, int& outPort) { return ctm::GetPeerName(m_fd, outIp, outPort); }

        std::string GetLocalAddr();
	    std::string GetRemoteAddr();
        
        void Show();

    protected:
        TcpConn(int fd) : BaseConn(fd) {}

        friend AsynTcpConnector;
        friend Acceptor;
        friend std::shared_ptr<TcpConn> Accept(int fd);
        friend std::shared_ptr<TcpConn> TcpConnect(const char* endpoint, int port);;
    };

    std::shared_ptr<TcpConn> Accept(int fd);

    int TcpConnect(int fd, const IP& ip, int port);
    std::shared_ptr<TcpConn> TcpConnect(const char* endpoint, int port);
    

    typedef void (*ConnCallBack)(int code, const char* message, std::shared_ptr<TcpConn> conn);

    class AsynTcpConnector
    {
        DISABLE_COPY_ASSIGN(AsynTcpConnector)
    public:
        AsynTcpConnector(const char* endpoint, uint16_t port, ConnCallBack cb, CEventMonitor* eventMonitor, uint32_t timeOutSecond = 10);
        ~AsynTcpConnector();

    private:
        void Start(); 
        void Next();

        void StopTimer();
        void StopConnect();

        void HandlerConnEv(int events);
        void HandlerTimeOutEv();

        static void OnEventCallBack(Event* ev, int events, void* data);
    private:
        std::string m_endpoint;
        uint16_t m_port;
        
        int m_idx;
        int m_sockfd;
        std::vector<IP> m_ips;
        ConnCallBack m_cb;

        CEventMonitor* m_eventMonitor;
        uint32_t m_timeout;

        Event* m_connEv;
        Event* m_timeoutEv;
    };
}

#endif