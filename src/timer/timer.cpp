#include "timer.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common/lock.h"
#include "common/time_tools.h"

#define CMD_EXIT "exit"
#define CMD_WAKE_UP "wake"
#define CMD_LEN 4
#define MAX_TIMER_COUNT 100000

#ifndef DELETE
    #define DELETE(ptr) if ((ptr)) { delete (ptr); ptr = NULL; }
#endif

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

    Json::Value  CTimerMessage::ToJsonObject()
    {
        Json::Value root = CMessage::ToJsonObject();
        root["milliInterval"] = m_milliInterval;
		root["remindCount"] = m_remindCount;
        root["totalCount"] = m_totalCount;
        root["object"] = (unsigned long)m_object;
        root["param"] = (unsigned long)m_param;
        root["totalCount"] = m_totalCount;
        root["timerId"] = m_timerId;
		root["beginTime"] = (unsigned long)m_beginTime;

        return root;
    }

    int  CTimerMessage::FormJsonString(const string &jsonString)
    {
        return 0;
    }

    int  CTimerMessage::FromJsonObject(const Json::Value &jsonObject)
    {
        return 0;
    }

    void  CTimerMessage::CopyFrom(const CMessage &other)
    {
        CMessage::CopyFrom(other);
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

    CTimer::CTimer()
    {
        m_timerMaxCount = default_timer_max_count;
        m_generateId = 1;
        assert(pipe(m_pipe) == 0);
    }

    CTimer::~CTimer()
    {
        SendCmd(CMD_EXIT, strlen(CMD_EXIT));
        Clear();
        close(m_pipe[0]);
        close(m_pipe[1]);
    }

    int CTimer::AddTimer(unsigned long milliSecond, int count, TimerCallBack cb, CTimerApi* object,
                void* param, void* param1, void* param2, 
                bool post, SafeyMsgQueue* queue)
    {
        CLockOwner Owner(m_mutex);
        
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

        WakeUp();

        return timerId;
    }

    int CTimer::StopTimer(unsigned int timerId)
    {
        CLockOwner Owner(m_mutex);

        TimerMap::iterator it = m_timerMap.find(timerId);
        if (it != m_timerMap.end())
        {
            delete it->second;
            m_timerMap.erase(it);
            WakeUp();
        }

        return 0;  
    }

    void CTimer::StopAllTimer()
    {
        CLockOwner Owner(m_mutex);
        Clear();
        WakeUp();
    }

    int CTimer::Run()
    {
        int ret = 0;
        int len = 0;
        int error = 0;
        unsigned long sleepMilliSecond = 0;
        char buf[CMD_LEN + 1] = {0};
        struct timeval val = {0};
        fd_set readset;

        while(1)
        {
            sleepMilliSecond = SleepMilliSecond();
            val.tv_sec = sleepMilliSecond / 1000;
            val.tv_usec = (sleepMilliSecond % 1000) * 1000;
            FD_ZERO(&readset);
            FD_SET(m_pipe[0], &readset);
            ret = select(m_pipe[0] + 1, &readset, NULL, NULL, &val);
            if (ret < 0)
            {
                error = errno;
                if (EINTR == error) continue;
                fprintf(stderr, "%s:%d select error %d:%s\n", __FILE__, __LINE__, errno, strerror(errno));
                goto exit;
            }
            else if (ret > 0)
            {
                while(1)
                {
                    len = read(m_pipe[0], buf, CMD_LEN);
                    if (len <= 0)
                    {
                        error = errno;
                        if (EINTR == error) continue;
                        fprintf(stderr, "%s:%d read error %d:%s\n", __FILE__, __LINE__, errno, strerror(errno));
                        goto exit;
                    }

                    buf[len] = '\0';
                    if (strncmp(buf, CMD_EXIT, CMD_LEN) == 0)
                    {
                        goto exit;
                    }

                    break;
                }
            }
        }

    exit:
        return 0;
    }

    unsigned long CTimer::SleepMilliSecond()
    {
        long timeCost = 0;
        long timeLeft = 0;
        unsigned long sleepTime = 1000;
        unsigned long currTime = MilliTimestamp();

        CLockOwner Owner(m_mutex);

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
                    if (it->second->m_object && it->second->m_cbFunction)
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

                if (it->second->m_totalCount <= it->second->m_remindCount)
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

    int CTimer::SendCmd(const char* cmd, int len)
    {
        int ret = 0;
        int offset = 0;
        while(1)
        {
            ret = write(m_pipe[1], cmd + offset, len - offset);
            if (ret <= 0)
            {
                int error = errno;
                if (EINTR == error) continue;
                fprintf(stderr, "%s:%d write error %d:%s\n", __FILE__, __LINE__, errno, strerror(errno));
                return -1;
            }
            offset += ret;
            if (offset == len) break;
        }

        return offset;
    }

    bool CTimer::WakeUp()
    {
        return (SendCmd(CMD_WAKE_UP, CMD_LEN) > 0);
    }

    unsigned int CTimer::GenerateId()
    {
        if (++m_generateId == (unsigned long)-1) m_generateId = 1;
        return m_generateId;
    }

    void CTimer::Clear()
    {
        TimerMap::iterator it = m_timerMap.begin();
        for (; it != m_timerMap.end(); it++)
        {
            delete it->second;
        }
        m_timerMap.clear();
    }
};
