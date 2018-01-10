#include "exception.h"

namespace ctm
{
	CBaseException::CBaseException() throw() :
		m_errno(0),
		m_errmsg("")	
	{
	}
		
	CBaseException::CBaseException(int errno, const std::string& errmsg) throw() :
		m_errno(errno),
		m_errmsg(errmsg)
	{
	}
		
	CBaseException::CBaseException(const CBaseException& other) throw() :
		m_errno(other.m_errno),
		m_errmsg(other.m_errmsg)		
	{
	}
		
	CBaseException& CBaseException::operator= (const CBaseException& rhs) throw()
	{
		this->m_errno = rhs.m_errno;
		this->m_errmsg = rhs.m_errmsg;

		return *this;	
	}
		
	CBaseException::~CBaseException() throw()
	{
	}
	
	int CBaseException::Errno() throw()
	{
		return m_errno;
	}
	
	const char* CBaseException::What() const throw()
	{
		return m_errmsg.c_str();
	}
}
