#ifndef CTM_COMMON_MESSAGE_H__
#define CTM_COMMON_MESSAGE_H__
#include <string>
#include <memory>
#include <queue>
#include <array>
#include <list>
#include <deque>
#include <atomic>
#include <unordered_map>
#include <time.h>
#include "json/json.h"
#include "json/json-forwards.h"
#include "thread/mutex.h"

namespace ctm
{
    using namespace std;
    enum EMessageType
    {
        MSG_SYS_NULL = 0,
        MSG_SYS_COMMON = 1,
        MSG_SYS_TIMER = 2,
        MSG_SYS_NET_DATA = 3,
        MSG_SYS_NET_CONN = 4,
    };

    class CMessage
    {
    public:
        CMessage() 
        : m_id(), m_type(0), m_createTime(time(NULL))
        { }

        CMessage(unsigned int type, unsigned long time = time(NULL)) 
        : m_id(), m_type(type), m_createTime(time)
        { }

        CMessage(const CMessage& other) 
        : m_id(other.m_id), m_type(other.m_type), m_createTime(other.m_createTime)
        { }

        virtual ~CMessage() { }

        CMessage& operator = (const CMessage& other) { CopyFrom(other); return *this; }

        virtual int Serialization(char* buf, int& len);
        virtual int DeSerialization(const char* buf, int len);
        
        virtual string ToString()
        {
            return "";
        }

        virtual string ToJsonString()
        {
            return ToJsonObject().toStyledString();
        }
        
        virtual Json::Value ToJsonObject();
        virtual int FormJsonString(const string& jsonString);
        virtual int FromJsonObject(const Json::Value& jsonObject);
        virtual void CopyFrom(const CMessage& other);
        virtual void Clear();

        unsigned int Id() const 
        {
            return m_id;
        }

        unsigned int Type() const
        {
            return m_type;
        }

        unsigned long CreateTime() const
        {
            return m_createTime;
        }

        void SetId(unsigned int id) { m_id = id;}
        void SetType(unsigned int type) { m_type = type; }
        void SetCreateTime(unsigned long time) { m_createTime = time; }

    protected:
        static unsigned int GenerateId();

    public:
        unsigned int m_id;
        unsigned int m_type;
        unsigned long m_createTime;
    };

    typedef CMessage* (*MessageFunction)();
    typedef unordered_map<unsigned int, MessageFunction> MessageFunctionMap;
    extern void RegisterMessage(unsigned int msgType, MessageFunction func);
    	
    #define DECLARE_MSG(MsgType, ClassName)\
	static inline CMessage* Function##ClassName() { return new ClassName(); }\
	class Register##ClassName\
	{\
	public:\
		Register##ClassName() { RegisterMessage(MsgType, Function##ClassName); }\
	};\
	static Register##ClassName *globalRegister##ClassName = new Register##ClassName()
	
	extern CMessage* CreateMessage(unsigned int msgType);

    class CMessgaeQueue
    {
    public:
        CMessgaeQueue() {}
        virtual ~CMessgaeQueue() {}
        virtual int PushFront(const shared_ptr<CMessage>& message) = 0;
        virtual int PushBack(const shared_ptr<CMessage>& message) = 0;
        virtual int NonBlockPushFront(const shared_ptr<CMessage>& message) = 0;
        virtual int NonBlockPushBack(const shared_ptr<CMessage>& message) = 0;
        virtual shared_ptr<CMessage>& GetFront(unsigned long millisec = -1) = 0;
        virtual shared_ptr<CMessage>& GetBack(unsigned long millisec = -1) = 0;
        virtual shared_ptr<CMessage> GetPopFront(unsigned long millisec = -1) = 0;
        virtual shared_ptr<CMessage> GetPopBack(unsigned long millisec = -1) = 0;
        virtual shared_ptr<CMessage>& NonBlockGetFront() = 0;
        virtual shared_ptr<CMessage>& NonBlockGetBack() = 0;
        virtual shared_ptr<CMessage> NonBlockGetPopFront() = 0;
        virtual shared_ptr<CMessage> NonBlockGetPopBack() = 0;
        virtual void PopFront() = 0;
        virtual void PopBack() = 0;
        virtual unsigned int Count() = 0;
        virtual unsigned int Capacity() = 0;
        virtual void Clear() {}
    protected:
        enum EPos
        {
            HEAD = 0,
            TAIL = 1,
        };
        static const unsigned int default_max_size = 50000;
    };

