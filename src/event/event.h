#ifndef CTM_EVENT_EVENT_H__
#define CTM_EVENT_EVENT_H__

#include <stdint.h>
#include "common/heap.h"


namespace ctm
{
    #define  EVENT_NULL  0x0000
    #define  EVENT_READ  0x0001
    #define  EVENT_WRITE 0x0002
    #define  EVENT_TIMEOUT 0x0004

    #define RD(events) (events & EVENT_READ)
    #define WR(events) (events & EVENT_WRITE)
    #define TO(events) (events & EVENT_TIMEOUT)
    #define AND_RW(events) (RD(events) && WR(events))
    #define OR_RW(events) (RD(events) || WR(events))


    class Event;
    class CEventMonitor;
    class CEpollEventMonitor;

    typedef void (*EventCallBack)(Event* ev, int events, void* data);

    class Event : public HeapItem
    {
    public:
        virtual ~Event() {}

        bool Less(const HeapItem* other) 
        {
            return Expired() < dynamic_cast<const Event*>(other)->Expired();
        }

        uint64_t Expired() const
        {
            return m_begin + m_interval;
        }

        int Fd() { return m_fd; }
        uint64_t Uid() { return m_uid; }
        uint32_t Remind() { return m_remind; }
        void* Data() { return m_data; }
        void Destroy();

    private:
        Event(CEventMonitor* monitor, uint64_t uid, int fd, int events, EventCallBack cb, void* data, uint64_t interval = -1, uint32_t count = -1);
        
        void Clear();

        void ResetBegin() 
        {
            m_begin += m_interval;
            Fix();
        }

    private:
        uint64_t m_uid;
        int m_fd;
        int  m_events;

        EventCallBack m_cb;
        void* m_data;

        uint32_t m_remind;
        uint32_t m_total;
        uint64_t m_interval;
        uint64_t m_begin;
        CEventMonitor* m_monitor;

        friend class CEpollEventMonitor;
    };

    class CEventMonitor
    {
    public:
        virtual ~CEventMonitor() { }

        virtual int Init() = 0;
        virtual int Dispatch() = 0;

        virtual Event* AddIOEvent(int fd, int events, EventCallBack cb, void* data) = 0;
        virtual Event* AddTimerEvent(uint64_t milliSecond, int count, EventCallBack cb, void* data) = 0;

        virtual int UpdateIOEvent(Event* ev, int events) = 0;
        virtual int RemoveEvent(Event* ev) = 0;
    };
}

#endif