#ifndef CTM_NET_RESOLVE_H__
#define CTM_NET_RESOLVE_H__

#include <vector>
#include <stdint.h>
#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

namespace ctm 
{
    class CAddr
    {
    public:
        CAddr() { Clear(); }
        CAddr(const sockaddr& addr) { Copy(addr, addr.sa_family); }
        CAddr(const sockaddr_un& addr) { Copy(addr, AF_UNIX); }
        CAddr(const sockaddr_in& addr) { Copy(addr, AF_INET);}
        CAddr(const sockaddr_in6& addr) { Copy(addr, AF_INET6); }

        CAddr(const std::string & ip, uint16_t port);
        CAddr(const char* path);
        CAddr(const CAddr& other) { Copy(other.m_addr, other.SaFamily()); }

        void Clear() { memset(&m_addr, 0, sizeof(addr_t)); }
        uint16_t SaFamily() const { return m_addr.addr.sa_family; }

        CAddr& operator= (const sockaddr& addr) { Copy(addr, addr.sa_family); return *this; }
        CAddr& operator= (const sockaddr_un& addr) { Copy(addr, AF_UNIX); return *this; }
        CAddr& operator= (const sockaddr_in& addr) { Copy(addr, AF_INET); return *this; }
        CAddr& operator= (const sockaddr_in6& addr) { Copy(addr, AF_INET6); return *this; }
        CAddr& operator= (const CAddr& other) { Copy(other.m_addr, other.SaFamily()); return *this; }
        CAddr& operator= (const char* path);

        std::string String() const;
        void SetPort(uint16_t port);

        const sockaddr* GetAddr() const { return &m_addr.addr; }
        sockaddr* GetAddr() { return &m_addr.addr; }
        socklen_t GetLen() const  
        {
            switch (SaFamily())
            {
            case AF_INET:
                return sizeof(sockaddr_in);
            case AF_INET6:
                return sizeof(sockaddr_in6);
            case AF_UNIX:
                return sizeof(sockaddr_un);
            }
            return sizeof(sockaddr);
        }

    protected:

        template<typename T>
        void Copy(const T& addr, uint16_t family = AF_INET)
        {
            Clear();
            memcpy(&m_addr, &addr, sizeof(addr)); 
            m_addr.addr.sa_family = family;
        }

        typedef union
        {
            sockaddr     addr;
            sockaddr_un  unAddr;   
            sockaddr_in  ipAddr;
            sockaddr_in6 ipAddr6;
        } addr_t;

        addr_t m_addr;
    };
    
    class CResolve {
    public:
        static int LookupIPAddr(const char* domain, std::vector<CAddr>& out);
        static std::string LookupCNAME(const char* domain);
    };
}

#endif
