#ifndef CTM_TIMER_TIMERHANDER_H__
#define CTM_TIMER_TIMERHANDER_H__
#include <unordered_map>
#include "common/message.h"

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

    typedef void (*TimerCallBackEx)(unsigned int timerId, unsigned int remindCount, 
        void* param, void* param1, void* param2);

    class CTimerMessage : public CMessage
    {
    public:
        CTimerMessage();
        ~CTimerMessage();
        
        virtual void Clear();

    public: 
        unsigned int m_timerId;
        int m_remindCount;
        int m_totalCount;
        unsigned long m_milliInterval;
        unsigned long m_beginTime;
        unsigned long m_expried;
        CTimerApi* m_object;
        TimerCallBack m_cbFunction;
        TimerCallBackEx m_cbFunctionEx;
        void* m_param;
        void* m_param1;
        void* m_param2;
        bool  m_post;
        SafeyMsgQueue* m_queue;
        CTimerMessage* m_prev;
        CTimerMessage* m_next;
    };

    class CTimerHandler
    {
        static const unsigned int default_timer_max_count = 100000;
    public:
        CTimerHandler();
        virtual ~CTimerHandler();
        
        int AddTimer(unsigned long milliSecond, int count, TimerCallBack cb, CTimerApi* object,
            void* param = NULL, void* param1 = NULL, void* param2 = NULL, 
            bool post = false, SafeyMsgQueue* queue = NULL)
        {
            return AddTimer(milliSecond, count, NULL, cb, object, param, param1, param2, post, queue);
        }

        int AddTimer(unsigned long milliSecond, int count, TimerCallBackEx cb,
            void* param = NULL, void* param1 = NULL, void* param2 = NULL, 
            bool post = false, SafeyMsgQueue* queue = NULL)
        {
            return AddTimer(milliSecond, count, cb, NULL, NULL, param, param1, param2, post, queue);
        }

        int StopTimer(unsigned int timerId);
        void StopAllTimer();
        void Clear();
        void SetTimerMaxCount(unsigned int maxCount) { m_timerMaxCount = maxCount; }
        unsigned long HandleTimeOuts();
    
    protected:
        int AddTimer(unsigned long milliSecond, int count, TimerCallBackEx cbFunc, TimerCallBack cb, CTimerApi* object,
            void* param, void* param1, void* param2, bool post, SafeyMsgQueue* queue);
        unsigned int GenerateId();

    private:
        typedef std::unordered_map<unsigned int, CTimerMessage*> TimerMap;
        unsigned int m_timerMaxCount;
        TimerMap m_timerMap;
        unsigned int m_generateId;
        CTimerMessage* m_head;
        CTimerMessage* m_tail;
    };
};

#endif