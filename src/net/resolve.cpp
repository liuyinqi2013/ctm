#include <string.h>
#include <arpa/inet.h>
#include "socket.h"

#include "common/log.h"
#include "common/defer.h"
#include "common/com.h"
#include "resolve.h"


namespace ctm 
{
    CAddr::CAddr(const std::string & ip, uint16_t port)
    {
        Clear();
        if (IsIPv4(ip)) {
            m_addr.ipAddr.sin_family = AF_INET;
            m_addr.ipAddr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &m_addr.ipAddr.sin_addr);
        } else {
            m_addr.ipAddr6.sin6_family = AF_INET6;
            m_addr.ipAddr6.sin6_port = htons(port);
            inet_pton(AF_INET6, ip.c_str(), &m_addr.ipAddr6.sin6_addr);
        }
    }

    CAddr::CAddr(const char* path) 
    {
        Clear();
        m_addr.unAddr.sun_family = AF_UNIX;
        memcpy(m_addr.unAddr.sun_path, path, strlen(path));
    }

    CAddr& CAddr::operator= (const char* path) 
    {
        Clear();
        m_addr.unAddr.sun_family = AF_UNIX;
        memcpy(m_addr.unAddr.sun_path, path, strlen(path));
        return *this;
    }

    std::string CAddr::String() const
    {
        char buf[128] = {0};
        switch (SaFamily())
        {
        case AF_INET:
            inet_ntop(AF_INET, &m_addr.ipAddr.sin_addr, buf, sizeof(buf));
            return string(buf) + ":" + I2S(ntohs(m_addr.ipAddr.sin_port));
        case AF_INET6:
            inet_ntop(AF_INET6, &m_addr.ipAddr6.sin6_addr, buf, sizeof(buf));
            return string(buf) + ":" + I2S(ntohs(m_addr.ipAddr6.sin6_port));
        }
        return string(m_addr.unAddr.sun_path);
    }

    void CAddr::SetPort(uint16_t port)
    {
        switch (SaFamily())
        {
        case AF_INET:
            m_addr.ipAddr.sin_port = htons(port);
            break;
        case AF_INET6:
            m_addr.ipAddr6.sin6_port = htons(port);
            break;
        }
    }

    int CResolve::LookupIPAddr(const char* domain, std::vector<CAddr>& out) 
    {
        if (IsIPv4(domain) || IsIPv6(domain)) {
            out.push_back(CAddr(domain, 0));
            return 0;
        }

        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        struct addrinfo* res, *p;

        if (getaddrinfo(domain, NULL, &hints, &res) < 0) {
            return -1;
        }

        defer([&res](){ freeaddrinfo(res);});

        for (p = res; p; p = p->ai_next){
            if (p->ai_family == AF_INET) {
                auto sa = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
                out.push_back(*sa);
            } else if (p->ai_family == AF_INET6) {
                auto sa = reinterpret_cast<struct sockaddr_in6*>(p->ai_addr);
                out.push_back(*sa);
            }
        }

        return 0;
    }

    std::string CResolve::LookupCNAME(const char* domain)
    {
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_flags = AI_CANONNAME;
        struct addrinfo* res;

        if (getaddrinfo(domain, NULL, &hints, &res) < 0) {
            return string("");
        }
        defer([res](){ freeaddrinfo(res);});

        std::string cname;
        if (res->ai_canonname){
            cname = res->ai_canonname;
        }
        return cname;
    }
}