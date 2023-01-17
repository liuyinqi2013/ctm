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

    void TcpConnHandler::OnConnect(int code, const char* message, TcpConn* conn)
    {
        if (code) {
            ERROR("on connect failed. code:%d, message:%s", code, message);
            return;
        }
        DEBUG("on connect ok. conn:%s", conn->GetRemoteAddr().c_str());
    }

    TcpConnector::TcpConnector(CPoller* poller, TcpConnHandler* handler, const CAddr& addr, uint32_t timeOut) :
    m_poller(poller),
    m_handler(handler),
    m_idx(-1),
    m_addrs(std::vector<CAddr>()),
    m_timeout(timeOut),
    m_conn(NULL),
    m_timerId(0)
    {
        m_addrs.push_back(addr);
        Start();
    }

    TcpConnector::TcpConnector(CPoller* poller, TcpConnHandler* handler, const char* endpoint, uint16_t port, uint32_t timeOut) :
    m_poller(poller),
    m_handler(handler),
    m_idx(-1),
    m_addrs(std::vector<CAddr>()),
    m_timeout(timeOut),
    m_conn(NULL),
    m_timerId(0)
    {
        Resolve(endpoint, port);
        Start();
    }

    TcpConnector::~TcpConnector()
    {
        m_poller->StopTimer(m_timerId);
    }

    int TcpConnector::Connect(int fd, const CAddr& addr)
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

    void TcpConnector::Resolve(const char* endpoint, uint16_t port) 
    {
        if (CResolve::LookupIPAddr(endpoint, m_addrs) < 0) {
            ERROR("resolve addr failed. endpoint:%s", endpoint);
            m_handler->OnConnect(-1, "resolve failed", NULL);
            return;
        }
        for (auto it = m_addrs.begin(); it != m_addrs.end(); it++) {
            it->SetPort(port);
        }
    }

    void TcpConnector::Start()
    {
        m_timerId = m_poller->AddTimer(1000 * m_timeout, 1, TcpConnector::OnTimer, this);
        Next();
    }

    void TcpConnector::Next()
    {
        int fd, ret;
        while (++m_idx < int(m_addrs.size()))
        {
            fd = socket(m_addrs[m_idx].SaFamily(), SOCK_STREAM, 0);
            if (fd == -1) {
                ERROR("create socket failed.");
                break;
            }

            auto conn = new TcpConn(fd, m_poller);
            conn->SetNonBlock();

            ret = Connect(fd, m_addrs[m_idx]);
            if (ret == 0) {
                DEBUG("connect ok. addr:%s", m_addrs[m_idx].String().c_str());
                m_poller->StopTimer(m_timerId);
                m_handler->OnConnect(0, "ok", conn);
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
        m_handler->OnConnect(-1, "connect failed", NULL); 
    }

    void TcpConnector::OnRead(CFile* file)
    {
        DEBUG("connect failed. ip:%s", m_addrs[m_idx].String().c_str());
        m_conn->Detach();
        DELETE(m_conn);
        Next();
    }

    void TcpConnector::OnWrite(CFile* file)
    {
        DEBUG("connect ok. ip:%s", m_addrs[m_idx].String().c_str());
        m_poller->StopTimer(m_timerId);
        m_conn->Detach();
        m_conn->SetHandler(NULL);
        m_handler->OnConnect(0, "ok", m_conn); 
        m_conn = NULL;
    }

    void TcpConnector::OnError(CFile* file)
    {
        DELETE(m_conn);
        Next();
    }

    void TcpConnector::OnTimer(uint64_t timerId, uint32_t remind, void* param)
    {
        DEBUG("on time out call back. id:%d, remind:%d", timerId, remind);
        auto p = reinterpret_cast<TcpConnector*>(param);
        if (p->m_conn) {
            p->m_conn->Detach();
            DELETE(p->m_conn);
        }
        p->m_handler->OnConnect(-1, "connect timeout", NULL); 
    }

    TcpListener::TcpAcceptor::TcpAcceptor(CPoller* poller, int fd, TcpConnHandler* handler) : CFile(fd, poller), m_handler(handler)
    {
        SetHandler(this);
        SetEvent(EvRead);
    }

    void TcpListener::TcpAcceptor::OnRead(CFile* file)
    {
        if (IsClosed()) {
            m_handler->OnConnect(-1, "closed", NULL);
            return; 
        }

        int fd;
        CAddr addr;
        socklen_t len =  sizeof(addr);
        while(1) {
            fd = accept(GetFd(), addr.GetAddr(), &len);
			if (fd < 0) {
                if (errno == EINTR) continue;
                else if (errno == EAGAIN || errno == EWOULDBLOCK) return;
                m_handler->OnConnect(-1, strerror(errno), NULL);
                return;
			}

            DEBUG("accept connected. addr:%s", addr.String().c_str());

            auto conn = new TcpConn(fd, GetPoller());
            conn->SetKeepAlive(30);
            conn->SetNoDelay();
            m_handler->OnConnect(0, "ok", conn);
			return;
        }
    }

    TcpListener::TcpListener(CPoller* poller, TcpConnHandler* handler, const CAddr& addr) : m_poller(poller), m_handler(handler), m_acceptor(NULL), m_addr(addr)
    {
    }

    TcpListener::~TcpListener()
    {
        DELETE(m_acceptor);
    }

    int TcpListener::Listen()
    {
		int fd = socket(m_addr.SaFamily(), SOCK_STREAM, 0);
		if(fd < 0) {
            ERROR("create socket failed. family:%d", m_addr.SaFamily());
			return -1;
		}
		if (SetReuseAddr(fd) < 0) {
            ERROR("set reuse addr failed. fd:%d", fd);
			close(fd);
			return -1;
		}

		if(bind(fd, m_addr.GetAddr(), m_addr.GetLen()) < 0) {
            ERROR("bind failed. addr:%s", m_addr.String().c_str());
			close(fd);
			return -1;
		}

		if(listen(fd, 100) < 0) {
            ERROR("listen failed. addr:%s", m_addr.String().c_str());
			close(fd);
			return -1;
		}

        DELETE(m_acceptor);
        m_acceptor = new TcpAcceptor(m_poller, fd, m_handler);
        return 0;
    }
}