#include <pthread.h>
#include "rwlock.h"


namespace ctm
{
    CRWLock::CRWLock()
    {
        m_rwlock = new pthread_rwlock_t;
        pthread_rwlock_init((pthread_rwlock_t*)m_rwlock, NULL);
    }

    CRWLock::~CRWLock()
    {
        pthread_rwlock_destroy((pthread_rwlock_t*)m_rwlock);
        delete (pthread_rwlock_t*)m_rwlock;
        m_rwlock = NULL;
    }

    bool CRWLock::Lock()
    {
        return WrLock();
    }

    bool CRWLock::TryLock()
    {
        return TryWrLock();
    }

    bool CRWLock::UnLock()
    {
        return !pthread_rwlock_unlock((pthread_rwlock_t*)m_rwlock);
    }

    bool CRWLock::RdLock()
    {
        return !pthread_rwlock_rdlock((pthread_rwlock_t*)m_rwlock);
    }

    bool CRWLock::WrLock()
    {
        return !pthread_rwlock_wrlock((pthread_rwlock_t*)m_rwlock);
    }

    bool CRWLock::TryRdLock()
    {
        return !pthread_rwlock_tryrdlock((pthread_rwlock_t*)m_rwlock);
    }

    bool CRWLock::TryWrLock()
    {
        return !pthread_rwlock_trywrlock((pthread_rwlock_t*)m_rwlock);
    }
}