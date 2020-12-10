#ifndef CTM_EVENT_EPOLL_MONITOR_H__
#define CTM_EVENT_EPOLL_MONITOR_H__

#include "event.h"

namespace ctm
{
    class CEpollEventMonitor : public CEventMonitor
    {
    public:
        CEpollEventMonitor();
        virtual ~CEpollEventMonitor();

        virtual int Init();
        virtual int Done();
        virtual int WaitProc(unsigned int msec);
        virtual int AddEvent(Event* ev, int events);
        virtual int DelEvent(Event* ev, int events);
        virtual int AddConn(CConn* conn);
        virtual int DelConn(CConn* conn);
        
    private:
        int ToEpollEvent(int events);
        int ToCtmEvent(int events);

    private:
        int m_epollFd;
        int m_eventCnt;
    };
}

#endif