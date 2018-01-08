#ifndef CTM_NET_SELECT_H__
#define CTM_NET_SELECT_H__

#include "socket.h"

#ifdef WIN32
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#else
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
		
		int WaitReadFd(struct timeval timeOut);
		int WaitWriteFd(struct timeval timeOut);
		int WaitExceptFd(struct timeval timeOut);
		
		SOCKET_T NextReadFd() const;
		SOCKET_T NextWriteFd() const;
		SOCKET_T NextExceptFd() const;

		bool IsReadFd(const SOCKET_T& fd);
		bool IsWriteFd(const SOCKET_T& fd);
		bool IsExcpetFd(const SOCKET_T& fd);

		void ClearReadFdSet();
		void ClearWriteFdSet();
		void ClearExceptFdSet();
		void ClearFdSet();
		
	private:
		void AddFd(std::set<SOCKET_T>& setFd, SOCKET_T& maxFd, const SOCKET_T& fd);

		void DelFd(std::set<SOCKET_T>& setFd, SOCKET_T& maxFd, const SOCKET_T& fd);
		
	private:
		SOCKET_T m_iMaxReadFd;
		SOCKET_T m_iMaxWriteFd;
		SOCKET_T m_iMaxExceptFd;
		
		fd_set m_readFdSet;
		fd_set m_writeFdSet;
		fd_set m_exceptFdSet;
		
		std::set<SOCKET_T> m_setReadFd;
		std::set<SOCKET_T> m_setWriteFd;
		std::set<SOCKET_T> m_setExceptFd;
	};

}

#endif

