#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tcpclient.h"
namespace ctm
{
    CTcpClient::CTcpClient(const string &ip, int port) :
        m_serverIp(ip),
        m_serverPort(port),
        m_connFd(-1),
        m_connStatus(UnConnect),
        m_sendQueue(NULL),
        m_autoReconnect(false),
        m_tryReconnectCount(default_try_reconnect_count),
        m_connectCount(0)
    {

    }

    CTcpClient::~CTcpClient()
    {

    }

    int CTcpClient::Init()
    {
        m_sendQueue = new CCommonQueue();
        return Connect();
    }

    int CTcpClient::UnInit()
    {
        Stop();
        Close();
        DELETE(m_sendQueue);
        return 0;
    }

    int CTcpClient::OnRunning()
    {
        Start();
        Detach();
        return 0;
    }

    int CTcpClient::ReConnect()
    {
        Close();
        return Connect();
    }

    int CTcpClient::SendData(char* data, int len)
    {
        if (!IsValidNetLen(len)) return -1;
        shared_ptr<CNetDataMessage> message = make_shared<CNetDataMessage>();
        message->m_conn = GetConn();
        message->m_buf = new RecvBuf(len);
        memcpy(message->m_buf->data, data, len);
        return m_sendQueue->PushBack(message);
    }

    int CTcpClient::SyncSendData(char* data, int len)
    {
        //DEBUG_LOG("SyncSendData size : %d", len);
        if (!IsValidNetLen(len)) return -1;
        int ret = SendPacketSize(m_connFd, len);
        if (ret == -1)
        {
            ERROR_LOG("send size : %d", ret);
            return -1;
        }
        return SendPacketData(m_connFd, data, len);
    }

    int CTcpClient::Connect()
    {
        m_connFd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_connFd == -1)
        {
            return -1;
        }

        ++m_connectCount;
        ClearSockError(m_connFd);
        SetNonBlock(m_connFd);
        int ret = ctm::Connect(m_connFd, m_serverIp, m_serverPort);
        if (ret < 0)
        {
            if (errno != EINPROGRESS)
            {
                m_connStatus = ConnectError;
                return -1;
            }
            m_connStatus = Connecting;
        }
        else
        {
            m_connectCount = 0;
            m_connStatus = Connected;
            SetKeepAlive(m_connFd, 10);
        }

