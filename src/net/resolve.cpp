#include <string.h>
#include <arpa/inet.h>
#include "socket.h"

#include "resolve.h"


namespace ctm 
{
    void IP::Copy(const char* ip)
    {
        memset(m_addr, 0, sizeof(m_addr));
        if (ctm::IsIPv4(ip)) {
            m_type = 1;
            inet_pton(AF_INET, ip, &m_addr);
        } else {
            m_type = 2;
            inet_pton(AF_INET6, ip, &m_addr);
        }
    }

    void IP::Copy(struct in_addr& addr)
    {
        memset(m_addr, 0, sizeof(m_addr));
        m_type = 1;
        memcpy(m_addr, &addr.s_addr, sizeof(addr.s_addr));
    }

    void IP::Copy(struct in6_addr& addr)
    {
        memset(m_addr, 0, sizeof(m_addr));
        m_type = 2;
        memcpy(m_addr, &addr.s6_addr, sizeof(addr.s6_addr));
    }

    void IP::Copy(const IP& addr)
    {
        m_type = addr.m_type;
        memcpy(m_addr, &addr.m_addr, sizeof(addr.m_addr));
    }

    std::string IP::String() const
    {
        char buf[64]= {0};
        if (IsIPv4()) {
            inet_ntop(AF_INET, &m_addr, buf, sizeof(buf));
        } else {
            inet_ntop(AF_INET6, &m_addr, buf, sizeof(buf));
        }
        return std::string(buf);
    }

    struct in6_addr IP::IPv6() const
    {
        struct in6_addr addr = {0};
        CopyTo(addr);
        return addr;
    }

    struct in_addr IP::IPv4() const
    {
        struct in_addr addr = {0};
        CopyTo(addr);
        return addr;
    }

    void IP::CopyTo(struct in6_addr& addr) const
    {
        memcpy(&addr.s6_addr, m_addr, sizeof(addr));
    }

    void IP::CopyTo(struct in_addr& addr) const
    {
        memcpy(&addr.s_addr, m_addr, sizeof(addr));
    }

    int Resolve(const char* endpoint, std::vector<IP>& out)
    {
        int ret = 0;
		char buf[2048] = {0};
		struct hostent tmp, *h;
		gethostbyname_r(endpoint, &tmp, buf, sizeof(buf), &h, &ret);
		if (ret != 0) {
			return -1;
		}

		if (h->h_addrtype != AF_INET) {
			return -1;
		}

		char** head = h->h_addr_list;
		for(; *head; head++) {
            out.push_back(IP(*((struct in_addr*)*head)));
		}
        
        return 0;
    }
}