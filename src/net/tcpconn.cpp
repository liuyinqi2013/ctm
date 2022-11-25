#include <unistd.h>

#include "event/event.h"
#include "common/com.h"
#include "common/log.h"
#include "io/io.h"
#include "tcpconn.h"

namespace ctm {

    std::string TcpConn::GetLocalAddr() 
    { 
        int port;
        std::string ip;
        ctm::GetSockName(m_fd, ip, port);
        return ip + ":" + I2S(port);
    }

	std::string TcpConn::GetRemoteAddr()
    { 
        int port;
        std::string ip;
        ctm::GetPeerName(m_fd, ip, port);
        return ip + ":" + I2S(port);
    }

    void TcpConn::Show()
    {
        DEBUG("state:%d", GetTcpState());
        DEBUG("send buf:%d", GetSendBuf());
        DEBUG("recv buf:%d", GetRecvBuf());
        DEBUG("send lowat:%d", GetSendLowat());
        DEBUG("recv lowat:%d", GetRecvLowat());
        DEBUG("local addr:%s", GetLocalAddr().c_str());
        DEBUG("remote addr:%s", GetRemoteAddr().c_str());
    }

    std::shared_ptr<TcpConn> Accept(int listenfd)
    {
        struct sockaddr_in addr;
        int fd = Accept(listenfd, &addr);
        if (fd < 0) {
            return std::shared_ptr<TcpConn>(NULL);
        }
        return std::shared_ptr<TcpConn>(new TcpConn(fd));
    }

    int TcpConnect(int fd, const IP& ip, int port)
    {
        socklen_t len = 0;
        struct sockaddr* addr = NULL;
        struct sockaddr_in addr4 = {0};
        struct sockaddr_in6 addr6 = {0};

        if (ip.IsIPv4()) {
            addr4.sin_family = AF_INET;
            addr4.sin_port = htons(port);
            addr4.sin_addr = ip.IPv4();
            
            len = sizeof(addr4);
            addr = reinterpret_cast<struct sockaddr*>(&addr4);
        } else {
            addr6.sin6_family = AF_INET6;
            addr6.sin6_port = htons(port);
            addr6.sin6_addr = ip.IPv6();

            len = sizeof(addr6);
            addr = reinterpret_cast<struct sockaddr*>(&addr6);
        }

        while(1) {
            int ret = connect(fd, addr, len);
            if (ret == 0) {
                DEBUG("connect ok. ip:%s, port:%d", ip.String().c_str(), port);
                return 0;
            }

            int errcode = errno;
            if (errcode == EINTR) {
                continue;
            }

            return errcode;
        }
        return -1;
    }

    std::shared_ptr<TcpConn> TcpConnect(const char* endpoint, int port)
    {
        int fd = ctm::Connect(endpoint, port);
        if (fd < 0) {
            return std::shared_ptr<TcpConn>(NULL);
        }
        return std::shared_ptr<TcpConn>(new TcpConn(fd));
    }

    AsynTcpConnector::AsynTcpConnector(const char* endpoint, uint16_t port, ConnCallBack cb, CEventMonitor* eventMonitor, uint32_t timeOutSecond) :
    m_endpoint(endpoint),
    m_port(port),
    m_idx(-1),
    m_sockfd(-1),
    m_ips(std::vector<IP>()),
    m_cb(cb),
    m_eventMonitor(eventMonitor),
    m_timeout(timeOutSecond),
    m_connEv(NULL),
    m_timeoutEv(NULL)
    {
        Start();
    }

    AsynTcpConnector::~AsynTcpConnector()
    {
        StopTimer();
        StopConnect();
    }

    void AsynTcpConnector::Start()
    {
        if (Resolve(m_endpoint.c_str(), m_ips) < 0) {
            m_cb(-1, "resolve failed", std::shared_ptr<TcpConn>(NULL));
            return;
        }

        m_timeoutEv = m_eventMonitor->AddTimerEvent(1000*m_timeout, 1, AsynTcpConnector::OnEventCallBack, this);

        Next();
    }

    void AsynTcpConnector::Next()
    {
        if (m_sockfd != -1) {
            close(m_sockfd);
        }

        int ret;
        while (++m_idx < int(m_ips.size()))
        {
            m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in sockAddrIn = {0};
            sockAddrIn.sin_family = AF_INET;
            sockAddrIn.sin_port = htons(m_port);
            sockAddrIn.sin_addr = m_ips[m_idx].IPv4();

            SetNonBlock(m_sockfd);

            while(1) {
                ret = connect(m_sockfd, (struct sockaddr*)&sockAddrIn, sizeof(sockAddrIn));
                if (ret == 0) {
                    DEBUG("connect ok. ip:%s, port:%d", m_ips[m_idx].String().c_str(), m_port);
                    StopTimer();
                    m_cb(0, "ok", std::shared_ptr<TcpConn>(new TcpConn(m_sockfd)));

                    return;
                }

                int errcode = errno;
                if (errcode == EINTR) {
                    continue;
                } else if (errcode == EINPROGRESS) {
                    DEBUG("connecting ok. ip:%s, port:%d", m_ips[m_idx].String().c_str(), m_port);
                    m_connEv = m_eventMonitor->AddIOEvent(m_sockfd, EVENT_READ|EVENT_WRITE, AsynTcpConnector::OnEventCallBack, this);
                    return;
                }

                ERROR("connect failed. ip:%s, port:%d", m_ips[m_idx].String().c_str(), m_port);
                close(m_sockfd);
                break;
            }
        }

        StopTimer();
        m_cb(-1, "connect failed", std::shared_ptr<TcpConn>(NULL)); 
    }

    void AsynTcpConnector::OnEventCallBack(Event* ev, int events, void* data)
    {
        DEBUG("on event call back. events:%d", events);
        AsynTcpConnector* p = reinterpret_cast<AsynTcpConnector*>(data);
        if (OR_RW(events)) {
            p->HandlerConnEv(events);
        } else {
            p->HandlerTimeOutEv();
        }
    }

    void AsynTcpConnector::HandlerConnEv(int events)
    {
        StopConnect();
        if (AND_RW(events)) {
            Next();
        } else if (WR(events)) {
            StopTimer();
            m_cb(0, "ok", std::shared_ptr<TcpConn>(new TcpConn(m_sockfd)));
        } 
    }

    void AsynTcpConnector::HandlerTimeOutEv()
    {
        StopConnect();
        m_cb(-1, "connect failed", std::shared_ptr<TcpConn>(NULL)); 
    }

    void AsynTcpConnector::StopTimer()
    {
        if (m_timeoutEv) {
            m_timeoutEv->Destroy();
            m_timeoutEv = NULL;
        }
    }

    void AsynTcpConnector::StopConnect()
    {
        if (m_connEv) {
            m_connEv->Destroy();
            m_connEv = NULL;
        }
    }
}