#ifndef CTM_EVENT_EVENT_H__
#define CTM_EVENT_EVENT_H__

namespace ctm
{
    #define EVENT_READ          0x00000001
    #define EVENT_WRITE         0x00000002
    #define EVENT_TIMEOUT       0x00000004
    #define EVENT_PEER_CLOSE    0x00000008
    #define EVENT_WRITE_EPIPE   0x00000010
    #define EVENT_ERROR         0x00000020

    #define EVENT_EPOLL_RDHUP   EVENT_PEER_CLOSE
    #define EVENT_EPOLL_ERROR   EVENT_ERROR

    #define EVENT_EPOLL_ET      0x00010000
    #define EVENT_EPOLL_ONESHOT 0x00020000
    #define EVENT_EPOLL_LLHUP   0x00040000
    
    #define EVENT_BEGIN EVENT_READ
    #define EVENT_END EVENT_EPOLL_ONESHOT

    class CConn;
    class CEventHandler;
    class CEventMonitor;

    struct Event
    {
        void* data;
        int fd;
        int events;
        int timeOut;
        int active;
        CEventHandler* handler;
        CEventMonitor* monitor;
    };

    class CEventHandler
    {
    public:
        virtual ~CEventHandler() {}
        virtual int OnProcessEvent(Event* ev, int events) = 0;
    };

    class CEventMonitor
    {
    public:
        enum 
        {
            SELECT = 1,
            POLL = 2,
            EPOLL = 3,
        };

        int   m_type;
        CEventMonitor(int type) : m_type(type) { }
        virtual ~CEventMonitor() { }
        virtual int Init() = 0;
        virtual int Done() = 0;
        virtual int WaitProc(unsigned int msec) = 0;
        virtual int AddEvent(Event* ev, int events) = 0;
        virtual int DelEvent(Event* ev, int events) = 0;
        virtual int AddConn(CConn* conn) = 0;
        virtual int DelConn(CConn* conn) = 0;
    };

    CEventMonitor* CrateEventMonitor(int type);
    void FreeEventMonitor(CEventMonitor* eventMonitor);
}

#endif