#ifndef CTM_SERVER_UNITS_H__
#define CTM_SERVER_UNITS_H__

#include <string>
#include <stdint.h>

namespace ctm
{
    #pragma pack(1)

    struct CMsgHeader
    {
        unsigned int dstId;
        unsigned int srcId;
        unsigned int msgId;
        char expand[16];
        unsigned int dataLen;
        int Encode(char* buf, unsigned int& len);
        int Decode(char* buf, unsigned int len);
        std::string ToString() const;
    };

    #pragma pack()

    #define MAX_PACK_LEN (64 * 1024)
    #define MSG_HEAD_LEN (sizeof(CMsgHeader))

    #define HIGH_16(id) ((id) & 0xFFFF0000)
    #define LOW_16(id) ((id) & 0xFFFF)
    #define LOW_2_HIGH(id) (LOW_16(id) << 16)
    #define HIGH_2_LOW(id) (HIGH_16(id) >> 16)
    #define UUID(id, type) ((type << 24) | (id & 0x00FFFFFF))
    #define GET_ID(uuid) ((uuid) & 0x00FFFFFF)
    #define GET_TYPE(uuid) (((uuid) & 0xFF000000) >> 24)
}

#endif