#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "net/conn.h"
#include "common/log.h"
#include "epoll_monitor.h"

namespace ctm
{
    #define MAX_WAIT_EVENTS 1024

    static struct epoll_event eventList[MAX_WAIT_EVENTS];

    CEpollEventMonitor::CEpollEventMonitor() 
    : CEventMonitor(CEventMonitor::EPOLL), 
      m_epollFd(0),
      m_eventCnt(0)
    {
    }

    CEpollEventMonitor::~CEpollEventMonitor()
    {
        Done();
    }

    int CEpollEventMonitor::Init()
    {
        m_epollFd = epoll_create(1024 * 10);
		if (-1 == m_epollFd)
		{
            int err = errno;
            ERROR("epoll_create failed %d:%s", err, strerror(err));
			return -1;
		}

        return 0;
    }

    int CEpollEventMonitor::Done()
    {
        if (m_epollFd > 0 )
        {
            close(m_epollFd);
            m_epollFd = 0;
        }

        return 0;
    }

    int CEpollEventMonitor::WaitProc(unsigned int msec)
    {
        if (m_eventCnt <= 0)
        {
           return 1; 
        }

        int n = 0;
        
        while (1)
        {
            n = epoll_wait(m_epollFd, eventList, MAX_WAIT_EVENTS, msec);

            if (n == -1)
            {
                int err = errno;
                if (EINTR == err) continue;
                ERROR("epoll_wait failed %d:%s", err, strerror(err));
                return -1;
            }
            else if (n == 0)
            {
                return 1;
            }

            break;
        }

        struct Event* ev = NULL;
        for (int i = 0; i < n; ++i)
        {
            ev = (struct Event*)eventList[i].data.ptr;
            if (ev == NULL || ev->handler == NULL) 
            {
                ERROR("epoll_wait event is null.");
                continue;
            }

            ev->handler->OnProcessEvent(ev, ev->events & ToCtmEvent(eventList[i].events));
        }

        return 0;
    }

    int CEpollEventMonitor::AddEvent(Event* ev, int events)
    {
        int op = 0;
        struct epoll_event ee;

        if (ev->handler == NULL)
        {
            ERROR("Event handler is null");
            return -1;
        }

        if (ev->active)
        {
            op = EPOLL_CTL_MOD;
            ee.events = ToEpollEvent(ev->events) | ToEpollEvent(events);
        }
        else
        {
            op = EPOLL_CTL_ADD;
            ee.events = ToEpollEvent(events);
        }

        ee.data.ptr = ev;
        int iRet = epoll_ctl(m_epollFd, op, ev->fd, &ee);
        if (iRet == -1)
        {
            int err = errno;
            ERROR("epoll_ctl failed %d:%s", err, strerror(err));
            return -1;
        }

        ev->events |= events;
        ev->active = 1;

        if (op == EPOLL_CTL_ADD) ++m_eventCnt;

        return 0;
    }

    int CEpollEventMonitor::DelEvent(Event* ev, int events)
    {
        if (ev->active == 0)
        {
            ERROR("Event active is 0 fd:%d", ev->fd);
            return -1;
        }

        int op = 0;
        struct epoll_event ee;

        if (ev->active && (ev->events & ~events))
        {
            op = EPOLL_CTL_MOD;
            ee.events = ToEpollEvent(ev->events) & ~ToEpollEvent(events);
            ee.data.ptr = ev;
        }
        else
        {
            DEBUG("Delete ev fd : %d", ev->fd);

            op = EPOLL_CTL_DEL;
            ee.events = 0;
            ee.data.ptr = NULL;
        }

        int iRet = epoll_ctl(m_epollFd, op, ev->fd, &ee);
        if (iRet == -1)
        {
            int err = errno;
            ERROR("epoll_ctl failed %d:%s", err, strerror(err));
            return -1;
        }

        ev->events &= ~events;
        if (op == EPOLL_CTL_DEL)
        {
            --m_eventCnt;
            ev->active = 0; 
        }
            
        return 0;
    }

    int CEpollEventMonitor::AddConn(CConn* conn)
    {
        if (conn->event.active)
        {
            //do someting
        }
    
        conn->event.data = conn;
        conn->event.fd = conn->fd;
        conn->event.monitor = this;

        return AddEvent(&conn->event, EVENT_READ | EVENT_WRITE | EVENT_PEER_CLOSE);
    }

    int CEpollEventMonitor::DelConn(CConn* conn)
    {
        if (conn->event.active == 0)
        {
            ERROR("Conn event active is 0 : [%s]", conn->ToString().c_str());
            return -1;
        }

        int iRet = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, conn->fd, NULL);
        if (iRet == -1)
        {
            int err = errno;
            ERROR("epoll_ctl failed %d:%s [%s]", err, strerror(err), conn->ToString().c_str());
            return -1;
        }

        DEBUG("Del conn :[%s]", conn->ToString().c_str());
        
        conn->event.events  = 0;
        conn->event.active  = 0;
        --m_eventCnt;

        return 0;
    }

    int CEpollEventMonitor::ToEpollEvent(int events)
    {
        int epollEvents = 0;

        if (events & EVENT_READ)
        {
            epollEvents |= EPOLLIN;
        }

        if (events & EVENT_WRITE)
        {
            epollEvents |= EPOLLOUT;
        }

        if (events & EVENT_EPOLL_RDHUP)
        {
            epollEvents |= EPOLLRDHUP;
        }

        if (events & EVENT_EPOLL_ET)
        {
            epollEvents |= EPOLLET;
        }

        if (events & EVENT_EPOLL_ONESHOT)
        {
            epollEvents |= EPOLLONESHOT;
        }

        return epollEvents;
    }

    int CEpollEventMonitor::ToCtmEvent(int events)
    {
        int ctmEvent = 0;

        if (events & EPOLLIN)
        {
            ctmEvent |= EVENT_READ;
        }
        
        if (events & EPOLLOUT)
        {
            ctmEvent |= EVENT_WRITE;
        }

        if (events & EPOLLHUP)
        {
            // ctmEvent |= EVENT_EPOLL_LLHUP;
            ctmEvent |= EVENT_WRITE | EVENT_READ;
        }

        if (events & EPOLLRDHUP)
        {
            // ctmEvent |= EVENT_PEER_CLOSE;
            ctmEvent |= EVENT_READ | EVENT_WRITE;
        }

        if (events & EPOLLERR)
        {
            // ctmEvent |= EVENT_ERROR(;
            ctmEvent |= EVENT_WRITE | EVENT_READ;
        }

        return ctmEvent;
    }
}