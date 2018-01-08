#include "select.h"

namespace ctm
{
	CSelect::CSelect() :
		m_iMaxReadFd(0),
		m_iMaxWriteFd(0),
		m_iMaxExceptFd(0)
	{
		FD_ZERO(&m_readFdSet);
		FD_ZERO(&m_writeFdSet);
		FD_ZERO(&m_exceptFdSet);
	}

	CSelect::~CSelect()
	{	
	}

	void CSelect::AddReadFd(const SOCKET_T& fd)
	{
		AddFd(m_setReadFd, m_iMaxReadFd, fd);
	}

	void CSelect::AddWriteFd(const SOCKET_T& fd)
	{
		AddFd(m_setWriteFd, m_iMaxExceptFd, fd);
	}

	void CSelect::WaitExceptFd(const SOCKET_T& fd)
	{
		AddFd(m_setExceptFd, m_iMaxExceptFd, fd);
	}

	void CSelect::AddFd(std::set<SOCKET_T>& setFd, SOCKET_T& maxFd, const SOCKET_T& fd)
	{
		if (maxFd < fd) 
		{
			maxFd = fd;
		}
		setFd.insert(fd);	
	}

	void CSelect::DelReadFd(const SOCKET_T& fd)
	{
		DelFd(m_setReadFd, m_iMaxReadFd, fd);
	}
	
	void CSelect::DelWriteFd(const SOCKET_T& fd)
	{
		DelFd(m_setWriteFd, m_iMaxWriteFd, fd);
	}
	
	void CSelect::DelExceptFd(const SOCKET_T& fd)
	{
		DelFd(m_setExceptFd, m_iMaxExceptFd, fd);
	}

	void CSelect::DelFd(std::set<SOCKET_T>& setFd, SOCKET_T& maxFd, const SOCKET_T& fd)
	{
		std::set<SOCKET_T>::iterator it = setFd.find(fd);
		if (it == setFd.end())
			return;
		setFd.erase(it);
		it = setFd.end();
		maxFd = *--it;
	}
		
}

