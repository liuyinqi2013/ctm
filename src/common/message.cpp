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

    /*
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

        unsigned int readInx = m_readOffset;

        message = (*m_array)[readInx];
        readInx = (readInx + 1) % m_maxSize;
  
        m_readOffset = readInx;
        --m_Count;
        return message;
    }

    void CSingleWriteReadQueue::Pop(int pos)
    {
        if (Empty()) return;

        unsigned int readInx = m_readOffset;

        readInx = (readInx + 1) % m_maxSize;

        m_readOffset = readInx;
        --m_Count;
    }

    unsigned int CSingleWriteReadQueue::Count()
    {
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
        return (m_Count >= m_maxSize);
    }

    bool CSingleWriteReadQueue::Empty()
    {
        return (m_Count == 0);
    }

    unsigned int CSingleWriteReadQueue::FreeCount()
    {

       return m_maxSize - m_Count;
    }
    */    
}