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
        head->protoId = htonl(head->protoId);
        head->duid = htonl(head->duid);
        head->suid = htonl(head->suid);
        head->msgid = htonl(head->msgid);
        head->sendTime = htonl(head->sendTime);
        head->recvTime = htonl(head->recvTime);
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
        this->protoId = ntohl(head->protoId);
        this->duid = ntohl(head->duid);
        this->suid = ntohl(head->suid);
        this->msgid = ntohl(head->msgid);
        this->sendTime = ntohl(head->sendTime);
        this->recvTime = ntohl(head->recvTime);
        this->dataLen = ntohl(head->dataLen);
        memcpy(this->expand, head->expand, sizeof(this->expand));

        return 0;
    }

    std::string CMsgHeader::ToString() const
    {
        char buf[128] = {0};
        snprintf(buf, sizeof(buf) - 1, "duid:%u, suid:%u, protoId:%d, msgid:%u, sendTime:%u, recvTime:%u dataLen:%u", 
            duid, suid, protoId, msgid, sendTime, recvTime, dataLen);
        return std::string(buf);
    }
}