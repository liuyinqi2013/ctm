#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "io/io.h"
#include "common/log.h"
#include "common/time_tools.h"

#include "epoll_monitor.h"


namespace ctm
{
    #define MAX_WAIT_EVENTS 1024

    CEpollEventMonitor::CEpollEventMonitor() 
    : CEventMonitor()
    {
        m_epollFd = 0;
        m_pipe[0] = 0;
        m_pipe[1] = 0;
    }

    CEpollEventMonitor::~CEpollEventMonitor()
    {
        if (m_epollFd > 0 ) {
            close(m_epollFd);
            m_epollFd = 0;
        }

        if (m_pipe[0] > 0) {
            close(m_pipe[0]);
            close(m_pipe[1]);
        }
    }

    int CEpollEventMonitor::Init()
    {
        m_epollFd = epoll_create1(0);
		if (-1 == m_epollFd) {
            int err = errno;
            ERROR("epoll_create failed %d:%s", err, strerror(err));
			return -1;
		}

        int ret = pipe(m_pipe);
        if (ret < 0) {
            int err = errno;
            ERROR("pipe failed %d:%s", err, strerror(err));
			return -1;
        }

        SetNonBlock(m_pipe[0]);
        SetNonBlock(m_pipe[1]);

        Event* ev = AddEvent(m_pipe[0], EventRead, CEpollEventMonitor::RecvBreak, this);
        if (ev == NULL) {
            ERROR("add pipe read event failed. fd:%d", m_pipe[0]);
			return -1;
        }

        return 0;
    }

    int CEpollEventMonitor::Dispatch()
    {
        int n = 0;
        struct epoll_event eventList[MAX_WAIT_EVENTS];
        int sleep = HandlerTimeOutEvent();

        while (1) {
            n = epoll_wait(m_epollFd, eventList, MAX_WAIT_EVENTS, sleep);
            if (n == -1) {
                int err = errno;
                if (EINTR == err) continue;

                ERROR("epoll_wait failed %d:%s", err, strerror(err));
                return -1;
            }

            break;
        }

        struct Event* ev = NULL;
        for (int i = 0; i < n; ++i) {
            ev = (struct Event*)eventList[i].data.ptr;

            if (ev == NULL) {
                ERROR("epoll_wait event is null.");
                continue;
            } 

            if (ev->m_cb) {
                ev->m_cb(ev->Fd(), ToEvent(eventList[i].events), ev->m_param, ev->m_remind);
            }
        }

        return 0;
    }

    Event* CEpollEventMonitor::AddEvent(int fd, int events, EventCallBack cb, void* param)
    {
        int op = 0 ;
        Event* ev = NULL;

        auto it = m_eventFdSet.find(fd);
        if (it != m_eventFdSet.end()) {
            op = EPOLL_CTL_MOD;
            ev = it->second;
            ev->m_events |= events;
        } else {
            op = EPOLL_CTL_ADD;
            ev = new Event(GetUid(), fd, events, cb, param);
            m_eventFdSet[fd] = ev;
        }

        struct epoll_event ee;
        ee.data.ptr = ev;
        ee.events = ToEpollEvent(ev->m_events);
        int iRet = epoll_ctl(m_epollFd, op, fd, &ee);
        if (iRet == -1) {
            int err = errno;
            ERROR("epoll_ctl failed %d:%s", err, strerror(err));
            return NULL;
        }

        return ev;
    }

    Event* CEpollEventMonitor::AddTimer(uint64_t milliSecond, int count, EventCallBack cb, void* param)
    {
        uint32_t timerId = GetUid();
        auto it = m_timerSet.find(timerId);
        if (it != m_timerSet.end()) {
            ERROR("timer %u is already exist.", timerId);
            return NULL;
        }

        Event *ev = new Event(timerId, -1, EventTimeOut, cb, param, milliSecond, count);
        m_timerSet[timerId] = ev;
        m_eventHeap.Push(ev);

        SendBreak();

        return ev;
    }

    int CEpollEventMonitor::Update(Event* ev, int events) 
    {
        return 0;
    }

    int CEpollEventMonitor::Remove(Event* ev) 
    {
        if (ev->Fd() > 0) {
            return RemoveEvent(ev->Fd());
        } 
        return RemoveTimer(ev->Id());
    }


    uint32_t CEpollEventMonitor::GetUid() 
    {
        if (++m_uid == (uint32_t)-1) m_uid = 0;
        return m_uid;
    }

