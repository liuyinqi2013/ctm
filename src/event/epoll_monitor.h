#ifndef CTM_EVENT_EPOLL_MONITOR_H__
#define CTM_EVENT_EPOLL_MONITOR_H__

#include <atomic> 
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

        virtual Event* AddIOEvent(int fd, int events, EventCallBack cb, void* data);
        virtual Event* AddTimerEvent(uint64_t milliSecond, int count, EventCallBack cb, void* data);

        virtual int UpdateIOEvent(Event* ev, int events);
        virtual int RemoveEvent(Event* ev); 

    private:
        uint64_t GetUid() { return ++m_uid; }

        int SendNotify();
        static void RecvNotify(Event* ev, int events, void* data);

        int RemoveIOEvent(int fd);
        int RemoveTimerEvent(uint64_t uid);

        int ToEpollEvent(int events);
        int ToEvent(int events);

        uint64_t ProcessTimerEvent();
    private:
        int m_epollFd;
        int m_pipe[2];

        std::atomic<uint64_t> m_uid;

        Heap m_eventHeap;
        std::unordered_map<int, Event*> m_eventFdSet;
        std::unordered_map<uint64_t, Event*> m_timerSet;
    };

    void TestTimerEvent();
}

#endif