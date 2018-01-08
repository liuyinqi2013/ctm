#ifndef CTM_NET_SELECT_H__
#define CTM_NET_SELECT_H__

#include "socket.h"

#ifdef WIN32

#else
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <set>

namespace ctm
{

	class CSelect
	{
	public:
		CSelect();
		~CSelect();
		
		void AddReadFd(const SOCKET_T& fd);
		void AddWriteFd(const SOCKET_T& fd);
		void AddExceptFd(const SOCKET_T& fd);

		void DelReadFd(const SOCKET_T& fd);
		void DelWriteFd(const SOCKET_T& fd);
		void DelExceptFd(const SOCKET_T& fd);
		
		int WaitReadFd(const struct timeval& timeOut);
		int WaitWriteFd(const struct timeval& timeOut);
		int WaitExceptFd(const struct timeval& timeOut);
		
		SOCKET_T NextReadFd();
		SOCKET_T NextWriteFd();
		SOCKET_T NextExceptFd();

		std::set<SOCKET_T> ReadGoodFds()
		{
			return GoodFds(m_setReadFd, m_readFdSet);
		}

		std::set<SOCKET_T> WriteGoodFds()
		{
			return GoodFds(m_setWriteFd, m_writeFdSet);
		}
		
		std::set<SOCKET_T> ExcpetGoodFds()
		{
			return GoodFds(m_setExceptFd, m_exceptFdSet);
		}

		bool IsReadFd(const SOCKET_T& fd)
		{
			return FD_ISSET(fd, &m_setReadFd);
		}

		bool IsWriteFd(const SOCKET_T& fd)
		{
			return FD_ISSET(fd, &m_setWriteFd);
		}

		bool IsExcpetFd(const SOCKET_T& fd)
		{
			return FD_ISSET(fd, &m_setExceptFd);
		}

		void ClearReadFdSet();
		void ClearWriteFdSet();
		void ClearExceptFdSet();
		void ClearFdSet();
		
	private:
		int WaitFd(const std::set<SOCKET_T>& setFd, fd_set& fdSet, const struct timeval& timeOut, int flag);

		std::set<SOCKET_T> GoodFds(const std::set<SOCKET_T>& setFd, fd_set& fdSet);
		
	private:
		fd_set m_readFdSet;
		fd_set m_writeFdSet;
		fd_set m_exceptFdSet;
		
		std::set<SOCKET_T> m_setReadFd;
		std::set<SOCKET_T> m_setWriteFd;
		std::set<SOCKET_T> m_setExceptFd;

		std::set<SOCKET_T>::iterator m_readIt;
		std::set<SOCKET_T>::iterator m_writeIt;
		std::set<SOCKET_T>::iterator m_exceptIt;
	};

}

#endif

