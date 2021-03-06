#include "event.h"
#include "epoll_monitor.h"
#include "conn_handler.h"

namespace ctm
{
    CEventMonitor* CrateEventMonitor(int type)
    {
        switch (type)
        {
        case CEventMonitor::SELECT:
        case CEventMonitor::POLL:
            return NULL;
        case CEventMonitor::EPOLL:
            return new CEpollEventMonitor();
        default:
            break;
        }
        return NULL;
    }

    void FreeEventMonitor(CEventMonitor* eventMonitor)
    {
        if (eventMonitor)
        {
            delete eventMonitor;
        }
    }

}