    int CEpollEventMonitor::SendBreak()
    {
        char cmd = 'x';
        return write(m_pipe[1], &cmd, 1);
    }

    void CEpollEventMonitor::RecvBreak(int fd, int events, void* param, uint32_t count)
    {
        DEBUG("pipe read fd:%d, events:%d, remind:%d", fd, events, count);
        char buf[1024];
        read(fd, &buf, 1024);
    }

    int CEpollEventMonitor::RemoveEvent(int fd)
    {
        auto it = m_eventFdSet.find(fd);
        if (it == m_eventFdSet.end()) {
            ERROR("not found fd:%d", fd);
            return -1;
        }

        int ret = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, NULL);
        if (ret == -1) {
            int err = errno;
            ERROR("epoll_ctl failed. fd:%d, %d:%s", fd, err, strerror(err));
            return -1;
        }

        delete it->second;
        m_eventFdSet.erase(it);

        return 0;
    }

    int CEpollEventMonitor::RemoveTimer(int timerId)
    {
        auto it = m_timerSet.find(timerId);
        if (it == m_timerSet.end()) {
            ERROR("not found timer:%d", timerId);
            return -1;
        }

        delete it->second;
        m_timerSet.erase(it);
        m_eventHeap.Remove(it->second->Index());

        return 0;
    }

    uint64_t CEpollEventMonitor::HandlerTimeOutEvent() 
    {
        uint64_t sleepTime = 100000;
        uint64_t currTime = MilliTimestamp();

        while(m_eventHeap.Len()) 
        {
            auto ev = dynamic_cast<Event*>(m_eventHeap.Top());
            if (ev->Expired() > currTime) {
                sleepTime = ev->Expired() - currTime;
                break;
            }

            ev->m_remind++;
            if (ev->m_cb) {
                ev->m_cb(ev->m_id, EventTimeOut, ev->m_param, ev->m_remind);
            }

            if (ev->m_remind >= ev->m_total) {
                RemoveTimer(ev->m_id);
            } else {
                ev->ResetBegin();
            }
        }

        return sleepTime;
    }


    int CEpollEventMonitor::ToEpollEvent(int events)
    {
        int epollEvents = EPOLLRDHUP | EPOLLHUP; 

        if (events & EventRead) {
            epollEvents |= EPOLLIN;
        }
        
        if (events & EventWrite) {
            epollEvents |= EPOLLOUT;
        }

        return epollEvents;
    }

    int CEpollEventMonitor::ToEvent(int events)
    {
        int ctmEvent = 0;

        if (events & EPOLLIN) {
            ctmEvent |= EventRead;
        }
        
        if (events & EPOLLOUT) {
            ctmEvent |= EventWrite;
        }

        if (events & EPOLLHUP) {
            ctmEvent |= EventRead | EventWrite;
        }

        if (events & EPOLLRDHUP) {
            ctmEvent |=  EventRead | EventWrite;
        }

        return ctmEvent;
    }

    static void Milli1(int timerId, int events, void* param, uint32_t remind)
    {
        DEBUG("Milli 1 id:%d, events:%d, remind:%d", timerId, events, remind);
    }

    static void Milli10(int timerId, int events, void* param, uint32_t remind)
    {
        DEBUG("Milli 10 id:%d, events:%d, remind:%d", timerId, events, remind);
    }

    static void Second(int timerId, int events, void* param, uint32_t remind)
    {
        DEBUG("Second 1 id:%d, events:%d, remind:%d", timerId, events, remind);
    }

    static void Second5(int timerId, int events, void* param, uint32_t remind)
    {
        DEBUG("Second 5 id:%d, events:%d, remind:%d", timerId, events, remind);
    }

    static void Second10(int timerId, int events, void* param, uint32_t remind)
    {
        DEBUG("Second 10 id:%d, events:%d, remind:%d", timerId, events, remind);
    }

    void TestTimerEvent()
    {
        CEpollEventMonitor m;
        if (m.Init() < 0) {
            ERROR("CEpollEventMonitor init failed.");
            return;
        }

        m.AddTimer(10, 10, Milli10, NULL);
        m.AddTimer(1000, 10, Second, NULL);
        m.AddTimer(10000, 1, Second10, NULL);
        m.AddTimer(1, 10, Milli1, NULL);

        bool b;
        while (1)
        {
            m.Dispatch();
            if (!b) {
                m.AddTimer(5000, 10, Second5, NULL); 
                b = true;
            }
        } 

    }
}