#include "units.h"
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>

namespace ctm
{
    int CMsgHeader::Encode(char* buf, unsigned int& len)
    {
        if (!buf || len < sizeof(CMsgHeader))
        {
            return -1;
        }

        memcpy(buf, this, sizeof(CMsgHeader));
        CMsgHeader* head = (CMsgHeader*)buf;
        head->dstId = htonl(head->dstId);
        head->srcId = htonl(head->srcId);
        head->uid = htonl(head->uid);
        head->msgId = htonl(head->msgId);
        head->dataLen = htonl(head->dataLen);
        
        len = sizeof(CMsgHeader);

        return 0;
    }

    int CMsgHeader::Decode(char* buf, unsigned int len)
    {
        if (!buf || len < sizeof(CMsgHeader))
        {
            return -1;
        }

        CMsgHeader* head = (CMsgHeader*)buf;
        this->dstId = ntohl(head->dstId);
        this->srcId = ntohl(head->srcId);
        this->uid = ntohl(head->uid);
        this->msgId = ntohl(head->msgId);
        this->dataLen = ntohl(head->dataLen);
        memcpy(this->expand, head->expand, sizeof(this->expand));

        return 0;
    }

    std::string CMsgHeader::ToString() const
    {
        char buf[128] = {0};
        snprintf(buf, sizeof(buf) - 1, "dstId:%u, srcId:%u, uid:%u, msgId:%d, dataLen:%u", 
            dstId, srcId, uid, msgId, dataLen);
        return std::string(buf);
    }
}