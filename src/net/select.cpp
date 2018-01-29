#include "select.h"

namespace ctm
{
	CSelect::CSelect()
	{
		FD_ZERO(&m_readFds);
		FD_ZERO(&m_writeFds);
		FD_ZERO(&m_exceptFds);

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


	int CSelect::WaitReadFd(struct timeval* timeOut)
	{
		m_readIt = m_setReadFd.begin();
		return WaitFd(m_setReadFd, m_readFds, timeOut, 0);
	}

	int CSelect::WaitWriteFd(struct timeval* timeOut)
	{
		m_writeIt = m_setWriteFd.begin();
		return WaitFd(m_setWriteFd, m_writeFds, timeOut, 1);
	}

	int CSelect::WaitExceptFd(struct timeval* timeOut)
	{
		m_exceptIt = m_setExceptFd.begin();
		return WaitFd(m_setExceptFd, m_exceptFds, timeOut, 2);
	}

	int CSelect::WaitFds(struct timeval* timeOut)
	{
		SOCKET_T maxFd = 0;

		m_readIt   = m_setReadFd.begin();
		m_writeIt  = m_setWriteFd.begin();
		m_exceptIt = m_setExceptFd.begin();
		
		FD_ZERO(&m_readFds);
		std::set<SOCKET_T>::iterator it = m_setReadFd.begin();
		for (; it != m_setReadFd.end(); it++)
		{
			FD_SET(*it, &m_readFds);
			if (maxFd < *it) 
				maxFd = *it;
		}

		FD_ZERO(&m_writeFds);
		it = m_setWriteFd.begin();
		for (; it != m_setWriteFd.end(); it++)
		{
			FD_SET(*it, &m_writeFds);
			if (maxFd < *it) 
				maxFd = *it;
		}

		FD_ZERO(&m_exceptFds);
		it = m_setExceptFd.begin();
		for (; it != m_setExceptFd.end(); it++)
		{
			FD_SET(*it, &m_exceptFds);
			if (maxFd < *it) 
				maxFd = *it;
		}

		return select(maxFd + 1, &m_readFds, &m_writeFds, &m_exceptFds, timeOut);
	}

	int CSelect::WaitFd(const std::set<SOCKET_T>& setFd, fd_set& fdSet, struct timeval* timeOut, int flag)
	{
		if (setFd.size() == 0)
			return -1;
		
		SOCKET_T maxFd = 0;
		FD_ZERO(&fdSet);
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
			return select(maxFd + 1, &fdSet, NULL, NULL, timeOut);
		case 1:
			return select(maxFd + 1, NULL, &fdSet, NULL, timeOut);
		case 2:
			return select(maxFd + 1, NULL, NULL, &fdSet, timeOut);
		case 3:
			return select(maxFd + 1, &fdSet, &fdSet, &fdSet, timeOut);
		}

		return select(maxFd + 1, &fdSet, NULL, NULL, timeOut);
	}

	SOCKET_T CSelect::NextReadFd()
	{
		for (; m_readIt != m_setReadFd.end(); m_readIt++)
		{
			if (FD_ISSET(*m_readIt, &m_readFds))
			{
				return *m_readIt++;
			}
		}
		return SOCKET_INVALID;
	}

	SOCKET_T CSelect::NextWriteFd()
	{
		for (; m_writeIt != m_setWriteFd.end(); m_readIt++)
		{
			if (FD_ISSET(*m_writeIt, &m_writeFds))
			{
				return *m_writeIt++;
			}
		}
		return SOCKET_INVALID;
	}

	SOCKET_T CSelect::NextExceptFd()
	{
		for (; m_exceptIt != m_setExceptFd.end(); m_readIt++)
		{
			if (FD_ISSET(*m_exceptIt, &m_exceptFds))
			{
				return *m_exceptIt++;
			}
		}
		return SOCKET_INVALID;
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

