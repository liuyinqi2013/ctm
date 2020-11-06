#ifndef CTM_THREAD_RWLOCK_H__
#define CTM_THREAD_RWLOCK_H__

#include "common/lock.h"

namespace ctm
{
    class CRWLock : public CLock
    {
    public:
        CRWLock();
        virtual ~CRWLock();

        bool Lock();
        bool TryLock();
        bool UnLock();

        bool RdLock();
        bool WrLock();
        bool TryRdLock();
        bool TryWrLock();

    private:
        void* m_rwlock;
    };
}

#endif