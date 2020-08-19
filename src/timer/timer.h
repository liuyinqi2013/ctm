#ifndef CTM_TIMER_TIMER_H__
#define CTM_TIMER_TIMER_H__
#include <map>
#include "common/message.h"
#include "thread/thread.h"

namespace ctm
{
    class CTimerApi
    {
    public:
        virtual ~CTimerApi() {};
        virtual void OnTimer(unsigned int timerId, unsigned int remindCount, void* param) {};
    };
    
    typedef void (CTimerApi::*TimerCallBack)(unsigned int timerId, unsigned int remindCount, 
                                             void* param, void* param1, void* param2);

    class CTimerMessage : public CMessage
    {
    public:
        CTimerMessage();
        ~CTimerMessage();
        
        virtual Json::Value ToJsonObject();
        virtual int FormJsonString(const string& jsonString);
        virtual int FromJsonObject(const Json::Value& jsonObject);
        virtual void CopyFrom(const CMessage& other);
        virtual void Clear();

    public: 
        unsigned int m_timerId;
        int m_remindCount;
        int m_totalCount;
        unsigned long m_milliInterval;
        unsigned long m_beginTime;
        CTimerApi* m_object;
        TimerCallBack m_cbFunction;
        void* m_param;
        void* m_param1;
        void* m_param2;
        bool  m_post;
        SafeyMsgQueue *m_queue;
    };

    class CTimer : public CThread
    {
        static const unsigned int default_timer_max_count = 100000;
    public:
        CTimer();
        virtual ~CTimer();
        int AddTimer(unsigned long milliSecond, int count, TimerCallBack cb, CTimerApi* object,
                    void* param = NULL, void* param1 = NULL, void* param2 = NULL, 
                    bool post = false, SafeyMsgQueue* queue = NULL);
        int StopTimer(unsigned int timerId);
        void StopAllTimer();
        void Clear();
        void SetTimerMaxCount(unsigned int maxCount) { m_timerMaxCount = maxCount; }

    protected:
        virtual int Run();
        unsigned long SleepMilliSecond();
        int SendCmd(const char* cmd, int len);
        bool WakeUp();
        unsigned int GenerateId();

    private:
        typedef std::map<unsigned int, CTimerMessage*> TimerMap;

        unsigned int m_timerMaxCount;
        int m_pipe[2];
        TimerMap m_timerMap;
        CMutex m_mutex;
        unsigned int m_generateId;
    };
};

#endif