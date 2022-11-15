#ifndef CTM_EVENT_EPOLL_MONITOR_H__
#define CTM_EVENT_EPOLL_MONITOR_H__

#include <unordered_map>
#include "common/heap.h"
#include "event.h"

namespace ctm
{
    class CEpollEventMonitor : public CEventMonitor
    {
    public:
        CEpollEventMonitor();
        virtual ~CEpollEventMonitor();

        virtual int Init();
        virtual int Dispatch();

        virtual Event* AddEvent(int fd, int events, EventCallBack cb, void* param);
        virtual Event* AddTimer(uint64_t milliSecond, int count, EventCallBack cb, void* param);

        virtual int Update(Event* ev, int events);
        virtual int Remove(Event* ev);

    private:
        uint32_t GetUid();
        int SendBreak();
        static void RecvBreak(int fd, int events, void* param, uint32_t count);

        int RemoveEvent(int fd);
        int RemoveTimer(int timerId);

        int ToEpollEvent(int events);
        int ToEvent(int events);

        uint64_t HandlerTimeOutEvent();
    private:
        int m_epollFd;
        int m_pipe[2];

        uint32_t m_uid;

        Heap m_eventHeap;
        std::unordered_map<int, Event*> m_eventFdSet;
        std::unordered_map<uint32_t, Event*> m_timerSet;
    };

    void TestTimerEvent();
}

#endif