#include "message.h"
#include "common/lock.h"
#include "common/time_tools.h"
#include "thread/mutex.h"

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
            return NULL;
        return (*globalMessageFunctionMap)[msgType]();
    }

    unsigned int  CMessage::GenerateId()
    {
        static CMutex mutex;
        static unsigned int index = 0;

        CLockOwner Owner(mutex);
        if (++index == -1) index = 0;

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

    CCommonQueue::CCommonQueue()
    {
        m_maxSize = default_max_size;
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_cond, NULL);
    }

    CCommonQueue::~CCommonQueue()
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }

    bool CCommonQueue::PutMessage(const shared_ptr<CMessage>& message)
    {
        if (m_maxSize < Count())
        {
            return false;
        }

        pthread_mutex_lock(&m_mutex);
        if (m_queue.size() == 0)
        {
            pthread_cond_signal(&m_cond);
        }
        m_queue.push(message);
        pthread_mutex_unlock(&m_mutex);
        
        return true;
    }

    shared_ptr<CMessage> CCommonQueue::GetAndPopMessage()
    {
        pthread_mutex_lock(&m_mutex);
        while (m_queue.size() == 0)
        {
            pthread_cond_wait(&m_cond, &m_mutex);
        }
        shared_ptr<CMessage> message = m_queue.front();
        m_queue.pop();
        pthread_mutex_unlock(&m_mutex);

        return message;
    }
};