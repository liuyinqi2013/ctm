#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "socket.h"
#include "conn.h"
#include "memconn.h"

namespace ctm
{
    string CConn::LocalStrIp() const
    {
        char buf[32] = {0};
        inet_ntop(family, &localAddr.sin_addr.s_addr, buf, sizeof(buf));

        return string(buf);
    }

    string CConn::PeerStrIp() const
    {
        char buf[32] = {0};
        inet_ntop(family, &peerAddr.sin_addr.s_addr, buf, sizeof(buf));

        return string(buf);
    }

    string CConn::StrFamily() const
    {
        switch (family)
        {
        case AF_INET:
            return "AF_INET";
        case AF_UNIX:
            return "AF_UNIX";
        case AF_INET6:
            return "AF_INET6";
        default:
            return "Other";
        }

        return "UnKnown";
    }


    uint16_t CConn::LocalPort() const
    {
        return htons(localAddr.sin_port);
    }

    uint16_t CConn::PeerPort() const
    {
        return htons(peerAddr.sin_port);
    }

    string CConn::ToString() const
    {
        char buf[128] = {0};
        snprintf(buf, sizeof(buf) - 1, "fd:%d,type:%d,family:%s,status:%d,listen:%d,local:%s:%d,peer:%s:%d",
        fd, type, StrFamily().c_str(), status, isListen, 
        LocalStrIp().c_str(), LocalPort(),
        PeerStrIp().c_str(), PeerPort());

        return string(buf);
    }

    int CConn::AsynSend(char* buf, size_t len)
    {
        if (buf == NULL || len == 0)
        {
            return -1;
        }

        Buffer* bbuf = new Buffer(len);
        memcpy(bbuf->data, buf, len);

        return AsynSend(bbuf);
    }

    int CConn::AsynSend(Buffer* buf)
    {
        if (buf == NULL)
        {
            return -1;
        }

        if (status == WRCLOSED || status >= HANGUP)
        {
            delete buf;
            return IO_NO_WRITE;
        }

        sendCache.push_back(buf);

        if (!(event.events & EVENT_WRITE) && event.monitor)
        {
            event.monitor->AddEvent(&event, EVENT_WRITE);
        }

        if (writable)
        {
            OnAsynWrite();
        }

        return 0;
    }

    int CConn::OnAsynWrite()
    {
        int cnt = 2;
        int ret = 0;
        Buffer* buf = NULL;

        for(int i = 0; i < cnt; i++)
        {
            if (sendCache.size() == 0)
            {
                if (!(event.events & EVENT_EPOLL_ET) && event.monitor) 
                {
                    event.monitor->DelEvent(&event, EVENT_WRITE);
                }
                    
                writable = true;

                return 0;
            }
            else
            {
                buf = sendCache.front();
                ret = Send(buf);

                if (ret == IO_WR_OK)
                {
                    sendCache.pop_front(); 
                    delete buf;
                } 
                else
                {
                    break;
                }
            }
        }

        if (action) action->OnError(this, ret);

        return ret;
    }

    int CConn::Send(Buffer* buf)
    {
        writable = false;

        if (status == WRCLOSED || status >= HANGUP)
        {
            CTM_DEBUG_LOG(log, "Conn can not write status : %d", status);

            return IO_NO_WRITE;
        }

        int retlen = 0;
        time_t now = time(NULL);

        while(1)
        {
            retlen = write(fd, buf->data + buf->offset, buf->len - buf->offset);

            if (retlen < 0)
            {
                error = errno;

                if (EINTR == error) { 
                    continue;
                }
                else if (EAGAIN == error || EWOULDBLOCK == error) {
                    //if (event.monitor) event.monitor->AddEvent(&event, EVENT_WRITE);
                    return IO_WR_AGAIN;
                }
                else {

                    // Close();

                    // if (action) action->OnException(this);

                    CTM_DEBUG_LOG(log, "write failed :%d:%s", error, strerror(error));

                    return IO_EXCEPT;
                }
            }
            else if (retlen == 0)
            {
                // CloseWrite();

                // if (action) action->OnWriteClose(this);

                return IO_WR_CLOSE;
            }
            else
            {
                lastWrite  = now;
                lastActive = now;

                buf->offset += retlen;
                if (buf->offset < buf->len) 
                {
                    continue;
                }
                else 
                {
                    // if (event.monitor) event.monitor->AddEvent(&event, EVENT_WRITE);

                    writable = true;
                    // if (action) action->OnReady(this);
                    
                    return IO_WR_OK;
                }
            }
        }
        
        return 0;
    }

