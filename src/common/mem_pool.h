#ifndef CTM_COMMON_MEM_POOL_H__
#define CTM_COMMON_MEM_POOL_H__

#include <set>
#include <list>
#include <string.h>
#include "thread/mutex.h"

namespace ctm
{
    template <typename T >
	class CSimpleMemPool
    {
    public:
        CSimpleMemPool(size_t size) : m_size(size) { };
        ~CSimpleMemPool() 
        {
            Clear();
        }

        T* Alloc()
        {
            CLockOwner owner(m_mutex);

            T* ptr = NULL;
            if (m_freeSet.size() > 0)
            {
                ptr = *m_freeSet.begin();
                m_freeSet.erase(ptr);
            }
            else
            {
                if (m_memPool.size() >= m_size) {
                    return ptr;
                }

                ptr = new T;
                if (ptr) {
                    m_memPool.insert(ptr);
                } 
            }
            
            return ptr;
        }

        void Free(T* ptr)
        {
            CLockOwner owner(m_mutex);

            if (m_memPool.find(ptr) != m_memPool.end())
            {
                m_freeSet.insert(ptr);
            }
        }

        void Add(T* ptr)
        {
            CLockOwner owner(m_mutex);
            m_freeSet.insert(ptr);
        }

        void GC()
        {
            CLockOwner owner(m_mutex);
            typename std::list<T*>::iterator it = m_freeSet.begin();
            for (; it != m_freeSet.end(); it++)
            {
                delete *it;
                m_memPool.erase(*it);
            }
            m_freeSet.clear();
        }

        void Clear()
        {
            CLockOwner owner(m_mutex);

            m_freeSet.clear();
            typename std::set<T*>::iterator it = m_memPool.begin();
            for (; it != m_memPool.end(); it++)
            {
                delete *it;
            }
            m_memPool.clear();
        }

    private:
        size_t m_size;
        CMutex m_mutex;
        std::set<T*>  m_memPool;
        std::set<T*>  m_freeSet;  
    };
}

#endif