    /*
     线程安全的通用消息队列、消息队列为空时取消息会阻塞
     直到消息队列不为空
    */
    class CCommonQueue : public CMessgaeQueue
    {
    public:
        CCommonQueue(unsigned int size = default_max_size);
        virtual ~CCommonQueue();

        virtual int PushFront(const shared_ptr<CMessage>& message);
        virtual int PushBack(const shared_ptr<CMessage>& message);
        virtual int NonBlockPushFront(const shared_ptr<CMessage>& message);
        virtual int NonBlockPushBack(const shared_ptr<CMessage>& message);
        virtual shared_ptr<CMessage>& GetFront(unsigned long millisec = -1);
        virtual shared_ptr<CMessage>& GetBack(unsigned long millisec = -1);
        virtual shared_ptr<CMessage> GetPopFront(unsigned long millisec = -1);
        virtual shared_ptr<CMessage> GetPopBack(unsigned long millisec = -1);
        virtual shared_ptr<CMessage>& NonBlockGetFront();
        virtual shared_ptr<CMessage>& NonBlockGetBack();
        virtual shared_ptr<CMessage> NonBlockGetPopFront();
        virtual shared_ptr<CMessage> NonBlockGetPopBack();
        virtual void PopFront();
        virtual void PopBack();
        virtual unsigned int Count();
        virtual unsigned int Capacity() ;
        virtual void Clear();

    private:
        int Push(const shared_ptr<CMessage>& message, int pos);
        int NonBlockPush(const shared_ptr<CMessage>& message, int pos);
        shared_ptr<CMessage>& Get(unsigned long millisec, int pos);
        shared_ptr<CMessage> GetPop(unsigned long millisec, int pos);
        shared_ptr<CMessage>& NonBlockGet(int pos);
        shared_ptr<CMessage> NonBlockGetPop(int pos);
        void Pop(int pos);
    private:
        typedef deque<shared_ptr<CMessage> > StdList;

        StdList m_queue;
        unsigned int m_maxSize;
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;
    };

    class CSingleWriteReadQueue : public CMessgaeQueue
    {
    public:
        CSingleWriteReadQueue(unsigned int size = default_max_size);
        virtual ~CSingleWriteReadQueue();

        virtual int PushFront(const shared_ptr<CMessage>& message);
        virtual int PushBack(const shared_ptr<CMessage>& message);
        virtual int NonBlockPushFront(const shared_ptr<CMessage>& message);
        virtual int NonBlockPushBack(const shared_ptr<CMessage>& message);
        virtual shared_ptr<CMessage>& GetFront(unsigned long millisec = -1);
        virtual shared_ptr<CMessage>& GetBack(unsigned long millisec = -1);
        virtual shared_ptr<CMessage> GetPopFront(unsigned long millisec = -1);
        virtual shared_ptr<CMessage> GetPopBack(unsigned long millisec = -1);
        virtual shared_ptr<CMessage>& NonBlockGetFront();
        virtual shared_ptr<CMessage>& NonBlockGetBack();
        virtual shared_ptr<CMessage> NonBlockGetPopFront();
        virtual shared_ptr<CMessage> NonBlockGetPopBack();
        virtual void PopFront();
        virtual void PopBack();
        virtual unsigned int Count();
        virtual unsigned int Capacity() ;
        virtual void Clear();

        bool Full();
        bool Empty();

    private:
        unsigned int FreeCount();
        int Push(const shared_ptr<CMessage>& message, int pos);
        shared_ptr<CMessage>& Get(unsigned long millisec, int pos);
        shared_ptr<CMessage> GetPop(unsigned long millisec, int pos);
        void Pop(int pos);

    private:
        enum EStatus
        {
            EEMPTY = 0,
            EFULL = 1,
            EOTHER = 2,
        };

        typedef vector<shared_ptr<CMessage> > StdVector;

        StdVector *m_array;
        unsigned int m_maxSize;
        atomic<unsigned int> m_readOffset;
        atomic<unsigned int> m_writeOffset;
        atomic<unsigned short> m_status;
        atomic<unsigned int> m_Count;
    };
};

#endif