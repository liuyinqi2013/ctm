#include "sem.h"

namespace ctm
{
	CSem::CSem(int value)
	{
		sem_init(&m_sem, 0, value);
	}
		
	CSem::~CSem()
	{
		sem_destroy(&m_sem);
	}

	int CSem::Post()
	{
		return sem_post(&m_sem);
	}
		
	int CSem::Wait()
	{
		return sem_wait(&m_sem);
	}
}