#ifndef CTM_COMMON_QUEUE_H__
#define CTM_COMMON_QUEUE_H__

#include <deque>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

namespace ctm
{
    template <typename T >
    class CSafetyQueue
    {
        #define queue_default_size  ((unsigned int)50000)
        #define queue_max_size ((unsigned int)100000)

    public:
        enum ErrCode
        {
            ERR_OK = 0,
            ERR_TIME_OUT = 1,
            ERR_OTHER = -1,
        };

        CSafetyQueue(unsigned int size = queue_default_size)
        {
            m_size = std::min(size, queue_max_size);
            m_mutex = PTHREAD_MUTEX_INITIALIZER;
            m_cond = PTHREAD_COND_INITIALIZER;

            //pthread_mutex_init(&m_mutex, NULL);
            //pthread_cond_init(&m_cond, NULL);
        }

        ~CSafetyQueue()
        {
            //pthread_mutex_destroy(&m_mutex);
            //pthread_cond_destroy(&m_cond);
        }
        
        int PushFront(const T& val,  unsigned int timeout = -1)
        {
            return Push(val, HEAD, timeout);
        }

        int PushBack(const T& val,  unsigned int timeout = -1)
        {
            return Push(val, TAIL, timeout);
        }

        int GetFront(T& out, unsigned int timeout = -1)
        {
            return Get(out, HEAD, timeout);
        }

        int GetBack(T& out, unsigned int timeout = -1)
        {
            return Get(out, TAIL, timeout);
        }

        int GetPopFront(T& out, unsigned int timeout = -1)
        {
            return GetPop(out, HEAD, timeout);
        }

        int GetPopBack(T& out, unsigned int timeout = -1)
        {
            return GetPop(out, TAIL, timeout);
        }

        void PopFront()
        {
            Pop(HEAD);
        }

        void PopBack()
        {
            Pop(TAIL);
        }

        unsigned int Count()
        {
            return m_queue.size(); 
        }

        unsigned int Capacity() 
        {
            return m_size;
        }

        void Clear()
        {
            pthread_mutex_lock(&m_mutex);
            m_queue.clear();
            pthread_mutex_unlock(&m_mutex);
        }

    private:
        enum EPos
        {
            HEAD = 0,
            TAIL = 1,
        };

        int Push(const T& val, int pos, unsigned int timeout) 
        {
            int ret = 0;
            struct timespec abstime = {0};

            pthread_mutex_lock(&m_mutex);
                        
            if (timeout != -1) 
                MakeTimeOut(abstime, timeout);

            while (m_queue.size() >= m_size) 
            {
                if (timeout == -1) {
                    pthread_cond_wait(&m_cond, &m_mutex);
                } 
                else {
                    ret = pthread_cond_timedwait(&m_cond, &m_mutex, &abstime);
                    if (ret == ETIMEDOUT) {
                        pthread_mutex_unlock(&m_mutex);
                        return ERR_TIME_OUT;
                    }
                    else if (ret != 0 ) {
                        pthread_mutex_unlock(&m_mutex);
                        return ERR_OTHER;
                    }
                }
            }

            if (m_queue.size() == 0)
                pthread_cond_broadcast(&m_cond);

            if (pos == HEAD) 
                m_queue.push_front(val);
            else 
                m_queue.push_back(val);

            pthread_mutex_unlock(&m_mutex);
            return ERR_OK;
        }

        int Get(T& out, int pos, unsigned int timeout)
        {
            int ret = 0;
            struct timespec abstime = {0};
            pthread_mutex_lock(&m_mutex);

            if (timeout != -1) 
                MakeTimeOut(abstime, timeout);

            while (m_queue.size() == 0)
            {
                if (timeout == -1) {
                    pthread_cond_wait(&m_cond, &m_mutex);
                } 
                else {
                    ret = pthread_cond_timedwait(&m_cond, &m_mutex, &abstime);
                    if (ret == ETIMEDOUT) {
                        pthread_mutex_unlock(&m_mutex);
                        return ERR_TIME_OUT;
                    }
                    else if (ret != 0 ) {
                        pthread_mutex_unlock(&m_mutex);
                        return ERR_OTHER;
                    }
                }
            }

            if (pos == HEAD) 
                out = m_queue.front();
            else 
                out = m_queue.back();

            pthread_mutex_unlock(&m_mutex);

            return ERR_OK;
        }

        int GetPop(T& out, int pos, unsigned int timeout)
        {
            int ret = 0;
            struct timespec abstime = {0};

            pthread_mutex_lock(&m_mutex);
                           
            if (timeout != -1) 
                MakeTimeOut(abstime, timeout);

            while (m_queue.size() == 0)
            {
                if (timeout == -1) {
                    pthread_cond_wait(&m_cond, &m_mutex);
                } 
                else {
                    ret = pthread_cond_timedwait(&m_cond, &m_mutex, &abstime);
                    if (ret == ETIMEDOUT) {
                        pthread_mutex_unlock(&m_mutex);
                        return ERR_TIME_OUT;
                    }
                    else if (ret != 0) {
                        pthread_mutex_unlock(&m_mutex);
                        return ERR_OTHER;
                    }
                }
            }

            if (m_queue.size() >= m_size)
                pthread_cond_broadcast(&m_cond);

            if (pos == HEAD)
            {
                out = m_queue.front();
                m_queue.pop_front();
            } 
            else
            {
                out = m_queue.back();
                m_queue.pop_back();
            }
            
            pthread_mutex_unlock(&m_mutex);

            return ERR_OK;
        }

        void Pop(int pos)
        {
            pthread_mutex_lock(&m_mutex);
            if (m_queue.size() > 0)
            {
                if (m_queue.size() >= m_size)
                    pthread_cond_broadcast(&m_cond);

                if (pos == HEAD) 
                    m_queue.pop_front();
                else 
                    m_queue.pop_back();
            }
            pthread_mutex_unlock(&m_mutex);
        }

        static void MakeTimeOut(struct timespec& val, unsigned int millsec)
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            val.tv_sec  = now.tv_sec + millsec / 1000;
            val.tv_nsec = now.tv_usec * 1000 + (millsec % 1000) * 1000000;
            val.tv_sec += val.tv_nsec / 1000000000;
            val.tv_nsec = val.tv_nsec % 1000000000;
        }

    private:
        typedef std::deque<T> StdList;
        StdList m_queue;
        unsigned int m_size;
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;
    };
}

#endif