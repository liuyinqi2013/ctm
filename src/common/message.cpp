#include "message.h"
#include "common/lock.h"
#include "common/time_tools.h"
#include "thread/mutex.h"
#include "common/log.h"
#include <unistd.h>

#include <stdio.h>
namespace ctm
{
    static MessageFunctionMap *globalMessageFunctionMap = NULL;

    void RegisterMessage(unsigned int msgType, MessageFunction func)
    {
        if (globalMessageFunctionMap == NULL)
        {
            globalMessageFunctionMap = new MessageFunctionMap;
        }
        (*globalMessageFunctionMap)[msgType] = func;
    }

    CMessage* CreateMessage(unsigned int msgType)
    {
        if (globalMessageFunctionMap->find(msgType) == globalMessageFunctionMap->end())
        {
            return NULL;
        }
        return (*globalMessageFunctionMap)[msgType]();
    }

    DECLARE_MSG(MSG_SYS_COMMON, CMessage);

    unsigned int  CMessage::GenerateId()
    {
        static CMutex mutex;
        static unsigned int index = 0;

        CLockOwner Owner(mutex);
        if (++index == (unsigned int)-1) index = 0;

        return index;
    }

    int CMessage::Serialization(char *buf, int &len)
    {
        return 0;
    }

    int CMessage::DeSerialization(const char *buf, int len)
    {
        return 0;
    }

    Json::Value CMessage::ToJsonObject()
    {
        Json::Value root;
        root["id"] = m_id;
		root["type"] = m_type;
		root["createTime"] = m_createTime;

        return root;
    }

    int CMessage::FormJsonString(const string &jsonString)
    {
        Json::Value root;
		Json::Reader reader;
		if (!reader.parse(jsonString, root))
		{
			return -1;
		}
		
		return FromJsonObject(root);
    }

    int CMessage::FromJsonObject(const Json::Value &jsonObject)
    {
        m_id = jsonObject["id"].asUInt();
        m_type = jsonObject["type"].asUInt();
        m_createTime = jsonObject["createTime"].asUInt64();

        return 0;
    }

    void CMessage::Clear()
    {
        m_id = 0;
        m_type = 0;
        m_createTime = 0;
    }

