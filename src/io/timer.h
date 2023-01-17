#ifndef CTM_IO_TIMER_H__
#define CTM_IO_TIMER_H__
#include <unordered_map>
#include "common/heap.h"

namespace ctm
{
    typedef void (*TimerCallBack)(uint64_t timerId, uint32_t remind, void* param);

    class CTimerMgr
    {
        class CTimer : public HeapItem
        {
        public:
            bool Less(const HeapItem* other) 
            {
                const CTimer* o = dynamic_cast<const CTimer*>(other);
                return Expired() < o->Expired();
            }

            uint64_t Expired() const 
            {
                return m_begin + m_interval;
            }

        private:
            CTimer(uint64_t id, uint64_t milliSecond, int count, TimerCallBack cb, void* param);
            void Clean();

            void ResetBegin() 
            {
                m_begin += m_interval;
                Fix();
            }

        private:
            uint64_t m_id;
            uint32_t m_remind;
            uint32_t m_total;
            uint64_t m_interval;
            uint64_t m_begin;
            TimerCallBack m_cb;
            void* m_param;

            friend class CTimerMgr;
        };

    public:
        CTimerMgr();
        virtual ~CTimerMgr() { Clear(); }
        
        virtual uint64_t AddTimer(uint64_t milliSecond, int count, TimerCallBack cb, void* param);
        int StopTimer(uint64_t timerId);

        void Clear();
        uint64_t GetUid();
        uint64_t Execute();

    private:
        uint64_t m_id;
        Heap m_timerHeap;

        std::unordered_map<uint64_t, CTimer*> m_timerMap;
    };

    void TestTimer();
};
#endif