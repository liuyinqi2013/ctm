#include "netpacket.h"
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace ctm
{
    RecvBuf *CContextCache::GetRecvBuf(SOCKET_T fd)
    {
        RecvNetDataMap::iterator it = m_dataMap.find(fd);
        if (it != m_dataMap.end())
        {
            return it->second;
        }
        return NULL;
    }

    void CContextCache::PutRecvBuf(SOCKET_T fd, RecvBuf *data)
    {
        DeleteRecvBuf(fd);
        m_dataMap[fd] = data;
    }

    void CContextCache::DeleteRecvBuf(SOCKET_T fd)
    {
        RecvNetDataMap::iterator it = m_dataMap.find(fd);
        if (it != m_dataMap.end())
        {
            DELETE(it->second);
            m_dataMap.erase(it);
        }
    }

    void CContextCache::Clear()
    {
        RecvNetDataMap::iterator it = m_dataMap.begin();
        while (it != m_dataMap.end())
        {
            DELETE(it->second);
        }
        m_dataMap.clear();
    }

    void CContextCache::Remove(SOCKET_T fd)
    {
        RecvNetDataMap::iterator it = m_dataMap.find(fd);
        if (it != m_dataMap.end())
        {
            m_dataMap.erase(it);
        }
    }

    RecvBuf *CreateRecvBuf(unsigned int len)
    {
        return new RecvBuf(len);
    }

    void DestroyRecvBuf(RecvBuf *&buf)
    {
        DELETE(buf);
    }

    int ReadPacketSize(SOCKET_T fd, unsigned long millisec)
    {
        int size = -1;
        int offset = 0;
        int count = 0;

        while (1)
        {
            int len = read(fd, (char*)&size + offset, 4 - offset);
            if (len < 0)
            {
                int errnum = errno;
                if (EINTR == errnum) {
                    continue;
                }
                else if ((EAGAIN == errnum ||  EWOULDBLOCK == errnum) && ++count <= 3) {
                    if (WaitReadable(fd, millisec) < 0) return -1;
                    continue;
                }
                ERROR_LOG("errno : %d, %s", errnum, strerror(errnum));
                return -1;
            }
            else if (len == 0)
            {
                ERROR_LOG("len = 0");
                return -1;
            }

            offset += len;
            if (offset < 4)
            {
                if (++count <= 3) 
                {
                    usleep(1);
                    continue;
                }
                ERROR_LOG("offset = %d", offset);
                return -1;
            }
            size = ntohl(size);
            break;
        }

        return size;
    }

    int ReadPacketData(SOCKET_T fd, RecvBuf &buf)
    {
        while(1)
        {
            int len = read(fd, buf.data + buf.offset, buf.len - buf.offset);
            if (len < 0)
            {
                int errnum = errno;
                if (EINTR == errnum)
                {
                    continue;
                }
                if (NotFatalError(errnum)) break;
                ERROR_LOG("errno : %d, %s", errnum, strerror(errnum));
                return -1;
            }
            else if (len == 0)
            {
                ERROR_LOG("len == 0");
                return -1;
            }

            buf.offset += len;
            break;
        }

        return 0;
    }

    int SendPacketSize(SOCKET_T fd, int size, unsigned long millisec)
    {
        int offset = 0;
        int count = 0;
        size = htonl(size);
        while (1)
        {         
            int len = write(fd, (char*)&size + offset, 4 - offset);
            if (len < 0)
            {
                int errnum = errno;
                if (EINTR == errnum) {
                    continue;
                }
                else if ((EAGAIN == errnum || errnum == EWOULDBLOCK) && ++count <= 4) {
                    if (WaitWritable(fd, millisec) < 0) return -1;
                    continue;
                }
                ERROR_LOG("errno : %d, %s", errnum, strerror(errnum));

                return -1;
            }
            else if (len == 0)
            {
                ERROR_LOG("len : %d", len);
                return -1;
            }

            offset += len;
            if (offset < 4)
            {
                if (++count <= 3) {
                    if (WaitWritable(fd, millisec) < 0) return -1;
                    continue;
                }
                return -1;
            }

            break;
        }

        return 0;
    }

    int SendPacketData(SOCKET_T fd, RecvBuf& buf)
    {
        while(1)
        {
            int len = write(fd, buf.data + buf.offset, buf.len - buf.offset);
            if (len < 0)
            {
                int errnum = errno;
                if (EINTR == errnum) continue;
                if (NotFatalError(errnum)) break;
                ERROR_LOG("errno : %d, %s", errnum, strerror(errnum));
                return -1;
            }
            /*
            else if (len == 0)
            {
                return -1;
            }
            */

            buf.offset += len;
            break;
        }

        return 0;
    }

    int SendPacketData(SOCKET_T fd, const char* data, int len)
    {
        int ret = 0;
        int offset = 0;
        int count = 0;
        
        while(1)
        {
            ret = write(fd, data + offset, len - offset);
            if (ret < 0)
            {
                int errnum = errno;
                if (EINTR == errnum) continue;
                if ((EAGAIN == errnum || errnum == EWOULDBLOCK))
                {
                    if (++count <= 3 && offset < len) 
                    {
                        if (WaitWritable(fd) < 0) return -1;
                        continue;
                    }
                    return offset;
                }
                return -1;
            }

            offset += ret;
            if (offset < len && ++count <= 3)
            {
                if (WaitWritable(fd) < 0)return -1;
                continue;
            }
            break;
        }

        return offset;
    }

    int WaitReadable(SOCKET_T fd, unsigned long millisec)
    {
        struct timeval val = {0};
        struct timeval* p = NULL;
        if ((long int)millisec > 0)
        {
            val.tv_sec  = millisec / 1000;
            val.tv_usec = (millisec % 1000) * 1000;
            p = &val;
        }
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        int ret = 0;
        while(1)
        {
            ret = select(fd + 1, &fdset, NULL, NULL, p);
            if (ret == -1)
            {
                int errnum = errno;
                if (errnum == EINTR) continue;
                ERROR_LOG("errno : %d, %s", errnum, strerror(errnum));
                return -1;
            }
            else if (ret == 0)
            {
                //DEBUG_LOG("WaitReadable Time out millisec = %d", millisec);
            }
            else
            {
                //DEBUG_LOG("WaitReadable ok ret = %d", ret);
            }
            break;
        }
        return ret;
    }

    int WaitWritable(SOCKET_T fd, unsigned long millisec)
    {
        struct timeval val = {0};
        struct timeval* p = NULL;
        if ((long int)millisec > 0)
        {
            val.tv_sec = millisec / 1000;
            val.tv_usec = (millisec % 1000) * 1000;
            p = &val;
        }
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        int ret = 0;
        while(1)
        {
            ret = select(fd + 1, NULL, &fdset, NULL, p);
            if (ret == -1)
            {
                int errnum = errno;
                if (errnum == EINTR) continue;
                ERROR_LOG("errno : %d, %s", errnum, strerror(errnum));
                return -1;
            }
            else if (ret == 0)
            {
                //DEBUG_LOG("WaitWritable Time out millisec = %d", millisec);
            }
            else
            {
                //DEBUG_LOG("WaitWritable ok ret = %d", ret);
            }
            break;
        }
        return ret;
    }

    DECLARE_MSG(MSG_SYS_NET_DATA, CNetDataMessage);
    DECLARE_MSG(MSG_SYS_NET_CONN, CNetConnMessage);
}