    int CConn::Recv(Buffer* buf)
    {
        readable = false;

        if (status == RDCLOSED || status >= HANGUP)
        {
            CTM_DEBUG_LOG(log, "Conn can not read status : %d", status);

            return IO_NO_READ;
        }

        int retlen = 0;
        time_t now = time(NULL);

        while(1)
        {
            retlen = read(fd, buf->data + buf->offset, buf->len - buf->offset);

            if (retlen < 0)
            {
                error = errno;
                if (EINTR == error) 
                { 
                    continue;
                }
                else if (EAGAIN == error || EWOULDBLOCK == error) 
                {
                    // if (event.monitor) event.monitor->AddEvent(&event, EVENT_READ);
                    return IO_RD_AGAIN;
                }
                else 
                {
                    //Close();

                    //if (action) action->OnException(this);

                    CTM_DEBUG_LOG(log, "Recv failed :%d:%s", error, strerror(error));

                    return IO_EXCEPT;
                }
            }
            else if (retlen == 0)
            {
                //CloseRead();

                //if (action) action->OnReadClose(this);

                return IO_RD_CLOSE;
            }
            else
            {
                lastRead   = now;
                lastActive = now;

                buf->offset += retlen;
                if (buf->offset < buf->len) 
                {
                    continue;
                }
                else 
                {
                    //if (event.monitor) event.monitor->AddEvent(&event, EVENT_READ);

                    readable = true;
                    //if (action) action->OnReady(this);

                    return IO_RD_OK;
                }
            }
        }
        return IO_EXCEPT;
    }

    void CConn::ClearCache()
    {
        list<Buffer*>::iterator it = sendCache.begin();
        for (; it != sendCache.end(); it++)
        {
            delete *it;
        }
        sendCache.clear();
    }

    void CConn::ChangeStatus(int hanpend)
    {
        if (hanpend == RDCLOSED)
        {
            if (status == WRCLOSED)
                status = HANGUP;
            else if (status == ACTIVE)
                status = RDCLOSED;
        }
        else if (hanpend == WRCLOSED)
        {
            if (status == RDCLOSED)
                status = HANGUP;
            else if (status == ACTIVE)
                status = WRCLOSED;
        }
        else
        {
            status = hanpend;
        }
    }

    void CConn::GetLocalAddr()
    {
        SOCKETLEN_T len = sizeof(localAddr);
        getsockname(fd, (struct sockaddr*)&localAddr, &len);
    }

    void CConn::GetPeerAddr()
    {
        SOCKETLEN_T len = sizeof(peerAddr);
        getpeername(fd, (struct sockaddr*)&peerAddr, &len);
    }

    CConn::CConn()
    {
        Reset();
    }

    CConn::~CConn()
    {
        Close();
    }

    void CConn::Reset()
    {
        id = 0;
        fd = 0;
        type = 0;
        family = 0;
        error = 0;
        status = CREATE;
        memset(&localAddr, 0, sizeof(localAddr));
        memset(&peerAddr, 0, sizeof(peerAddr));
        readable = false;
        writable = false;

        head = NULL;
        recvBuff = NULL;
        action = NULL;
        log = NULL;
        data = NULL;

        lastRead = time(NULL);
        lastWrite = lastRead;
        lastActive = lastRead;

        ClearCache();
    }

    void CConn::Close()
    {
        if (status == CLOSED)
        {
            return;
        }

        if (event.active && event.monitor)
        {
            event.monitor->DelConn(this);
        }

        close(fd);
        ClearCache();
        ChangeStatus(CLOSED);
    }

