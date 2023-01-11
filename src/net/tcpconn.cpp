#include <unistd.h>

#include "common/com.h"
#include "common/log.h"
#include "io/poller.h"
#include "tcpconn.h"

namespace ctm {

    std::string TcpConn::GetLocalAddr() 
    { 
        int port;
        std::string ip;
        ctm::GetSockName(GetFd(), ip, port);
        return ip + ":" + I2S(port);
    }

	std::string TcpConn::GetRemoteAddr()
    { 
        int port;
        std::string ip;
        ctm::GetPeerName(GetFd(), ip, port);
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

    int BaseConnect(int fd, const CAddr& addr)
    {
        int ret = 0;
        while(1) {
            ret = connect(fd, addr.GetAddr(), addr.GetLen());
            if (ret == 0) {
                DEBUG("connect ok. addr:%s", addr.String().c_str());
                return 0;
            }

            ret = errno;
            if (ret == EINTR) {
                continue;
            }

            break;
        }

        return ret;
    }

    std::shared_ptr<TcpConn> TcpConnect(const char* endpoint, int port)
    {
        int fd = ctm::Connect(endpoint, port);
        if (fd < 0) {
            return std::shared_ptr<TcpConn>(NULL);
        }
        return std::shared_ptr<TcpConn>(new TcpConn(fd));
    }

    AsynTcpConnector::AsynTcpConnector(const char* endpoint, uint16_t port, ConnCallBack cb, CPoller* poller, uint32_t timeOut) :
    m_endpoint(endpoint),
    m_port(port),
    m_idx(-1),
    m_addrs(std::vector<CAddr>()),
    m_cb(cb),
    m_poller(poller),
    m_timeout(timeOut),
    m_conn(NULL),
    m_timerId(0)
    {
        Start();
    }

    AsynTcpConnector::~AsynTcpConnector()
    {
        m_poller->StopTimer(m_timerId);
    }

    void AsynTcpConnector::Start()
    {
        if (CResolve::LookupIPAddr(m_endpoint.c_str(), m_addrs) < 0) {
            ERROR("create socket failed.");
            m_cb(-1, "resolve failed", std::shared_ptr<TcpConn>(NULL));
            return;
        }
        m_timerId = m_poller->AddTimer(1000 * m_timeout, 1, AsynTcpConnector::OnTimer, this);
        Next();
    }

    void AsynTcpConnector::Next()
    {
        int fd, ret;
        while (++m_idx < int(m_addrs.size()))
        {
            fd = socket(m_addrs[m_idx].SaFamily(), SOCK_STREAM, 0);
            if (fd == -1) {
                ERROR("create socket failed.");
                break;
            }

            m_addrs[m_idx].SetPort(m_port);

            auto conn = std::shared_ptr<TcpConn>(new TcpConn(fd, m_poller));
            conn->SetNonBlock();
            ret = BaseConnect(fd, m_addrs[m_idx]);
            if (ret == 0) {
                DEBUG("connect ok. addr:%s", m_addrs[m_idx].String().c_str());
                m_poller->StopTimer(m_timerId);
                m_cb(0, "ok", conn);
                return;
            } else if (ret == EINPROGRESS) {
                DEBUG("connecting. addr:%s", m_addrs[m_idx].String().c_str());
                conn->SetHandler(this);
                conn->SetEvent(EvReadWrite);
                m_conn = conn;
                return;
            }
            ERROR("connect failed. addr:%s", m_addrs[m_idx].String().c_str());
        }

        m_conn = NULL;
        m_poller->StopTimer(m_timerId);
        m_cb(-1, "connect failed", NULL); 
    }

    void AsynTcpConnector::OnRead()
    {
        DEBUG("connect failed. ip:%s", m_addrs[m_idx].String().c_str());
        m_conn->Detach();
        Next();
    }

    void AsynTcpConnector::OnWrite()
    {
        DEBUG("connect ok. ip:%s", m_addrs[m_idx].String().c_str());
        m_poller->StopTimer(m_timerId);
        m_conn->Detach();
        m_conn->SetHandler(NULL);
        m_cb(0, "ok", m_conn); 
    }

    void AsynTcpConnector::OnError()
    {
        Next();
    }

    void AsynTcpConnector::OnTimer(uint64_t timerId, uint32_t remind, void* param)
    {
        DEBUG("on time out call back. id:%d, remind:%d", timerId, remind);
        auto p = reinterpret_cast<AsynTcpConnector*>(param);

        if (p->m_conn) {
            p->m_conn->Detach();
            p->m_conn = NULL;
        }

        if (p->m_cb) {
            p->m_cb(-1, "connect timeout", NULL); 
        }
    }
}