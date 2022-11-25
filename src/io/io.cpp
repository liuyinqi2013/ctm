#include "io.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common/log.h"
#include "buffer.h"

namespace ctm {

	int ToIOErrCode(int n, int err) {
		if (n > 0) {
			return IO_OK;
		} else if (n == 0) {
			return IO_EOF; 
		}

		if (err == EINTR) {
			return IO_EINTR;
		} else if (err == EAGAIN || err == EWOULDBLOCK) {
			return IO_AGAIN;
		} 
		return IO_ERROR;
	}

	int SetBlock(int fd)
	{
		int flags = fcntl(fd, F_GETFL);
	    if (flags < 0) 
			return -1; 

		flags &= ~O_NONBLOCK; 
		return fcntl(fd, F_SETFL, flags);
	}

    int SetNonBlock(int fd)
	{
		int flags = fcntl(fd, F_GETFL);
	    if (flags < 0)          
			return -1; 

		flags |= O_NONBLOCK;
		return fcntl(fd, F_SETFL, flags);
	}

	int Close(int fd)
	{
		return close(fd);
	}

	int Read(int fd, void* buf,  size_t len)
	{
		int n = 0;
		while(1) 
		{
			n = read(fd, buf, len);
			if (n < 0 && errno == EINTR) {
				continue;
			} 
			break;
		}
		return n;
	}

    int Write(int fd, void* buf, size_t len)
	{
		int n = 0;
		while(1) 
		{
			n = write(fd, buf, len);
			if (n < 0 && errno == EINTR) {
				continue;
			}
			break;
		}
		return n;
	}

	int Read(int fd, Buffer* buf)
	{
		int n = 0;
		int err = 0;
		while(1) 
		{
			n = read(fd, buf->WrBegin(), buf->FreeLen());
			err = ToIOErrCode(n, errno);
			if (err == IO_EINTR) {
				continue;
			} else if (err != IO_OK) {
				return err;
			}

			buf->Use(n);
			break;
		}

		return IO_OK;
	}

    int Write(int fd, Buffer* buf)
	{
		int n = 0;
		int err = 0;
		while(1) 
		{
			n = write(fd, buf->RdBegin(), buf->Len());
			err = ToIOErrCode(n, errno);
			if (err == IO_EINTR) {
				continue;
			} else if (err != IO_OK) {
				return err;
			}
			
			buf->Free(n);
			break;
		}

		return IO_OK;

	}

	int ReadFull(int fd, Buffer* buf) 
	{
		int n = 0;
		int err = 0;
		while(buf->IsFull()) 
		{
			n = read(fd, buf->WrBegin(), buf->FreeLen());
			err = ToIOErrCode(n, errno);
			if (err == IO_EINTR) {
				continue;
			} else if (err != IO_OK) {
				return err;
			}
			
			buf->Use(n);
		}

		return IO_OK;
	}

    int WriteFull(int fd, Buffer* buf)
	{
		int n = 0;
		int err = 0;
		while(!buf->IsEmpty()) 
		{
			n = write(fd, buf->RdBegin(), buf->Len());
			err = ToIOErrCode(n, errno);
			if (err == IO_EINTR) {
				continue;
			} else if (err != IO_OK) {
				return err;
			}

			buf->Free(n);
		}

		return IO_OK;
	}

	int ReadAll(int fd, std::string & out) 
	{
		Buffer buf(4096);
		int err = 0;
		while (1)
		{
			err = ReadFull(fd, &buf);
			if (err == IO_OK) {
				out.append(buf.Raw(), buf.Len());
				buf.Reset();
			} else if (err == IO_EOF) {
				out.append(buf.Raw(), buf.Len());
				break;
			} else {
				return err;
			}
		}

		return IO_OK;
	}
}