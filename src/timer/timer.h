#ifndef CTM_TIMER_TIMER_H__
#define CTM_TIMER_TIMER_H__

#include "timerhandler.h"
#include "common/message.h"
#include "thread/thread.h"

namespace ctm
{
    class CTimer : public CThread
    {
    public:
        CTimer();
        virtual ~CTimer();

        int AddTimer(unsigned long milliSecond, int count, TimerCallBack cb, CTimerApi* object,
            void* param = NULL, void* param1 = NULL, void* param2 = NULL, 
            bool post = false, SafeyMsgQueue* queue = NULL);

        int AddTimer(unsigned long milliSecond, int count, TimerCallBackEx cb,
            void* param = NULL, void* param1 = NULL, void* param2 = NULL, 
            bool post = false, SafeyMsgQueue* queue = NULL);

        int StopTimer(unsigned int timerId);
        void StopAllTimer();
        
    protected:
        virtual int Run();
        int SendCmd(const char* cmd, int len);
        bool WakeUp();

    private:
        int m_pipe[2];
        CMutex m_mutex;
        CTimerHandler m_timerHandle;
    };
};

#endif