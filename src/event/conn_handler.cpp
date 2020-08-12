#include "conn_handler.h"
#include "net/conn.h"
#include "net/socket.h"

namespace ctm
{
    CConnHandler::CConnHandler()
    {

    }

    CConnHandler::~CConnHandler()
    {

    }

    int CConnHandler::OnProcessEvent(Event* ev, int events)
    {
        CConn* conn = (CConn*)ev->data;
        if (conn->status == CConn::CONNING)
        {
            if ((events & EVENT_READ) && (events & EVENT_WRITE))
            {
                conn->status = CConn::CONN_TIME_OUT;
                conn->event.monitor->DelConn(conn);
                conn->action->OnAsynConnTimeOut(conn);
            }
            else if(events & EVENT_WRITE)
            {
                conn->status = CConn::ACTIVE;
                conn->event.monitor->DelEvent(&conn->event, EVENT_WRITE);
                conn->action->OnAsynConnOk(conn);
            }
            else if(events & EVENT_EPOLL_LLHUP)
            {
                CTM_ERROR_LOG(conn->log, "Conn hanpend LLHUP event:[%s]", conn->ToString().c_str());

                // conn->status = CConn::EXCEPT;
                // conn->event.monitor->DelConn(conn);
                // conn->action->OnException(conn);
            }
        }
        else
        {
            if (events & EVENT_READ)
            {
                if (conn->isListen)
                {
                    conn->action->OnAccept(conn);
                }
                else
                {
                    conn->action->OnRead(conn);
                }
            }

            if (events & EVENT_WRITE)
            {
                conn->action->OnWrite(conn);
            }

            if(events & EVENT_EPOLL_LLHUP)
            {
                CTM_ERROR_LOG(conn->log, "Conn hanpend llhup event:[%s]", conn->ToString().c_str());

                // conn->status = CConn::EXCEPT;
                // conn->event.monitor->DelConn(conn);
                // conn->action->OnException(conn);
            }
        }
        
        return 0;
    }
}