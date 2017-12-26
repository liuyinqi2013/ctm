#ifndef CTM_THREAD_MUTEX_H__
#define CTM_THREAD_MUTEX_H__

#include "common/lock.h"
#ifdef WIN32
#define mutex_t pthread_mutex_t
#define mutex_init(mutex)    pthread_mutex_init(&mutex, NULL)
#define mutex_destroy(mutex) pthread_mutex_destroy(&mutex)
#define mutex_lock(mutex)    pthread_mutex_lock(&mutex)
#define mutex_trylock(mutex) pthread_mutex_trylock(&mutex)
#define mutex_unlock(mutex)  pthread_mutex_unlock(&mutex)
#else
#include <pthread.h>
#define mutex_t pthread_mutex_t
#define mutex_init(mutex)    pthread_mutex_init(&mutex, NULL)
#define mutex_destroy(mutex) pthread_mutex_destroy(&mutex)
#define mutex_lock(mutex)    pthread_mutex_lock(&mutex)
#define mutex_trylock(mutex) pthread_mutex_trylock(&mutex)
#define mutex_unlock(mutex)  pthread_mutex_unlock(&mutex)
#endif

namespace ctm
{
	class Mutex : public BaseLock
	{
	public:

	private:
		mutex_t m_mutex;
	};
}

#endif

