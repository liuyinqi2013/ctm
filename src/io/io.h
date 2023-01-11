#ifndef CTM_IO_IO_H__
#define CTM_IO_IO_H__

#include <string>

namespace ctm 
{
    #define IO_OK        0
    #define IO_EINTR     (-1)
    #define IO_EOF       (-2)
    #define IO_AGAIN     (-3)
    #define IO_ERROR     (-4)

    class Buffer;

    int SetBlock(int fd);
	int SetNonBlock(int fd);
    int Close(int fd);

    int Read(int fd, void* buf,  uint32_t len);
    int Write(int fd, void* buf, uint32_t len);

    int Read(int fd, Buffer* buf);
    int Write(int fd, Buffer* buf);

    int ReadFull(int fd, Buffer* buf);
    int WriteFull(int fd, Buffer* buf);

    int ReadAll(int fd, std::string & out);
}

#endif