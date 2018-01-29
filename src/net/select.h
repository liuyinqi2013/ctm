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
		
		int WaitReadFd(struct timeval* timeOut);
		int WaitWriteFd(struct timeval* timeOut);
		int WaitExceptFd(struct timeval* timeOut);

		int WaitFds(struct timeval* timeOut);
		
		SOCKET_T NextReadFd();
		SOCKET_T NextWriteFd();
		SOCKET_T NextExceptFd();

		std::set<SOCKET_T> ReadGoodFds()
		{
			return GoodFds(m_setReadFd, m_readFds);
		}

		std::set<SOCKET_T> WriteGoodFds()
		{
			return GoodFds(m_setWriteFd, m_writeFds);
		}
		
		std::set<SOCKET_T> ExcpetGoodFds()
		{
			return GoodFds(m_setExceptFd, m_exceptFds);
		}

		bool IsReadFd(const SOCKET_T& fd)
		{
			return FD_ISSET(fd, &m_readFds);
		}

		bool IsWriteFd(const SOCKET_T& fd)
		{
			return FD_ISSET(fd, &m_writeFds);
		}

		bool IsExcpetFd(const SOCKET_T& fd)
		{
			return FD_ISSET(fd, &m_exceptFds);
		}

		void ClearReadFdSet()
		{
			FD_ZERO(&m_exceptFds);
			m_setReadFd.clear();
			m_readIt = m_setReadFd.begin();
		}

		void ClearWriteFdSet()
		{
			FD_ZERO(&m_writeFds);
			m_setWriteFd.clear();
			m_writeIt = m_setWriteFd.begin();
		}

		void ClearExceptFdSet()
		{
			FD_ZERO(&m_readFds);
			m_setExceptFd.clear();
			m_exceptIt = m_setExceptFd.begin();
		}

		void ClearFdSet()
		{
			ClearReadFdSet();
			ClearWriteFdSet();
			ClearExceptFdSet();
		}
		
	private:
		int WaitFd(const std::set<SOCKET_T>& setFd, fd_set& fdSet, struct timeval* timeOut, int flag);

		std::set<SOCKET_T> GoodFds(const std::set<SOCKET_T>& setFd, fd_set& fdSet);
		
	private:
		fd_set m_readFds;
		fd_set m_writeFds;
		fd_set m_exceptFds;
		
		std::set<SOCKET_T> m_setReadFd;
		std::set<SOCKET_T> m_setWriteFd;
		std::set<SOCKET_T> m_setExceptFd;

		std::set<SOCKET_T>::iterator m_readIt;
		std::set<SOCKET_T>::iterator m_writeIt;
		std::set<SOCKET_T>::iterator m_exceptIt;
	};

}

#endif

