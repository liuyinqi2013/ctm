#ifndef CTM_NET_NETPACKET_H__
#define CTM_NET_NETPACKET_H__

#include <string>
#include <unordered_map>
#include "socket.h"
#include "common/macro.h"
#include "common/message.h"

#define NET_PACKET_MAX_SIZE (32 * 1024 * 1024)

namespace ctm
{
    using namespace std;

    struct Conn
    {
        Conn() : fd(-1), port(0) {}
        Conn(SOCKET_T sockfd, unsigned int cliPort, const string &cliIp) : fd(sockfd), port(cliPort), ip(cliIp) {}

        SOCKET_T fd;
        unsigned int port;
        string ip;
    };

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
    int ReadPacketSize(SOCKET_T fd);
    int ReadPacketData(SOCKET_T fd, RecvBuf& buf);
    int SendPacketSize(SOCKET_T fd, int size);
    int SendPacketData(SOCKET_T fd, RecvBuf& buf);
    inline bool IsCompletePack(RecvBuf& buf) { return buf.len == buf.offset; }

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
        ~CNetConnMessage() {  }
    public:
        Conn m_conn;
        int m_opt;
    };
};

#endif