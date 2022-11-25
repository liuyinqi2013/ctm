#include "event.h"
#include "common/time_tools.h"

namespace ctm
{

    Event::Event(CEventMonitor* monitor, uint64_t uid, int fd, int events, EventCallBack cb, void* data, uint64_t interval, uint32_t count)
    {
        Clear();
        m_monitor = monitor;
        m_uid = uid;
        m_fd = fd;
        m_events = events;
        m_cb = cb,
        m_data = data;
        m_interval = interval;
        m_total = count;
        m_begin = MilliTimestamp();
    }

    void Event::Clear()
    {
        m_monitor = NULL;
        m_uid = 0;
        m_fd = -1;
        m_remind = 0;
        m_events = EVENT_NULL;
        m_total = 0;
        m_interval = 0;
        m_begin = 0;
        m_cb = NULL;
        m_data= NULL;
    }

    void Event::Destroy() { m_monitor->RemoveEvent(this); }

}