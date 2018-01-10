#ifndef CTM_COMMON_EXCEPTION_H__
#define CTM_COMMON_EXCEPTION_H__
#include <exception>
#include <string>

namespace ctm
{
	class CBaseException : public std::exception
	{
	public:
		CBaseException() throw();
		CBaseException(int errno, const std::string& errmsg) throw();
		CBaseException(const CBaseException& other) throw();
		CBaseException& operator= (const CBaseException& rhs) throw();
		
		virtual ~CBaseException() throw();
		virtual int Errno() throw();
		virtual const char* What() const throw();
	private:
		int m_errno;
		std::string	m_errmsg;
	};
}

#endif

