#ifndef CTM_COMMON_REFCOUNT_H__
#define CTM_COMMON_REFCOUNT_H__

#include "macro.h"

namespace ctm
{
	class CRefCount
	{
	public:
		CRefCount() : m_count(new int(1)) {}

		CRefCount(const CRefCount& other) : m_count(other.m_count)
		{
			++*m_count;
		}

		virtual ~CRefCount() 
		{ 
			Clear(); 
		}

		CRefCount& operator=(const CRefCount& other) 
		{
			if(m_count != other.m_count) {
				Clear();
				m_count = other.m_count;
				++*m_count;
			}
			return *this;
		}

		void Clear()
		{
			if (--*m_count == 0) {
				delete m_count;
				m_count = NULL;
			}
		}

		bool Only() const 
		{ 
			return *m_count == 1;
		}

		int Count() const
		{
			return *m_count;
		}
		
	private:
		int* m_count;		
	};
}

#endif

