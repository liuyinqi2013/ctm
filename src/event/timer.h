#ifndef CTM_TIMER_TIMERHANDER_H__
#define CTM_TIMER_TIMERHANDER_H__
#include <unordered_map>
#include "common/heap.h"

namespace ctm
{
    typedef void (*TimerCallBack)(uint32_t timerId, uint32_t remind, void* param);

    class CTimerItem : public HeapItem
    {
    public:
        bool Less(const HeapItem* other) 
        {
            const CTimerItem* o = dynamic_cast<const CTimerItem*>(other);
            return Expired() < o->Expired();
        }

        uint64_t Expired() const
        {
            return m_begin + m_interval;
        }

    private:
        CTimerItem(uint32_t id, uint64_t milliSecond, int count, TimerCallBack cb, void* param);
        
        void Clean();
        void ResetBegin() 
        {
            m_begin += m_interval;
            Fix();
        }
    private:
        uint32_t m_id;
        uint32_t m_remind;
        uint32_t m_total;
        uint64_t m_interval;
        uint64_t m_begin;
        TimerCallBack m_cb;
        void* m_param;

        friend class CTimerMgr;
    };

    class CTimerMgr
    {
        static const uint32_t default_timer_max_count = 100000;
    public:
        CTimerMgr();
        ~CTimerMgr() { Clear(); }
        
        uint32_t Add(uint64_t milliSecond, int count, TimerCallBack cb, void* param);
        int Stop(uint32_t timerId);

        void Clear();
        void SetMaxTimer(uint32_t max) { m_timerMaxCount = max; }
        uint64_t Run();
    private:
        uint32_t GetTimerId();

    private:
        uint32_t m_timerMaxCount;
        uint32_t m_timerId;

        Heap m_timerHeap;
        std::unordered_map<uint32_t, CTimerItem*> m_timerMap;
    };

    void TestTimer();
};
#endif