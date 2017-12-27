#ifndef CTM_THREAD_MUTEX_H__
#define CTM_THREAD_MUTEX_H__

#include "common/lock.h"
#ifdef WIN32
#include <Windows.h>
#define mutex_t HANDLE 
inline int mutex_init(mutex_t& mutex) { return ((mutex = CreateMutex(NULL, FALSE, NULL)) ? 0 : -1); }
inline int mutex_destroy(mutex_t& mutex) { return CloseHandle(mutex) ? 0 : -1; }
inline int mutex_lock(mutex_t& mutex) { return WaitForSingleObject(mutex, INFINITE); }
inline int mutex_trylock(mutex_t& mutex) { return 0; }
inline int mutex_unlock(mutex_t& mutex)  { return ReleaseMutex(mutex) ? 0 : -1; }

#else
#include <pthread.h>
#define mutex_t pthread_mutex_t
inline int mutex_init(mutex_t& mutex) { return pthread_mutex_init(&mutex, NULL); }
inline int mutex_destroy(mutex_t& mutex) { return pthread_mutex_init(&mutex, NULL); }
inline int mutex_lock(mutex_t& mutex) { return pthread_mutex_lock(&mutex); }
inline int mutex_trylock(mutex_t& mutex) { return pthread_mutex_trylock(&mutex); }
inline int mutex_unlock(mutex_t& mutex)  { return pthread_mutex_unlock(&mutex); }

//#define mutex_init(mutex)    pthread_mutex_init(&mutex, NULL)
//#define mutex_destroy(mutex) pthread_mutex_destroy(&mutex)
//#define mutex_lock(mutex)    pthread_mutex_lock(&mutex)
//#define mutex_trylock(mutex) pthread_mutex_trylock(&mutex)
//#define mutex_unlock(mutex)  pthread_mutex_unlock(&mutex)
	
#endif

namespace ctm
{
	class Mutex : public BaseLock
	{
	public:
			Mutex()
			{
				mutex_init(m_mutex);
			}
			
			virtual ~Mutex()
			{
				mutex_destroy(m_mutex);
			}

			bool Lock()
			{
				return !mutex_lock(m_mutex);
			}

			bool TryLock()
			{
				return !mutex_trylock(m_mutex);
			}

			bool UnLock()
			{
				return !mutex_unlock(m_mutex);
			}
	private:
		mutex_t m_mutex;
	};
}

#endif

