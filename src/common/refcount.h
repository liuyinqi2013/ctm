#ifndef CTM_COMMON_REFCOUNT_H__
#define CTM_COMMON_REFCOUNT_H__

#include "macro.h"

namespace ctm
{
	class CRefCount
	{
	public:
		CRefCount() : m_pCount(new int(1))
		{
		}

		CRefCount(const CRefCount& other) : m_pCount(other.m_pCount)
		{
			++*m_pCount;
		}

		virtual ~CRefCount()
		{
			Clear();
		}

		CRefCount& operator=(const CRefCount& other);

		bool Only() const
		{
			return (*m_pCount == 1);
		}

		int Count() const
		{
			return *m_pCount;
		}

		void Clear();

	private:
		int* m_pCount;		
	};
}

#endif

