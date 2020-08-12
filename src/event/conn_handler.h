#ifndef CTM_EVENT_CONN_HANDER_H__
#define CTM_EVENT_CONN_HANDER_H__
 
#include "event.h"
#include "net/conn.h"

namespace ctm
{
    class CConnHandler : public CEventHandler
    {
    public:
        CConnHandler();

        virtual ~CConnHandler();

        virtual int OnProcessEvent(Event* ev, int events);
    };
}

#endif