#ifndef CTM_NET_NETPACKET_H__
#define CTM_NET_NETPACKET_H__

#include <string>
#include <unordered_map>
#include "socket.h"
#include "common/com.h"
#include "common/macro.h"
#include "common/message.h"

#define NET_PACKET_MIN_SIZE (1)
#define NET_PACKET_MAX_SIZE (32 * 1024 * 1024)

namespace ctm
{
    using namespace std;

    struct Conn
    {
        enum EConnStatus
        {
            ConnectCreate = 0,
            Connecting = 1,
            Connected = 2,
            ConnectListen = 3,
            ConnectReadClose = 5,
            ConnectWriteClose = 6,
            ConnectClose = 7,
            ConnectError = 99,
        };

        Conn() : 
            fd(-1),
            port(0), 
            ip(""), 
            status(ConnectCreate) 
        {
        }

        Conn(SOCKET_T sockfd, unsigned int cliPort, const string &cliIp, int istatus = ConnectCreate) :
            fd(sockfd), 
            port(cliPort), 
            ip(cliIp), 
            status(istatus) 
        {
        }

        Conn(const Conn& rhs) : 
            fd(rhs.fd), 
            port(rhs.port), 
            ip(rhs.ip), 
            status(rhs.status) 
        {
        }

        Conn& operator= (const Conn& rhs) 
        { 
            fd = rhs.fd; 
            port = rhs.port; 
            ip = rhs.ip; 
            status = rhs.status;
            return *this; 
        }

        string ToString() 
        {
            return "fd:" + I2S(fd) + ",port:" + I2S(port) + ",ip:" + ip + ",status:" + I2S(status);
        }

        SOCKET_T fd;
        unsigned int port;
        string ip;
        unsigned int m_localPort;
        string m_localIp;
        int status;
    };

    inline bool operator == (const Conn& lhs, const Conn& rhs)
    {
        return bool(lhs.fd == rhs.fd && lhs.port == rhs.port && lhs.ip == rhs.ip && lhs.status == rhs.status);
    }

    /*接收网络数据的buf*/
    struct RecvBuf
    {
        RecvBuf() : len(0), offset(0), data(NULL) {}
        RecvBuf(unsigned int size) : len(size), offset(0), data(new char[size + 1]) {}
        ~RecvBuf() { DELETE_ARRAY(data); }

        unsigned int len;
        unsigned int offset;
        char *data;
    };

    /*接收网络数据上下文缓存*/
    class CContextCache
    {
        DISABLE_COPY_ASSIGN(CContextCache);
    public:
        CContextCache() {}
        ~CContextCache() { Clear(); };
        RecvBuf *GetRecvBuf(SOCKET_T fd);
        void PutRecvBuf(SOCKET_T fd, RecvBuf *data);
        void DeleteRecvBuf(SOCKET_T fd);
        void Clear();

        void Remove(SOCKET_T fd);

    private:
        typedef std::unordered_map<SOCKET_T, RecvBuf *> RecvNetDataMap;
        RecvNetDataMap m_dataMap;
    };

    RecvBuf *CreateRecvBuf(unsigned int len);
    void DestroyRecvBuf(RecvBuf *&buf);
    int ReadPacketSize(SOCKET_T fd, unsigned long millisec = -1);
    int ReadPacketData(SOCKET_T fd, RecvBuf& buf);
    int SendPacketSize(SOCKET_T fd, int size, unsigned long millisec = -1);
    int SendPacketData(SOCKET_T fd, RecvBuf& buf);
    int SendPacketData(SOCKET_T fd, const char* data, int len);
    inline bool IsCompletePack(RecvBuf& buf) { return buf.len == buf.offset; }
    inline bool IsValidNetLen(unsigned int len) { return (NET_PACKET_MIN_SIZE <= len && len <= NET_PACKET_MAX_SIZE); }
    int WaitReadable(SOCKET_T fd, unsigned long millisec = -1);
    int WaitWritable(SOCKET_T fd, unsigned long millisec = -1);

    class CNetDataMessage : public CMessage
    {
    public:
        CNetDataMessage() : CMessage(MSG_SYS_NET_DATA), m_conn(), m_buf(NULL) {}
        ~CNetDataMessage() { DELETE(m_buf); }
    public:
        Conn m_conn;
        RecvBuf* m_buf;
    };

    class CNetConnMessage : public CMessage
    {
    public:
        enum ConnOpt
        {
            CONNECT_OK = 0,
            CONNECT_FAIL = 1,
            DISCONNECT = 2,
        };
        CNetConnMessage() : CMessage(MSG_SYS_NET_CONN), m_opt(CONNECT_OK) {}
        ~CNetConnMessage() {}
    public:
        Conn m_conn;
        int m_opt;
    };
};

#endif