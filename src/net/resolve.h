#ifndef CTM_NET_RESOLVE_H__
#define CTM_NET_RESOLVE_H__

#include <vector>
#include <stdint.h>
#include <string>

#include <netinet/in.h>

namespace ctm 
{
    class IP
    {
    public:
        IP(const char* ip)
        {
            Copy(ip);
        }

        IP(struct in_addr& addr)
        {
            Copy(addr);
        }

        IP(struct in6_addr& addr)
        {
            Copy(addr);
        }

        IP(const IP& addr)
        {
            Copy(addr);
        }

        std::string String() const;

        IP& operator=(const char* ip)
        {
            Copy(ip);
            return *this;
        }

        IP& operator=(struct in_addr& other)
        {
            Copy(other);
            return *this;
        }

        IP& operator=(struct in6_addr& other)
        {
            Copy(other);
            return *this;
        }

        IP& operator=(const IP& other)
        {
            Copy(other);
            return *this;
        }

        bool IsIPv4() const
        {
            return m_type == 1;
        }

        bool IsIPv6() const
        {
            return m_type == 2;
        }

        struct in6_addr IPv6() const;
        struct in_addr IPv4() const;
    
    public:
        void Copy(const char* ip);
        void Copy(struct in_addr& other);
        void Copy(struct in6_addr& other);
        void Copy(const IP& other);

        void CopyTo(struct in6_addr& other) const;
        void CopyTo(struct in_addr& other) const;

    private:
        int m_type;
        uint32_t m_addr[4];
    };

    int Resolve(const char* endpoint, std::vector<IP>& out);
}

#endif
