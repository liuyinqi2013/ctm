#ifndef CTM_COMMON_BUFFER_H__
#define CTM_COMMON_BUFFER_H__

#include <stddef.h>

namespace ctm
{
    struct Buffer
    {
        Buffer() : len(0), offset(0), data(NULL) {}
        Buffer(unsigned int size) : len(size), offset(0), data(new char[size + 1]) {}
        ~Buffer() { delete[] data; }

        unsigned int len;
        unsigned int offset;
        char *data;
    };
}

#endif