#ifndef CTM_THREAD_SEM_H__
#define CTM_THREAD_SEM_H__

#include </usr/include/semaphore.h>

namespace ctm
{
	class CSem
	{
	public:
		CSem(int value);
		~CSem();

		int Post();
		int Wait();
		
	private:
		sem_t m_sem;
	};
}

#endif
