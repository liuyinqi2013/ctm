#ifndef CTM_EVENT_EVENT_H__
#define CTM_EVENT_EVENT_H__

#include <stdint.h>
#include "common/heap.h"


namespace ctm
{
    #define  EventNull  0x0000
    #define  EventRead  0x0001
    #define  EventWrite 0x0002
    #define  EventTimeOut 0x0004

    class CEventMonitor;
    class CEpollEventMonitor;

    typedef void (*EventCallBack)(int fd, int events, void* param, uint32_t count);

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
        uint32_t Id() { return m_id; }


    private:
        Event(uint32_t id, int fd, int events, EventCallBack cb, void* param, uint64_t interval = -1, uint32_t count = -1);
        
        void Clear();

        void ResetBegin() 
        {
            m_begin += m_interval;
            Fix();
        }

    private:
        int m_fd;
        uint32_t m_id;
        int  m_events;

        EventCallBack m_cb;
        void* m_param;

        uint32_t m_remind;
        uint32_t m_total;
        uint64_t m_interval;
        uint64_t m_begin;

        friend class CEpollEventMonitor;
    };

    class CEventMonitor
    {
    public:
        virtual ~CEventMonitor() { }

        virtual int Init() = 0;
        virtual int Dispatch() = 0;

        virtual Event* AddEvent(int fd, int events, EventCallBack cb, void* param) = 0;
        virtual Event* AddTimer(uint64_t milliSecond, int count, EventCallBack cb, void* param) = 0;

        virtual int Update(Event* ev, int events) = 0;
        virtual int Remove(Event* ev) = 0;
    };
}

#endif