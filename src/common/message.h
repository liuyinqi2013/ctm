#ifndef CTM_COMMON_MESSAGE_H__
#define CTM_COMMON_MESSAGE_H__
#include <string>
#include <memory>
#include <queue>
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
        CMessage() : m_id(/*GenerateId()*/), m_type(0), m_createTime(time(NULL)){
        }

        CMessage(unsigned int type, unsigned long time = time(NULL)) 
        : m_id(/*GenerateId()*/), m_type(type), m_createTime(time){   
        }

        CMessage(const CMessage& other) 
        : m_id(other.m_id), m_type(other.m_type), m_createTime(other.m_createTime){
        }

        virtual ~CMessage() { 
        }

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
        virtual bool PutMessage(const shared_ptr<CMessage>& message) = 0;
        virtual shared_ptr<CMessage> GetAndPopMessage() = 0;
        virtual unsigned int Count() = 0;
        virtual unsigned int Capacity() = 0;
        virtual void Clear() {}
    protected:
        static const unsigned int default_max_size = 10000;
    };

    /*
     线程安全的通用消息队列、消息队列为空时取消息会阻塞
     直到消息队列不为空
    */
    class CCommonQueue : public CMessgaeQueue
    {
    public:
        CCommonQueue();
        virtual ~CCommonQueue();

        virtual bool PutMessage(const shared_ptr<CMessage>& message);
        virtual shared_ptr<CMessage> GetAndPopMessage();
        shared_ptr<CMessage> NonblockGet();
        virtual unsigned int Count() { 
            return m_queue.size(); 
        }
        virtual unsigned int Capacity(){
            return m_maxSize;
        }
        virtual void Clear() {
        }

    private:
        typedef queue<shared_ptr<CMessage> > StdQueue;

        StdQueue m_queue;
        unsigned int m_maxSize;
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;
    };
};

#endif