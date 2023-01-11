#ifndef CTM_COMMON_BUFFER_H__
#define CTM_COMMON_BUFFER_H__

#include <string.h>

namespace ctm
{
    class Buffer
    {
    public:

        Buffer(uint32_t size) : r(0), w(0), cap(size), data(new char[size + 1]) 
        {
        }

        ~Buffer() 
        { 
            delete[] data; 
        }

        uint32_t Len() 
        {
            return w - r; 
        }

        uint32_t Cap() 
        { 
            return cap; 
        }

        char* WrBegin() 
        {
            return data + w;
        }

        char* RdBegin() 
        {
            return data + r;
        }

        char* Raw() 
        {
            return data;
        }

        char* Data() {
            data[Len()] = '\0';
            return data;
        }

        bool Use(uint32_t n) 
        {
            if (n > FreeLen()) return false;

            w += n;
            return true;
        }

        bool Free(uint32_t n) 
        {
            if (n > Len()) return false;

            r += n;
            if (r == w) {
                Reset();
            }
            return true;
        }

        bool IsFull() {
            return bool(cap == w);
        }

        bool IsEmpty() {
            return bool(r == w);
        }

        void Reset() 
        { 
            r = 0; 
            w = 0;
        }

        void Clean() {
            Reset();
            memset(data, 0, cap);
        }

        uint32_t FreeLen() 
        { 
           return cap - w; 
        }

    private:
        uint32_t r;
        uint32_t w;
        uint32_t cap;
        char *data;
    };
}

#endif