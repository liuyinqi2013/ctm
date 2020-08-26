#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "pack.h"

namespace ctm
{
    int CFixedPack::Pack(unsigned int protoId, Buffer* src, Buffer* dst)
    {
        if (src == NULL)
        {
            return -1;
        }

        if (dst == NULL)
        {
            unsigned int len =0;
            return Pack(protoId, src->data, src->offset, NULL, len);
        }

        return Pack(protoId, src->data, src->offset, dst->data, dst->len);
    }

    int CFixedPack::Pack(unsigned int protoId, void* src, unsigned int srcLen, void* dst, unsigned int& dstLen)
    {
        if (dst == NULL)
        {
            return srcLen + 4;
        }

        protoId = htonl(protoId);

        memmove(dst, &protoId, 4);
        memmove((char*)dst + 4, src, srcLen);

        dstLen = srcLen + 4;

        return srcLen + 4;
    }

    int CFixedPack::UnPack(Buffer* src, unsigned int& protoId, Buffer* dst)
    {
        if (src == NULL)
        {
            return -1;
        }

        if (dst == NULL)
        {
            unsigned int len =0;
            return UnPack(src->data, src->offset, protoId, NULL, len);
        }

        return UnPack(src->data, src->offset, protoId, dst->data, dst->len);
    }

    int CFixedPack::UnPack(void* src, unsigned int srcLen, unsigned int& protoId, void* dst, unsigned int& dstLen)
    {
        if (srcLen < 4 || src == NULL)
        {
            return -1;
        }

        if (dst == NULL)
        {
            return srcLen - 4;
        }

        memmove(&protoId, src, 4);
        memmove(dst, (char*)src + 4, srcLen - 4);

        protoId = ntohl(protoId);
        dstLen = srcLen - 4;

        return srcLen - 4;
    }

}