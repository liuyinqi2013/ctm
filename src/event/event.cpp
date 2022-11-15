#include "event.h"
#include "common/time_tools.h"

namespace ctm
{

    Event::Event(uint32_t id, int fd, int events, EventCallBack cb, void* param, uint64_t interval, uint32_t count)
    {
        Clear();
        m_id = id;
        m_fd = fd;
        m_events = events;
        m_cb = cb,
        m_param = param;
        m_interval = interval;
        m_total = count;
        m_begin = MilliTimestamp();
    }

    void Event::Clear()
    {
        m_id = 0;
        m_fd = -1;
        m_remind = 0;
        m_events = EventNull;
        m_total = 0;
        m_interval = 0;
        m_begin = 0;
        m_cb = NULL;
        m_param = NULL;
    }

}