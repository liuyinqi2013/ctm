#ifndef CTM_MODULE_TIMER_H__
#define CTM_MODULE_TIMER_H__
#include <map>
#include "module.h"
#include "common/message.h"
#include "thread/thread.h"

namespace ctm
{
    typedef void (CModule::*TimerCallBack)(unsigned int timerId, unsigned int remindCount, void* param);

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
        CModule* m_object;
        TimerCallBack m_cbFunction;
        void* m_param;
    };

    class CTimer : public CModule, public CThread
    {
        static const unsigned int default_timer_max_count = 100000;
    public:
        CTimer();
        virtual ~CTimer();
        int AddTimer(unsigned long milliSecond, int count, TimerCallBack cb, void* m_param, CModule* object);
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
        typedef std::map<unsigned int, shared_ptr<CTimerMessage> > TimerMap;

        unsigned int m_timerMaxCount;
        int m_pipe[2];
        TimerMap m_timerMap;
        CMutex m_mutex;
        unsigned int m_generateId;
    };
};

#endif