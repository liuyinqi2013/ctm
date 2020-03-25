#include "netpacket.h"

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

    int ReadPacketSize(SOCKET_T fd)
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
                else if ((EAGAIN == errnum || EWOULDBLOCK) && ++count <= 3) {
                    usleep(1);
                    continue;
                }
                return -1;
            }
            else if (len == 0)
            {
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
                if (EINTR == errnum) continue;
                if (NotFatalError(errnum)) break;
                return -1;
            }
            else if (len == 0)
            {
                return -1;
            }

            buf.offset += len;
            break;
        }

        return 0;
    }

    int SendPacketSize(SOCKET_T fd, int size)
    {
        int offset = 0;
        int count = 0;
        while (1)
        {      
            size = htonl(size);
            int len = write(fd, (char*)&size + offset, 4 - offset);
            if (len < 0)
            {
                int errnum = errno;
                if (EINTR == errnum) {
                    continue;
                }
                else if ((EAGAIN == errnum || EWOULDBLOCK) && ++count <= 3) {
                    usleep(1);
                    continue;
                }
                return -1;
            }
            else if (len == 0)
            {
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

    DECLARE_MSG(MSG_SYS_NET_DATA, CNetDataMessage);
    DECLARE_MSG(MSG_SYS_NET_CONN, CNetConnMessage);
}