    void CMessage::CopyFrom(const CMessage& other)
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_type = other.m_type;
            m_createTime = other.m_createTime;
        }
    }

    static shared_ptr<CMessage> NullMessage;

    CCommonQueue::CCommonQueue(unsigned int size)
    {
        m_maxSize = size;
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_cond, NULL);
    }

    CCommonQueue::~CCommonQueue()
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }

    int CCommonQueue::PushFront(const shared_ptr<CMessage>& message)
    {
        return Push(message, HEAD);
    }

    int CCommonQueue::PushBack(const shared_ptr<CMessage>& message)
    {
        return Push(message, TAIL);
    }

    int CCommonQueue::NonBlockPushFront(const shared_ptr<CMessage>& message)
    {
        return NonBlockPush(message, HEAD);
    }

    int CCommonQueue::NonBlockPushBack(const shared_ptr<CMessage>& message)
    {
        return NonBlockPush(message, TAIL);
    }

    shared_ptr<CMessage>& CCommonQueue::GetFront(unsigned long millisec)
    {
        return Get(millisec, HEAD);
    }

    shared_ptr<CMessage>& CCommonQueue::GetBack(unsigned long millisec)
    {
        return Get(millisec, TAIL);
    }

    shared_ptr<CMessage> CCommonQueue::GetPopFront(unsigned long millisec)
    {
        return GetPop(millisec, HEAD);
    }

    shared_ptr<CMessage> CCommonQueue::GetPopBack(unsigned long millisec)
    {
        return GetPop(millisec, TAIL);
    }

    shared_ptr<CMessage>& CCommonQueue::NonBlockGetFront()
    {
        return NonBlockGet(HEAD);
    }

    shared_ptr<CMessage>& CCommonQueue::NonBlockGetBack()
    {
        return NonBlockGet(TAIL);
    }

    shared_ptr<CMessage> CCommonQueue::NonBlockGetPopFront()
    {
        return NonBlockGetPop(HEAD);
    }

    shared_ptr<CMessage> CCommonQueue::NonBlockGetPopBack()
    {
        return NonBlockGetPop(TAIL);
    }

    void CCommonQueue::PopFront()
    {
        Pop(HEAD);
    }

    void CCommonQueue::PopBack()
    {
        Pop(TAIL);
    }

    unsigned int CCommonQueue::Count()
    { 
        return m_queue.size(); 
    }

    unsigned int CCommonQueue::Capacity()
    {
        return m_maxSize;
    }

    void CCommonQueue::Clear() 
    {
        m_queue.clear();
    }

    int CCommonQueue::Push(const shared_ptr<CMessage>& message, int pos)
    {
        pthread_mutex_lock(&m_mutex);
        if (m_queue.size() > m_maxSize)
        {
            do {
                pthread_cond_wait(&m_cond, &m_mutex);
            } while (m_queue.size() > m_maxSize);
        }
        else if (m_queue.size() == 0)
        {
            pthread_cond_signal(&m_cond);
        }

        if (pos == HEAD) 
            m_queue.push_front(message);
        else 
            m_queue.push_back(message);

        pthread_mutex_unlock(&m_mutex);

        return 0;
    }

    int CCommonQueue::NonBlockPush(const shared_ptr<CMessage>& message, int pos)
    {
        pthread_mutex_lock(&m_mutex);
        if (m_queue.size() > m_maxSize)
        {
            pthread_mutex_unlock(&m_mutex);
            return -1;
        }
        else if (m_queue.size() == 0)
        {
            pthread_cond_signal(&m_cond);
        }

        if (pos == HEAD) 
            m_queue.push_front(message);
        else 
            m_queue.push_back(message);
        
        pthread_mutex_unlock(&m_mutex);

        return 0;
    }

    shared_ptr<CMessage>& CCommonQueue::Get(unsigned long millisec, int pos)
    {
        pthread_mutex_lock(&m_mutex);
        struct timespec val = {0};
		clock_gettime(CLOCK_REALTIME, &val);
        val.tv_sec += millisec / 1000;
        val.tv_nsec += (millisec % 1000) * 1000000;

        if (m_queue.size() == 0)
        {
            do{
                pthread_cond_wait(&m_cond, &m_mutex);
                /*
                ret = pthread_cond_timedwait(&m_cond, &m_mutex, &val);
                if (ret == ETIMEDOUT)
                {
                    pthread_mutex_unlock(&m_mutex);
                    return NullMessage;
                }
                */
            } while (m_queue.size() == 0);
        }
        shared_ptr<CMessage>& front = m_queue.front();
        shared_ptr<CMessage>& back = m_queue.back();
        //shared_ptr<CMessage>& back = m_queue.front();
        pthread_mutex_unlock(&m_mutex);

        return (pos == HEAD) ? front : back;
    }

    shared_ptr<CMessage> CCommonQueue::GetPop(unsigned long millisec, int pos)
    {
        shared_ptr<CMessage> message;
        pthread_mutex_lock(&m_mutex);
        if (m_queue.size() == 0)
        {
            do{
                pthread_cond_wait(&m_cond, &m_mutex);
            } while (m_queue.size() == 0);
        }

        if (pos == HEAD)
        {
            message = m_queue.front();
            m_queue.pop_front();
        } 
        else
        {
            message = m_queue.back();
            m_queue.pop_back();
        }
        pthread_mutex_unlock(&m_mutex);

        return message;
    }

    shared_ptr<CMessage>& CCommonQueue::NonBlockGet(int pos)
    {
        pthread_mutex_lock(&m_mutex);
        if (m_queue.size() == 0)
        {
            pthread_mutex_unlock(&m_mutex);
            return NullMessage;
        }
        shared_ptr<CMessage>& front = m_queue.front();
        shared_ptr<CMessage>& back = m_queue.back();
        //shared_ptr<CMessage>& back = m_queue.front();
        pthread_mutex_unlock(&m_mutex);

        return (pos == HEAD) ? front : back;
    }

    shared_ptr<CMessage> CCommonQueue::NonBlockGetPop(int pos)
    {
        shared_ptr<CMessage> message;
        pthread_mutex_lock(&m_mutex);
        if (m_queue.size() == 0)
        {
            pthread_mutex_unlock(&m_mutex);
            return message;
        }
        if (pos == HEAD)
        {
            message = m_queue.front();
            m_queue.pop_front();
        } 
        else
        {
            message = m_queue.back();
            m_queue.pop_back();
        }
        pthread_mutex_unlock(&m_mutex);

        return message;
    }

    void CCommonQueue::Pop(int pos)
    {
        pthread_mutex_lock(&m_mutex);
        if (m_queue.size() > 0)
        {
            if (m_queue.size() > m_maxSize)
            {
                pthread_cond_signal(&m_cond);
            }
            
            if (pos == HEAD) m_queue.pop_front();
            else m_queue.pop_back();

            //m_queue.pop();
        }
        pthread_mutex_unlock(&m_mutex);
    }

    CSingleWriteReadQueue::CSingleWriteReadQueue(unsigned int size) :
        m_array(NULL),
        m_maxSize(size),
        m_readOffset(0),
        m_writeOffset(0),
        m_status(0),
        m_Count(0)
    {
        m_array =  new StdVector(m_maxSize);
    }

    CSingleWriteReadQueue::~CSingleWriteReadQueue()
    {
        delete m_array;
    }

    int CSingleWriteReadQueue::PushFront(const shared_ptr<CMessage>& message)
    {
        return Push(message, HEAD);
    }

    int CSingleWriteReadQueue::PushBack(const shared_ptr<CMessage>& message)
    {
        return Push(message, TAIL);
    }

    int CSingleWriteReadQueue::NonBlockPushFront(const shared_ptr<CMessage>& message)
    {
        return Push(message, HEAD);
    }

    int CSingleWriteReadQueue::NonBlockPushBack(const shared_ptr<CMessage>& message)
    {
        return Push(message, TAIL);
    }

    shared_ptr<CMessage>& CSingleWriteReadQueue::GetFront(unsigned long millisec)
    {
        return Get(millisec, HEAD);
    }

    shared_ptr<CMessage>& CSingleWriteReadQueue::GetBack(unsigned long millisec)
    {
        return Get(millisec, TAIL);
    }

    shared_ptr<CMessage> CSingleWriteReadQueue::GetPopFront(unsigned long millisec)
    {
        return GetPop(millisec, HEAD);
    }

    shared_ptr<CMessage> CSingleWriteReadQueue::GetPopBack(unsigned long millisec)
    {
         return GetPop(millisec, TAIL);
    }

    shared_ptr<CMessage>& CSingleWriteReadQueue::NonBlockGetFront()
    {
        return Get(0, HEAD);
    }

    shared_ptr<CMessage>& CSingleWriteReadQueue::NonBlockGetBack()
    {
        return Get(0, TAIL);
    }

    shared_ptr<CMessage> CSingleWriteReadQueue::NonBlockGetPopFront()
    {
        return GetPop(0, HEAD);
    }

    shared_ptr<CMessage> CSingleWriteReadQueue::NonBlockGetPopBack()
    {
        return GetPop(0, TAIL);
    }

    void CSingleWriteReadQueue::PopFront()
    {
        Pop(HEAD);
    }

    void CSingleWriteReadQueue::PopBack()
    {
        Pop(TAIL);
    }

    int CSingleWriteReadQueue::Push(const shared_ptr<CMessage>& message, int pos)
    {
        if (Full()) 
        {
            ERROR_LOG("QUEUE FULL");
            return -1;
        }

        unsigned int writeInx = m_writeOffset;
        unsigned int readInx = m_readOffset;

        (*m_array)[writeInx] = message;

        writeInx = (writeInx + 1) % m_maxSize;
        /*
        if (writeInx == readInx) m_status = EFULL;
        if (m_status == EEMPTY)  m_status = EOTHER;
        */
        m_writeOffset = writeInx;
        ++m_Count;

        return 0;
    }

    shared_ptr<CMessage>& CSingleWriteReadQueue::Get(unsigned long millisec, int pos)
    {
        if (Empty()) 
            return NullMessage;

        return (*m_array)[m_readOffset];
    }

    shared_ptr<CMessage> CSingleWriteReadQueue::GetPop(unsigned long millisec, int pos)
    {
        shared_ptr<CMessage> message;
        if (Empty()) return message;

        unsigned int writeInx = m_writeOffset;
        unsigned int readInx = m_readOffset;

        message = (*m_array)[readInx];
        readInx = (readInx + 1) % m_maxSize;
        /*
        if (readInx == writeInx) m_status = EEMPTY;
        if (m_status == EFULL)   m_status = EOTHER;
        */

        m_readOffset = readInx;
        --m_Count;
        return message;
    }

    void CSingleWriteReadQueue::Pop(int pos)
    {
        if (Empty()) return;

        unsigned int writeInx = m_writeOffset;
        unsigned int readInx = m_readOffset;

        readInx = (readInx + 1) % m_maxSize;
        /*
        if (readInx == writeInx) m_status = EEMPTY;
        if (m_status == EFULL)   m_status = EOTHER;
        */
        m_readOffset = readInx;
        --m_Count;
    }

    unsigned int CSingleWriteReadQueue::Count()
    {
        // return m_maxSize - FreeCount();
        return m_Count;
    }

    unsigned int CSingleWriteReadQueue::Capacity()
    {
        return m_maxSize;
    }

    void CSingleWriteReadQueue::Clear()
    {
        m_readOffset = 0;
        m_writeOffset = 0;
        m_status = EEMPTY;
        m_Count = 0;
    }

    bool CSingleWriteReadQueue::Full()
    {
        //return (m_status == EFULL);
        return (m_Count >= m_maxSize);
    }

    bool CSingleWriteReadQueue::Empty()
    {
        //return (m_status == EEMPTY);
        return (m_Count == 0);
    }

    unsigned int CSingleWriteReadQueue::FreeCount()
    {
        /*
        unsigned int writeInx = m_writeOffset;
        unsigned int readInx  = m_readOffset;
        if (readInx > writeInx) return readInx - writeInx;
        else if (writeInx > readInx) return m_maxSize - (writeInx - readInx);
        
        if (Full())  return 0;
        if (Empty()) return m_maxSize;

        return 0;
        */
       return m_maxSize - m_Count;
    }
};
