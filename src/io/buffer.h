#ifndef CTM_COMMON_BUFFER_H__
#define CTM_COMMON_BUFFER_H__

#include <string.h>

namespace ctm
{
    struct Buffer
    {
        Buffer(size_t size) : len(0), size(size), data(new char[size + 1]) 
        {
            memset(data, 0, size);
        }

        ~Buffer() 
        { 
            delete[] data; 
        }

        size_t Len() 
        {
            return len; 
        }

        size_t Size() 
        { 
            return size; 
        }

        char* Begin() 
        {
            return data + len;
        }

        char* Raw() 
        {
            return data;
        }

        char* Data() {
            return data;
        }

        bool Consume(size_t n) 
        {
            if (n > FreeLen()) return false;

            len += n;
            return true;
        }

        bool IsFull() {
            return size == len;
        }

        void Reset() 
        { 
            len = 0; 
        }

        void Clean() {
            len = 0;
            memset(data, 0, size);
        }

        size_t FreeLen() 
        { 
           return size - len; 
        }

        size_t len;
        size_t size;
        char *data;
    };
}

#endif