#ifndef CTM_NET_TCPINTERFACE_H__
#define CTM_NET_TCPINTERFACE_H__

namespace ctm
{
    struct Conn;
    class CTcpInterface
    {
    public:
        enum ConnOpt
        {
            CONNECT_OK = 0,
            CONNECT_ING = 1,
            CONNECT_FAIL = 2,
            CONNECT_NEW = 3,
            CONNECT_CLOSE = 4,
            CONNECT_READ_CLOSE = 5,
            CONNECT_WRITE_CLOSE = 6
        };

        CTcpInterface() {}
        virtual ~CTcpInterface(){}
        
        virtual void OnConnectOpt(const Conn* conn, int opt) = 0;
        virtual void OnRecv(const Conn* conn, const char* data, int len) = 0;
        virtual void OnSend(const Conn* conn, const char* data, int len) = 0;
    };
}
#endif