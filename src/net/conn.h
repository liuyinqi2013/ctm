#ifndef CTM_NET_CONN_H__
#define CTM_NET_CONN_H__

#include <stdint.h>
#include <string>
#include <list>
#include <unordered_map>
#include <netinet/in.h>
#include "common/buffer.h"
#include "common/mem_pool.h"
#include "thread/mutex.h"
#include "event/event.h"

namespace ctm
{
    using std::list;
    using std::string;
    using std::unordered_map;

    class CLog;
    class Action;
    class CMutex;
    class CConnPool;

    class CConn
    {
    public:
    
        enum
        {
            CREATE        = 0,
            CONNING       = 1,
            CONN_TIME_OUT = 3,
            ACTIVE        = 4,
            RDCLOSED      = 5,
            WRCLOSED      = 6,
            HANGUP        = 7,
            CLOSED        = 8,
        };

        int  fd;
        int  type;
        int  family;
        int  status;
        int  error;
        bool isListen;
        struct sockaddr_in localAddr;
        struct sockaddr_in peerAddr;
        struct Event event;
        list<Buffer*> sendCache; 
        bool  readable;
        bool  writable;
        CMutex mutex;

        Action* action;
        Buffer* recvBuff;
        CLog* log;
        void* data;
        void* data1;

        string LocalStrIp() const;
        string PeerStrIp() const;
        string StrFamily() const;
        uint16_t LocalPort() const;
        uint16_t PeerPort() const;
        string ToString() const;

        int AsynSend(char* buf, size_t len);
        int AsynSend(Buffer* buf);
        int OnAsynWrite();

        int Send(Buffer* buf);
        int Recv(Buffer* buf);

        void ClearCache();
        void ChangeStatus(int hanpend);

        void GetLocalAddr();
        void GetPeerAddr();

        void Reset();
        void Close();
        void CloseRead();
        void CloseWrite();

        CConn();
        ~CConn();
    };

    class Action
    {
    public:
        virtual ~Action() {}
        virtual void OnAccept(CConn* conn) = 0;
        virtual void OnRead(CConn* conn) = 0;
        virtual void OnWrite(CConn* conn) = 0;
        virtual void OnReadClose(CConn* conn) = 0;
        virtual void OnWriteClose(CConn* conn) = 0;
        virtual void OnClose(CConn* conn) = 0;
        virtual void OnAsynConnOk(CConn* conn) = 0;
        virtual void OnAsynConnTimeOut(CConn* conn) = 0;
        virtual void OnHangUp(CConn* conn) = 0;
        virtual void OnError(CConn* conn, int error) = 0;
    };

    class CConnPool
    {
    public:
        CConnPool(size_t size);
        ~CConnPool();

        CConn* Get(int fd);
        void Free(CConn* conn);

    private:
        CSimpleMemPool<CConn>* m_memPool;
        unordered_map<int, CConn*> m_connMap;
    };

    enum IO_ERROR
    {
        IO_RD_OK       = 1,
        IO_RD_AGAIN    = 2,
        IO_RD_CLOSE    = 3,
        IO_WR_OK       = 4,
        IO_WR_AGAIN    = 5,
        IO_WR_CLOSE    = 6,
        IO_EXCEPT      = 9,
        IO_NO_READ     = 10,
        IO_NO_WRITE    = 11,
    };

    int Read(int fd, Buffer* buf, int& errnum);
    int Write(int fd, Buffer* buf, int& errnum);

    enum CTYPE
    {
        C_FILE   = 0,
        C_FDIR   = 1,
        C_SOCK   = 2,
        C_FIFO   = 3,
        C_FCHR   = 4,
        C_FBLK   = 5,
        C_FLNK   = 6,
        C_FREG   = 7,
        C_OTHER  = 8,
    };

    int FileType(int fd);
}

#endif