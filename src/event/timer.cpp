#include "timer.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <algorithm>
#include <list>

#include "common/log.h"
#include "common/time_tools.h"

namespace ctm
{
    CTimerItem::CTimerItem(uint32_t id, uint64_t milliSecond, int count, TimerCallBack cb, void* param)
    {
        Clean();
        m_id = id;
        m_interval = milliSecond;
        m_total = count;
        m_cb = cb;
        m_param = param;
        m_begin = MilliTimestamp();
    }

    void CTimerItem::Clean()
    {
        m_id = 0;
        m_remind = 0;
        m_total = 0;
        m_interval = 0;
        m_begin = 0;
        m_cb = NULL;
        m_param = NULL;
    }

    CTimerMgr::CTimerMgr()
    {
        m_timerMaxCount = default_timer_max_count;
        m_timerId = 0;
    }

    uint32_t CTimerMgr::Add(uint64_t milliSecond, int count, TimerCallBack cb, void *param)
    {
        if (m_timerMaxCount < m_timerMap.size())
        {
            ERROR("timer maximum limit %d.", m_timerMaxCount);
            return -1;
        }

        uint32_t timerId = GetTimerId();
        auto it = m_timerMap.find(timerId);
        if (it != m_timerMap.end())
        {
            ERROR("timer %u is already exist.", timerId);
            return -1;
        }

        CTimerItem *item = new CTimerItem(timerId, milliSecond, count, cb, param);
        m_timerMap[timerId] = item;
        m_timerHeap.Push(item);
        
        return timerId;
    }

    int CTimerMgr::Stop(uint32_t timerId)
    {
        auto it = m_timerMap.find(timerId);
        if (it != m_timerMap.end())
        {
            delete it->second;

            m_timerMap.erase(it);
            m_timerHeap.Remove(it->second->Index());
        }

        return 0;
    }

    uint64_t CTimerMgr::Run()
    {
        uint64_t sleepTime = 100000;
        uint64_t currTime = MilliTimestamp();

        while(m_timerHeap.Len()) 
        {
            auto item = dynamic_cast<CTimerItem*>(m_timerHeap.Top());
            if (item->Expired() > currTime) {
                sleepTime = item->Expired() - currTime;
                break;
            }

            item->m_remind++;
            if (item->m_cb) {
                item->m_cb(item->m_id, item->m_remind, item->m_param);
            }

            if (item->m_remind >= item->m_total) {
                m_timerHeap.Remove(item->Index());
                m_timerMap.erase(item->m_id);
            } else {
                item->ResetBegin();
            }
        }

        return sleepTime;
    }

    uint32_t CTimerMgr::GetTimerId()
    {
        if (++m_timerId == (uint32_t)-1) m_timerId = 0;
        return m_timerId;
    }

    void CTimerMgr::Clear()
    {
        auto it = m_timerMap.begin();
        for (; it != m_timerMap.end(); it++) {
            delete it->second;
        }

        m_timerHeap.Clear();
        m_timerMap.clear();
    }

    void Milli1(uint32_t timerId, uint32_t remind, void* param)
    {
        DEBUG("Milli 1 id:%d, remind:%d", timerId, remind);
    }

    void Milli10(uint32_t timerId, uint32_t remind, void* param)
    {
        DEBUG("Milli 10 id:%d, remind:%d", timerId, remind);
    }

    void Second(uint32_t timerId, uint32_t remind, void* param)
    {
        DEBUG("Second 1 id:%d, remind:%d", timerId, remind);
    }

    void Second5(uint32_t timerId, uint32_t remind, void* param)
    {
        DEBUG("Second 5 id:%d, remind:%d", timerId, remind);
    }

    void Second10(uint32_t timerId, uint32_t remind, void* param)
    {
        DEBUG("Second 10 id:%d, remind:%d", timerId, remind);
    }

    void TestTimer()
    {
        CTimerMgr mgr;
        mgr.Add(10, 10, Milli10, NULL);
        mgr.Add(1000, 10, Second, NULL);
        mgr.Add(10000, 1, Second10, NULL);
        mgr.Add(1, 10, Milli1, NULL);

        bool b;
        while (1)
        {
            usleep(mgr.Run()*1000);
            if (!b) {
                mgr.Add(5000, 10, Second5, NULL); 
                b = true;
            }
        } 
    }

}; 
