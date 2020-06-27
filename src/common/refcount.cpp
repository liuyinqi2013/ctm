#include "refcount.h"

namespace ctm
{
	CRefCount& CRefCount::operator=(const CRefCount& other) 
	{
		if(m_pCount != other.m_pCount)
		{
			Clear();
			m_pCount = other.m_pCount;
			++*m_pCount;
		}
		return *this;
	}

	void CRefCount::Clear()
	{
		if (--*m_pCount == 0) 
		{
			delete m_pCount;
			m_pCount = NULL;
		}
	}
}

