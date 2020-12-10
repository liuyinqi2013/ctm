#include "timerhandler.h"
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
    DECLARE_MSG(MSG_SYS_TIMER, CTimerMessage);

    CTimerMessage::CTimerMessage() : CMessage(MSG_SYS_TIMER, Timestamp()),
     m_timerId(0),
     m_remindCount(0),
     m_totalCount(0),
     m_milliInterval(0),
     m_beginTime(0),
     m_expried(0),
     m_object(NULL),
     m_cbFunction(NULL),
     m_cbFunctionEx(NULL),
     m_param(NULL),
     m_param1(NULL),
     m_param2(NULL),
     m_post(false),
     m_queue(NULL),
     m_prev(NULL),
     m_next(NULL)
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
        m_prev = NULL;
        m_next = NULL;
    }

    inline void NodeInsertPrev(CTimerMessage* node, CTimerMessage* node1)
    {
        node1->m_prev = node->m_prev;
        node->m_prev->m_next = node1;
        node->m_prev = node1;
        node1->m_next = node;
    }

    inline void NodeInsertNext(CTimerMessage* node, CTimerMessage* node1)
    {
        node1->m_next = node->m_next;
        node->m_next->m_prev = node1;
        node->m_next = node1;
        node1->m_prev = node;
    }

    inline CTimerMessage* NodeRemove(CTimerMessage* node)
    {
        CTimerMessage* tmp = node->m_next;
        node->m_prev->m_next = node->m_next;
        node->m_next->m_prev = node->m_prev;
        return tmp;
    }

    void ShowList(CTimerMessage* head)
    {
        DEBUG("--------- begin --------");
        CTimerMessage* node = head->m_next;
        while(node != head)
        {
            DEBUG("id:%d, remind:%d", node->m_timerId, node->m_remindCount);
            node = node->m_next;
        }
        DEBUG("--------- end --------");
    }

    CTimerHandler::CTimerHandler()
    {
        m_timerMaxCount = default_timer_max_count;
        m_head = new CTimerMessage;
        m_tail = m_head;
        m_head->m_next = m_head;
        m_head->m_prev = m_head;
        m_generateId = 0;
    }

    CTimerHandler::~CTimerHandler()
    {
        Clear();
        delete m_head;
    }

    int CTimerHandler::AddTimer(unsigned long milliSecond, int count, TimerCallBackEx cbFunc,
        TimerCallBack cb, CTimerApi *object, void *param,
        void *param1, void *param2, bool post, SafeyMsgQueue *queue)
    {
        if (m_timerMaxCount < m_timerMap.size())
        {
            ERROR("Timer maximum limit %d.", m_timerMaxCount);
            return -1;
        }

        unsigned int timerId = GenerateId();
        TimerMap::iterator it = m_timerMap.find(timerId);
        if (it != m_timerMap.end())
        {
            ERROR("Timer %u is alreay exist.", timerId);
            return -1;
        }

        CTimerMessage *message = new CTimerMessage;
        message->m_beginTime = MilliTimestamp();
        message->m_milliInterval = milliSecond;
        message->m_expried = message->m_beginTime + milliSecond;
        message->m_totalCount = count;
        message->m_remindCount = 0;
        message->m_cbFunctionEx = cbFunc;
        message->m_cbFunction = cb;
        message->m_object = object;
        message->m_timerId = timerId;
        message->m_param = param;
        message->m_param1 = param1;
        message->m_param2 = param2;
        message->m_post = post;
        message->m_queue = queue;

        m_timerMap[timerId] = message;
        // 根据过期时间排序
        CTimerMessage *node = m_head->m_next;
        while (node != m_tail && node->m_expried <= message->m_expried)
        {
            node = node->m_next;
        }
        NodeInsertPrev(node, message);
        
        return timerId;
    }

    int CTimerHandler::StopTimer(unsigned int timerId)
    {
        TimerMap::iterator it = m_timerMap.find(timerId);
        if (it != m_timerMap.end())
        {
            NodeRemove(it->second);
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
        unsigned long sleepTime = 100000;
        unsigned long currTime = MilliTimestamp();

        CTimerMessage* node = m_head->m_next;
        while (node != m_tail)
        {
            if (node->m_expried > currTime)
            {
                break;
            }
            node->m_remindCount++;
            // 超时通知上层
            if (node->m_post)
            {
                if (node->m_queue)
                {
                    node->m_queue->PushBack(node);
                }
            }
            else
            {
                if (node->m_cbFunctionEx)
                {
                    TimerCallBackEx func = node->m_cbFunctionEx;
                    func(node->m_timerId, node->m_remindCount,
                         node->m_param, node->m_param1, node->m_param2);
                }
                else if (node->m_object && node->m_cbFunction)
                {
                    CTimerApi *object = node->m_object;
                    TimerCallBack func = node->m_cbFunction;
                    (object->*func)(node->m_timerId, node->m_remindCount,
                        node->m_param, node->m_param1, node->m_param2);
                }
                else if (node->m_object && node->m_cbFunction == NULL)
                {
                    CTimerApi *object = node->m_object;
                    object->OnTimer(node->m_timerId, node->m_remindCount, node->m_param);
                }
            }

            CTimerMessage* old = node;
            node = NodeRemove(node);
            if (old->m_totalCount != -1 && old->m_totalCount <= old->m_remindCount)
            {
                m_timerMap.erase(old->m_timerId);
                if (old->m_post == false)
                    delete old;
                else
                    old->m_delete = true;
                continue;
            }
            else
            {
                old->m_beginTime = currTime;
                old->m_expried = currTime + old->m_milliInterval;
                // 根据过期时间排序
                CTimerMessage* tmp = node;
                while (tmp != m_tail && tmp->m_expried <= old->m_expried)
                {
                    tmp = tmp->m_next;
                }
                NodeInsertPrev(tmp, old);
                if (tmp == node) node = old;
            }
        }

        if (m_head->m_next != m_tail)
        {
            sleepTime = m_head->m_next->m_expried - currTime;
        }

        return sleepTime;
    }

    unsigned int CTimerHandler::GenerateId()
    {
        if (++m_generateId == (unsigned long)-1)
            m_generateId = 0;
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
        m_head->m_next = m_head;
        m_head->m_prev = m_head;
    }
}; // namespace ctm
