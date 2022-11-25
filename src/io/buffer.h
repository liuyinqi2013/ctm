#ifndef CTM_COMMON_BUFFER_H__
#define CTM_COMMON_BUFFER_H__

#include <string.h>

namespace ctm
{
    class Buffer
    {
    public:

        Buffer(size_t size) : r(0), w(0), cap(size), data(new char[size + 1]) 
        {
        }

        ~Buffer() 
        { 
            delete[] data; 
        }

        size_t Len() 
        {
            return w - r; 
        }

        size_t Cap() 
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

        bool Use(size_t n) 
        {
            if (n > FreeLen()) return false;

            w += n;
            return true;
        }

        bool Free(size_t n) 
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

        size_t FreeLen() 
        { 
           return cap - w; 
        }

    private:
        size_t r;
        size_t w;
        size_t cap;
        char *data;
    };
}

#endif