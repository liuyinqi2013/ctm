#include "timer.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common/log.h"
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
    CTimer::CTimer()
    {
        assert(pipe(m_pipe) == 0);
    }

    CTimer::~CTimer()
    {
        SendCmd(CMD_EXIT, strlen(CMD_EXIT));
        close(m_pipe[0]);
        close(m_pipe[1]);
    }

    int CTimer::StopTimer(unsigned int timerId)
    {
        CLockOwner Owner(m_mutex);
        m_timerHandle.StopTimer(timerId);
        WakeUp();
        return 0;  
    }

    void CTimer::StopAllTimer()
    {
        CLockOwner Owner(m_mutex);
        m_timerHandle.StopAllTimer();
        WakeUp();
    }

    int CTimer::AddTimer(unsigned long milliSecond, int count, TimerCallBack cb, CTimerApi* object,
        void* param, void* param1, void* param2, bool post, SafeyMsgQueue* queue)
    {
        CLockOwner Owner(m_mutex);
        int id = m_timerHandle.AddTimer(milliSecond, count, cb, object, param, param1, param2, post, queue);
        WakeUp();
        return id;
    }

    int CTimer::AddTimer(unsigned long milliSecond, int count, TimerCallBackEx cb,
        void* param, void* param1, void* param2, bool post, SafeyMsgQueue* queue)
    {
        CLockOwner Owner(m_mutex);
        int id = m_timerHandle.AddTimer(milliSecond, count, cb,  param, param1, param2, post, queue);
        WakeUp();
        return id;
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
            m_mutex.Lock();
            sleepMilliSecond = m_timerHandle.HandleTimeOuts();
            m_mutex.UnLock();
            val.tv_sec = sleepMilliSecond / 1000;
            val.tv_usec = (sleepMilliSecond % 1000) * 1000;
            FD_ZERO(&readset);
            FD_SET(m_pipe[0], &readset);
            ret = select(m_pipe[0] + 1, &readset, NULL, NULL, &val);
            if (ret < 0)
            {
                error = errno;
                if (EINTR == error) continue;
                ERROR("select error %d:%s\n", errno, strerror(errno));
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
                        ERROR("read error %d:%s\n", errno, strerror(errno));
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
                ERROR("write error %d:%s\n", errno, strerror(errno));
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
};
