#include "exception.h"

namespace ctm
{
	CException::CException() throw() :
		m_errno(0),
		m_errmsg("")	
	{
	}
		
	CException::CException(int errno, const std::string& errmsg) throw() :
		m_errno(errno),
		m_errmsg(errmsg)
	{
	}
		
	CException::CException(const CException& other) throw() :
		m_errno(other.m_errno),
		m_errmsg(other.m_errmsg)		
	{
	}
		
	CException& CException::operator= (const CException& rhs) throw()
	{
		this->m_errno = rhs.m_errno;
		this->m_errmsg = rhs.m_errmsg;

		return *this;	
	}
		
	CException::~CException() throw()
	{
	}
	
	int CException::Errno() throw()
	{
		return m_errno;
	}
	
	const char* CException::what() const throw()
	{
		return m_errmsg.c_str();
	}
}
