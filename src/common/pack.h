#ifndef CTM_COMMON_PACK_H__
#define CTM_COMMON_PACK_H__

#include "buffer.h"

namespace ctm
{
    class CPack
    {
    public:
        virtual ~CPack() {}

        virtual int Pack(unsigned int protoId, Buffer* src, Buffer* dst) = 0;
        virtual int Pack(unsigned int protoId, void* src, unsigned int srcLen, void* dst, unsigned int& dstLen) = 0;

        virtual int UnPack(Buffer* src, unsigned int& protoId, Buffer* dst) = 0;
        virtual int UnPack(void* src, unsigned int srcLen, unsigned int& protoId, void* dst, unsigned int& dstLen) = 0;
    };

    class CFixedPack : public CPack
    {
    public:
        virtual ~CFixedPack() {}

        virtual int Pack(unsigned int protoId, Buffer* src, Buffer* dst);
        virtual int Pack(unsigned int protoId, void* src, unsigned int srcLen, void* dst, unsigned int& dstLen);

        virtual int UnPack(Buffer* src, unsigned int& protoId, Buffer* dst);
        virtual int UnPack(void* src, unsigned int srcLen, unsigned int& protoId, void* dst, unsigned int& dstLen);
    };
}

#endif