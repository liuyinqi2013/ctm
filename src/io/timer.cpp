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
    CTimerMgr::CTimerItem::CTimerItem(uint64_t id, uint64_t milliSecond, int count, TimerCallBack cb, void* param)
    {
        Clean();
        m_id = id;
        m_interval = milliSecond;
        m_total = count;
        m_cb = cb;
        m_param = param;
        m_begin = MilliTimestamp();
    }

    void CTimerMgr::CTimerItem::CTimerItem::Clean()
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
        m_id = 0;
    }

    uint64_t CTimerMgr::AddTimer(uint64_t milliSecond, int count, TimerCallBack cb, void *param)
    {
        uint64_t timerId = GetUid();
        auto it = m_timerMap.find(timerId);
        if (it != m_timerMap.end()) {
            ERROR("timer %u is already exist.", timerId);
            return -1;
        }

        CTimerItem *item = new CTimerItem(timerId, milliSecond, count, cb, param);
        m_timerMap[timerId] = item;
        m_timerHeap.Push(item);
        
        return timerId;
    }

    int CTimerMgr::StopTimer(uint64_t timerId)
    {
        auto it = m_timerMap.find(timerId);
        if (it != m_timerMap.end()){
            delete it->second;
            m_timerMap.erase(it);
            m_timerHeap.Remove(it->second->Index());
        }

        return 0;
    }

    uint64_t CTimerMgr::Execute()
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

    uint64_t CTimerMgr::GetUid()
    {
        return ++m_id;
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
}; 
