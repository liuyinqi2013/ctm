#include "select.h"

namespace ctm
{
	CSelect::CSelect()
	{
		FD_ZERO(&m_readFdSet);
		FD_ZERO(&m_writeFdSet);
		FD_ZERO(&m_exceptFdSet);

		m_readIt   = m_setReadFd.begin();
		m_writeIt  = m_setWriteFd.begin();
		m_exceptIt = m_setExceptFd.begin();
	}

	CSelect::~CSelect()
	{	
	}

	void CSelect::AddReadFd(const SOCKET_T& fd)
	{
		m_setReadFd.insert(fd);
	}

	void CSelect::AddWriteFd(const SOCKET_T& fd)
	{
		m_setWriteFd.insert(fd);
	}

	void CSelect::AddExceptFd(const SOCKET_T& fd)
	{
		m_setWriteFd.insert(fd);
	}

	void CSelect::DelReadFd(const SOCKET_T& fd)
	{
		m_setReadFd.erase(fd);
	}
	
	void CSelect::DelWriteFd(const SOCKET_T& fd)
	{
		m_setWriteFd.erase(fd);
	}
	
	void CSelect::DelExceptFd(const SOCKET_T& fd)
	{
		m_setExceptFd.erase(fd);
	}


	int CSelect::WaitReadFd(const struct timeval& timeOut)
	{
		m_readIt = m_setReadFd.begin();
		return WaitFd(m_setReadFd, m_readFdSet, timeOut, 0);
	}

	int CSelect::WaitWriteFd(const struct timeval& timeOut)
	{
		m_writeIt = m_setWriteFd.begin();
		return WaitFd(m_setWriteFd, m_writeFdSet, timeOut, 1);
	}

	int CSelect::WaitExceptFd(const struct timeval& timeOut)
	{
		m_exceptIt = m_setExceptFd.begin();
		return WaitFd(m_setExceptFd, m_exceptFdSet, timeOut, 2);
	}

	int CSelect::WaitFd(const std::set<SOCKET_T>& setFd, fd_set& fdSet, const struct timeval& timeOut, int flag)
	{
		SOCKET_T maxFd = 0;
		FD_ZERO(&setFd);
		std::set<SOCKET_T>::iterator it = setFd.begin();
		for (; it != setFd.end(); it++)
		{
			FD_SET(*it, &fdSet);
			if (maxFd < *it) 
				maxFd = *it;
		}
		
		switch(flag)
		{
		case 0:
			return select(maxFd + 1, &fdSet, NULL, NULL, &timeOut);
		case 1:
			return select(maxFd + 1, NULL, &fdSet, NULL, &timeOut);
		case 2:
			return select(maxFd + 1, NULL, NULL, &fdSet, &timeOut);
		case 3:
			return select(maxFd + 1, &fdSet, &fdSet, &fdSet, &timeOut);
		}

		return select(maxFd + 1, &fdSet, NULL, NULL, &timeOut);
	}

	SOCKET_T CSelect::NextReadFd()
	{
		for (; m_readIt != m_setReadFd.end(); m_readIt++)
		{
			if (FD_ISSET(*m_readIt, &m_setReadFd))
			{
				return *m_readIt;
			}
		}
		return INVALID_SOCKET;
	}

	SOCKET_T CSelect::NextWriteFd()
	{
		for (; m_writeIt != m_setWriteFd.end(); m_readIt++)
		{
			if (FD_ISSET(*m_writeIt, &m_setWriteFd))
			{
				return *m_writeIt;
			}
		}
		return INVALID_SOCKET;
	}

	SOCKET_T CSelect::NextExceptFd()
	{
		for (; m_exceptIt != m_setExceptFd.end(); m_readIt++)
		{
			if (FD_ISSET(*m_exceptIt, &m_setExceptFd))
			{
				return *m_exceptIt;
			}
		}
		return INVALID_SOCKET;
	}

	std::set<SOCKET_T> CSelect::GoodFds(const std::set<SOCKET_T>& setFd, fd_set& fdSet)
	{
		std::set<SOCKET_T> goodSet;
		std::set<SOCKET_T>::iterator it = setFd.begin();
		for (; it != setFd.end(); it++)
		{
			if (FD_ISSET(*it, &fdSet))
			{
				goodSet.insert(*it);
			}
		}

		return goodSet;
	}
		
}

