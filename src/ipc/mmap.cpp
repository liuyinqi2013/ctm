#include "mmap.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace ctm
{
	CMmap::CMmap(size_t len) :
		m_nLen(len),
		m_strName(""),
		m_ptr(NULL)
	{
	}
	
	CMmap::CMmap(const char* fileName, size_t len) :
		m_nLen(len),
		m_strName(fileName),
		m_ptr(NULL)
	{
	}
	
	CMmap::~CMmap()
	{
	}
		
	void* CMmap::Open()
	{
		if (!m_ptr)
		{
			int fd = -1;
			int flags = MAP_SHARED;
			if (m_strName != "")
			{
				struct stat buf = {0};
				if (stat(m_strName.c_str(), &buf) == -1)
				{
					fd = open(m_strName.c_str(), O_CREAT | O_RDWR, 00777);
					if (fd == -1) {
						fprintf(stderr, "open %s failed", m_strName.c_str());
						return NULL;
					}
					
					if (lseek(fd, m_nLen, SEEK_SET) == -1) {
						fprintf(stderr, "lseek failed");
					}

					if (write(fd, " ", 1) == -1) {
						fprintf(stderr, "write failed");
					}
				}
				else
				{
					m_nLen = buf.st_size;
					fd = open(m_strName.c_str(), O_RDWR, 00777);
					if (fd == -1) {
						fprintf(stderr, "open %s failed", m_strName.c_str());
						return NULL;
					}
				}
			}
			else
			{
				fd = -1;
				flags |= MAP_ANONYMOUS;
			}
			
			m_ptr = mmap(NULL, m_nLen, PROT_READ | PROT_WRITE, flags, fd, 0);
			if (!m_ptr)
			{
				fprintf(stderr, "call mmap failed");
			}
			
			if (fd > 0) close(fd);	
		}

		return m_ptr;
	}
	
	int CMmap::Sync()
	{
		return msync(m_ptr, m_nLen, MS_SYNC);
	}
	
	void CMmap::Close()
	{
		if (m_ptr) {
			munmap(m_ptr,  m_nLen);
			m_ptr = NULL;
		}
	}
}
