#include "refcount.h"

namespace ctm
{
	CRefCount& CRefCount::operator=(const CRefCount& other) 
	{
		printf("CRefCount::operator=(const CRefCount& other) \n");
		if(m_pCount != other.m_pCount)
		{
			if(--(*m_pCount) == 0)
			{
				delete m_pCount;
				m_pCount = 0;
			}
			m_pCount = other.m_pCount;
			++(*m_pCount);
		}
		return *this;
	}

}

