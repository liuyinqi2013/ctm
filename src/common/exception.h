#ifndef CTM_COMMON_EXCEPTION_H__
#define CTM_COMMON_EXCEPTION_H__
#include <exception>
#include <string>

namespace ctm
{
	class CException : public std::exception
	{
	public:
		CException() throw();
		CException(int errno, const std::string& errmsg) throw();
		CException(const CException& other) throw();
		CException& operator= (const CException& rhs) throw();
		
		virtual ~CException() throw();
		virtual int errno() throw();
		virtual const char* what() const throw();
	private:
		int m_errno;
		std::string	m_errmsg;
	};
}

#endif

