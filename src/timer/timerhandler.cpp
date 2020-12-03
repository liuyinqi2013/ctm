#include "timerhandler.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common/time_tools.h"

namespace ctm
{
    DECLARE_MSG(MSG_SYS_TIMER, CTimerMessage);

    CTimerMessage::CTimerMessage() :
        CMessage(MSG_SYS_TIMER, Timestamp()),
        m_timerId(0),
        m_remindCount(0),
        m_totalCount(0),
        m_milliInterval(0),
        m_beginTime(0),
        m_object(NULL),
        m_cbFunction(NULL),
        m_cbFunctionEx(NULL),
        m_param(NULL),
        m_param1(NULL),
        m_param2(NULL),
        m_post(false),
        m_queue(NULL)
    {
    }

    CTimerMessage::~CTimerMessage()
    {
    }

    void CTimerMessage::Clear()
    {
        CMessage::Clear();
        m_milliInterval = 0;
        m_cbFunction = 0;
        m_object = 0;
        m_remindCount = 0;
        m_totalCount = 0;
        m_beginTime = 0;
        m_timerId = 0;
        m_param = NULL;
        m_param1 = NULL;
        m_param2 = NULL;
        m_post = false;
        m_queue = NULL;
    }

    CTimerHandler::CTimerHandler()
    {
        m_timerMaxCount = default_timer_max_count;
        m_generateId = 1;
    }

    CTimerHandler::~CTimerHandler()
    {
        Clear();
    }

    int CTimerHandler::AddTimer(unsigned long milliSecond, int count, TimerCallBackEx cbFunc, 
        TimerCallBack cb, CTimerApi* object, void* param, 
        void* param1, void* param2, bool post, SafeyMsgQueue* queue)
    {
        if (m_timerMaxCount < m_timerMap.size())
        {
            fprintf(stderr, "%s:%d Timer maximum limit %d\n", __FILE__, __LINE__, m_timerMaxCount);
            return -1;
        }

        unsigned int timerId = GenerateId();
        TimerMap::iterator it = m_timerMap.find(timerId);
        if (it != m_timerMap.end())
        {
            return -1;
        }
        
        CTimerMessage* message = new CTimerMessage;
        message->m_milliInterval = milliSecond;
        message->m_totalCount = count;
        message->m_remindCount = 0;
        message->m_cbFunctionEx = cbFunc;
        message->m_cbFunction = cb;
        message->m_object = object;
        message->m_beginTime = MilliTimestamp();
        message->m_timerId = timerId;
        message->m_param = param;
        message->m_param1 = param1;
        message->m_param2 = param2;
        message->m_post = post;
        message->m_queue = queue;

        m_timerMap[timerId] = message;

        return timerId;
    }

    int CTimerHandler::StopTimer(unsigned int timerId)
    {
        TimerMap::iterator it = m_timerMap.find(timerId);
        if (it != m_timerMap.end())
        {
            delete it->second;
            m_timerMap.erase(it);
        }

        return 0;  
    }

    void CTimerHandler::StopAllTimer()
    {
        Clear();
    }

    unsigned long CTimerHandler::HandleTimeOuts()
    {
        long timeCost = 0;
        long timeLeft = 0;
        unsigned long sleepTime = 100000;
        unsigned long currTime = MilliTimestamp();

        TimerMap::iterator it = m_timerMap.begin();
        while(it != m_timerMap.end())
        {
            timeCost = currTime - it->second->m_beginTime;
            timeLeft = it->second->m_milliInterval - timeCost;
            if (timeLeft <= 0) 
            {
                it->second->m_remindCount++;

                // 超时通知上层
                if (it->second->m_post)
                {
                    if (it->second->m_queue) it->second->m_queue->PushBack(it->second);
                }
                else
                {
                    if (it->second->m_cbFunctionEx)
                    {
                        TimerCallBackEx func = it->second->m_cbFunctionEx;
                        func(it->second->m_timerId, it->second->m_remindCount, 
                            it->second->m_param, it->second->m_param1, it->second->m_param2);
                    }
                    else if (it->second->m_object && it->second->m_cbFunction)
                    {
                        CTimerApi* object = it->second->m_object;
                        TimerCallBack func = it->second->m_cbFunction;
                        (object->*func)(it->second->m_timerId, it->second->m_remindCount, 
                            it->second->m_param, it->second->m_param1, it->second->m_param2);
                    }
                    else if (it->second->m_object && it->second->m_cbFunction == NULL)
                    {
                        CTimerApi* object = it->second->m_object;
                        object->OnTimer(it->second->m_timerId, it->second->m_remindCount, it->second->m_param);
                    }
                }

                if (it->second->m_totalCount != -1 && it->second->m_totalCount <= it->second->m_remindCount)
                {
                    if (it->second->m_post == false)
                        delete it->second;
                    else
                        it->second->m_delete = true;

                    m_timerMap.erase(it++);
                    continue;
                }
                else
                {
                    timeLeft = it->second->m_milliInterval;
                    it->second->m_beginTime = currTime;
                }
            }

            if (timeLeft > 0 && (unsigned long)timeLeft < sleepTime)
            {
                sleepTime = timeLeft;
            }

            it++; 
        }

        return sleepTime;
    }

    unsigned int CTimerHandler::GenerateId()
    {
        if (++m_generateId == (unsigned long)-1) m_generateId = 1;
        return m_generateId;
    }

    void CTimerHandler::Clear()
    {
        TimerMap::iterator it = m_timerMap.begin();
        for (; it != m_timerMap.end(); it++)
        {
            delete it->second;
        }
        m_timerMap.clear();
    }
};