        return 0;
    }

    void CTcpClient::ConnOptNotify(int opt)
    {
        shared_ptr<CNetConnMessage> message = make_shared<CNetConnMessage>();
        message->m_conn.fd = m_connFd;
        message->m_conn.ip = m_serverIp;
        message->m_conn.port= m_serverPort;
        message->m_opt = opt;
        if (m_outMessageQueue)
        {
            m_outMessageQueue->PushBack(message);
        }
    }

    int CTcpClient::Run()
    {
        if (m_connStatus == ConnectError || m_connStatus == UnConnect)
        {
            return -1;
        }

        fd_set rdset, wrset;
        struct timeval val = {0};
        while(1)
        {
            FD_ZERO(&rdset);
            FD_ZERO(&wrset);
            FD_SET(m_connFd, &rdset);
            if (m_connStatus == Connecting || m_sendQueue->Count() > 0)
            {
                FD_SET(m_connFd, &wrset);
            }

            val.tv_sec = 5;
            int n = select(m_connFd + 1, &rdset, &wrset, NULL, &val);
            if (n < 0)
            {
                int err = errno;
                if (err = EINTR) continue;
                fprintf(stderr, "%s:%d select error %d:%s\n", __FILE__, __LINE__, err, strerror(err));
                return -1;
            }
            else if (n > 0)
            {
                if (m_connStatus == Connecting)
                {
                    // 连接失败
                    if (FD_ISSET(m_connFd, &wrset) && FD_ISSET(m_connFd, &rdset))
                    {
                        if (CanAutoReconnect())
                        {
                            usleep(5);
                            ReConnect();
                            continue;
                        }

                        Close();
                        ConnOptNotify(CNetConnMessage::CONNECT_FAIL);
                        int err = errno;
                        m_connStatus = ConnectError;
                        fprintf(stderr, "%s:%d connect error %d:%s\n", __FILE__, __LINE__, err, strerror(err));
                        return -1;
                    }

                    SetKeepAlive(m_connFd, 10);
                    ConnOptNotify(CNetConnMessage::CONNECT_OK);
                    m_connStatus = Connected;
                    m_connectCount = 0;
                }

                if (FD_ISSET(m_connFd, &rdset))
                {
                    if (Read(m_connFd) == -1)
                    {
                        Close();
                        ConnOptNotify(CNetConnMessage::DISCONNECT);
                        if (CanAutoReconnect())
                        {
                            usleep(5);
                            ReConnect();
                        }
                    }
                }

                if (FD_ISSET(m_connFd, &wrset))
                {
                    if (Write() == -1)
                    {
                        Close();
                        ConnOptNotify(CNetConnMessage::DISCONNECT);
                        if (CanAutoReconnect())
                        {
                            usleep(5);
                            ReConnect();
                        }
                    }
                }
            }
        }

        return 0;
    }

    int CTcpClient::Read(SOCKET_T fd)
    {
        RecvBuf* buf = m_readCache.GetRecvBuf(fd);
        if (buf == NULL)
        {
            int ret = ReadPacketSize(fd);
            if (ret == -1 || (unsigned int)ret > NET_PACKET_MAX_SIZE) 
            {
                return -1;
            }

            buf = new RecvBuf(ret);
            ret = ReadPacketData(fd, *buf);
            if (ret == -1) 
            {
                return -1;
            }

            if (IsCompletePack(*buf))
            {
                shared_ptr<CNetDataMessage> message = make_shared<CNetDataMessage>();
                message->m_conn = GetConn();
                message->m_buf = buf;
                if (m_outMessageQueue) m_outMessageQueue->PushBack(message);
            }
            else
            {
                m_readCache.PutRecvBuf(fd, buf);
            }
        }
        else
        {
            int ret = ReadPacketData(fd, *buf);
            if (ret == -1) 
            {
                return -1;
            }

            if (IsCompletePack(*buf))
            {
                m_readCache.Remove(fd);
                shared_ptr<CNetDataMessage> message = make_shared<CNetDataMessage>();
                message->m_conn = GetConn();
                message->m_buf = buf;
                if (m_outMessageQueue) m_outMessageQueue->PushBack(message);
            }
        }

        return 0;
    }

    int CTcpClient::Write()
    {
        shared_ptr<CNetDataMessage> message = dynamic_pointer_cast<CNetDataMessage>(m_sendQueue->NonBlockGetFront());
        if(message.get() == NULL)
        {
            return 0;
        }

        if (message->m_buf->offset == 0)
        {
            int ret = SendPacketSize(message->m_conn.fd, message->m_buf->len);
            if (ret != 0)
            {
                ERROR_LOG("send size : %d", ret);
                m_sendQueue->PopFront();
                return -1;
            }
        }

        if (SendPacketData(message->m_conn.fd, *message->m_buf) != 0)
        {
            m_sendQueue->PopFront();
            return -1;
        }

        if (IsCompletePack(*message->m_buf))
        {
            m_sendQueue->PopFront();
        }
        else
        {
            DEBUG_LOG("len=%d, offset=%d", message->m_buf->len, message->m_buf->offset);
        }
        
        return 0;
    }

    Conn CTcpClient::GetConn()
    {
        return Conn(m_connFd, m_serverPort, m_serverIp);
    }

    bool CTcpClient::CanAutoReconnect()
    {
        return bool(m_autoReconnect && m_connectCount < m_tryReconnectCount);
    }

    void CTcpClient::Close()
    {
        if (m_connStatus != UnConnect)
        {
            close(m_connFd);
            m_connStatus = UnConnect;      
        }
    }
}