    void CConn::CloseRead()
    {
        readable = false;

        if (event.active && event.monitor)
        {
            event.monitor->DelEvent(&event, EVENT_READ);
        }

        if (type == C_SOCK)
        {
            shutdown(fd, SHUT_RD);
            ChangeStatus(RDCLOSED);
        }
        else
        {
            ChangeStatus(HANGUP);
        }
    }

    void CConn::CloseWrite()
    {
        writable = false;

        if (event.active && event.monitor)
        {
            event.monitor->DelEvent(&event, EVENT_WRITE);
        }

        if (type == C_SOCK)
        {
            shutdown(fd, SHUT_WR);
            ClearCache();
            ChangeStatus(WRCLOSED);
        }
        else
        {
            ChangeStatus(HANGUP);
        }
    }

    CConnPool::CConnPool(unsigned int size) 
    : m_size(size)
    {

    }

    CConnPool::~CConnPool()
    {
        set<CConn*>::iterator it = m_connSet.begin();
        for (; it != m_connSet.end(); it++)
        {
            (*it)->Close();
            delete *it;
        }

        m_connSet.clear();
    }

    CConn* CConnPool::Create(unsigned int type)
    {
        CConn* conn = NULL;
        if (m_connTypeMap[type].size() > 0)
        {
            conn = m_connTypeMap[type].front();
            m_connTypeMap[type].pop_front();

            conn->Reset();

            return conn;
        }

        if (type == C_SMEM)
        {
            conn = new CShardMemConn;
        }
        else
        {
            conn = new CConn;
        }

        if (conn)
        {
            conn->type = type;
            m_connSet.insert(conn);
        }

        return conn;
    }

    void CConnPool::Free(CConn* conn)
    {
        if (m_connSet.find(conn) != m_connSet.end())
        {
            m_connTypeMap[conn->type].push_back(conn);
            conn->Close();
            
            // m_connSet.erase(conn);
            // delete conn;
        }
    }

    int Read(int fd, Buffer* buf, int& errnum)
    {
        int retlen = 0;

        errnum = 0;

        while(1)
        {
            retlen = read(fd, buf->data + buf->offset, buf->len - buf->offset);
            if (retlen < 0)
            {
                errnum = errno;
                if (EINTR == errnum) { 
                    continue;
                }
                else if (EAGAIN == errnum || EWOULDBLOCK == errnum) {
                    return IO_RD_AGAIN;
                }
                else {
                    return IO_EXCEPT;
                }
            }
            else if (retlen == 0)
            {
                return IO_RD_CLOSE;
            }
            else
            {
                buf->offset += retlen;
                if (buf->offset < buf->len) {
                    continue;
                }
                else {
                    return IO_RD_OK;
                }
            }
        }
        
        return 0;
    }

    int Write(int fd, Buffer* buf, int& errnum)
    {
        int retlen = 0;

        errnum = 0;

        while(1)
        {
            retlen = write(fd, buf->data + buf->offset, buf->len - buf->offset);

            if (retlen < 0)
            {
                errnum = errno;

                if (EINTR == errnum) { 
                    continue;
                }
                else if (EAGAIN == errnum || EWOULDBLOCK == errnum) {
                    return IO_WR_AGAIN;
                }
                else {
                    return IO_EXCEPT;
                }
            }
            else if (retlen == 0)
            {
                return IO_WR_CLOSE;
            }
            else
            {
                buf->offset += retlen;
                if (buf->offset < buf->len) {
                    continue;
                }
                else {
                    return IO_WR_OK;
                }
            }
        }
        
        return 0;
    }

    int FileType(int fd)
    {
        struct stat buf;
        if(fstat(fd, &buf) == -1) 
			return -1;

        switch (buf.st_mode & S_IFMT) {
        case S_IFBLK: return C_FBLK ;
        case S_IFCHR: return C_FCHR ;
        case S_IFDIR: return C_FDIR ;
        case S_IFIFO: return C_FIFO ;
        case S_IFSOCK:return C_SOCK ;
        case S_IFLNK: return C_FLNK ;
        case S_IFREG: return C_FREG ;
        default:      return C_OTHER;
        }

        return C_OTHER;
    }
}