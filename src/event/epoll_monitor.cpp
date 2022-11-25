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
        m_uid = 0;
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

        Event* ev = AddIOEvent(m_pipe[0], EVENT_READ, CEpollEventMonitor::RecvNotify, this);
        if (ev == NULL) {
            ERROR("add pipe read event failed. fd:%d", m_pipe[0]);
			return -1;
        }

        return 0;
    }

    int CEpollEventMonitor::Dispatch()
    {
        int n = 0;
        int sleep = ProcessTimerEvent();
        struct epoll_event eventList[MAX_WAIT_EVENTS];

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
                ev->m_cb(ev, ToEvent(eventList[i].events), ev->m_data);
            }
        }

        return 0;
    }

    Event* CEpollEventMonitor::AddIOEvent(int fd, int events, EventCallBack cb, void* param)
    {
        int op = 0 ;
        Event* ev = NULL;

        auto it = m_eventFdSet.find(fd);
        if (it != m_eventFdSet.end()) {
            op = EPOLL_CTL_MOD;
            ev = it->second;
        } else {
            op = EPOLL_CTL_ADD;
            ev = new Event(this, GetUid(), fd, events, cb, param);
            m_eventFdSet[fd] = ev;
        }

        struct epoll_event ee;
        ee.data.ptr = ev;
        ee.events = ToEpollEvent(ev->m_events | events);

        int ret = epoll_ctl(m_epollFd, op, fd, &ee);
        if (ret == -1) {
            m_eventFdSet.erase(fd);
            delete ev;

            int err = errno;
            ERROR("epoll_ctl failed %d:%s", err, strerror(err));
            return NULL;
        }

        ev->m_events |= events;

        return ev;
    }

    Event* CEpollEventMonitor::AddTimerEvent(uint64_t milliSecond, int count, EventCallBack cb, void* param)
    {
        uint32_t timerId = GetUid();
        auto it = m_timerSet.find(timerId);
        if (it != m_timerSet.end()) {
            ERROR("timer %u is already exist.", timerId);
            return NULL;
        }

        Event *ev = new Event(this, timerId, -1, EVENT_TIMEOUT, cb, param, milliSecond, count);
        m_timerSet[timerId] = ev;
        m_eventHeap.Push(ev);

        SendNotify();

        return ev;
    }

    int CEpollEventMonitor::UpdateIOEvent(Event* ev, int events) 
    {
        if (ev->Fd() == -1) return 0;

        auto it = m_eventFdSet.find(ev->Fd());
        if (it == m_eventFdSet.end() || ev != it->second) {
            ERROR("not found fd:%d", ev->Fd());
            return -1;
        } 

        struct epoll_event ee;
        ee.data.ptr = ev;
        ee.events = ToEpollEvent(events);

        int ret = epoll_ctl(m_epollFd, EPOLL_CTL_MOD, ev->Fd(), &ee);
        if (ret == -1) {
            int err = errno;
            ERROR("epoll_ctl failed %d:%s", err, strerror(err));
            return -1;
        }

        ev->m_events = events;

        return 0;
    }

    int CEpollEventMonitor::RemoveEvent(Event* ev) 
    {
        if (ev->Fd() > 0) 
            return RemoveIOEvent(ev->Fd());

        return RemoveTimerEvent(ev->Uid());
    }

    int CEpollEventMonitor::SendNotify()
    {
        char cmd = 'x';
        int n = Write(m_pipe[1], &cmd, 1);
        if (n < 0) {
            int err = errno;
            ERROR("write failed. fd:%d, %d:%s", m_pipe[1], err, strerror(err));
        }
        
        return n;
    }

    void CEpollEventMonitor::RecvNotify(Event* ev, int events, void* data)
    {
        char buf[1024];
        int n = Read(ev->Fd(), &buf, 1024);
        if (n < 0) {
            int err = errno;
            ERROR("read failed. fd:%d, %d:%s", ev->Fd(), err, strerror(err));
        }

        DEBUG("pipe read fd:%d, events:%d, remind:%d, len:%d", ev->Fd(), events, ev->Remind(), n);
    }

    int CEpollEventMonitor::RemoveIOEvent(int fd)
    {
        auto it = m_eventFdSet.find(fd);
        if (it == m_eventFdSet.end()) {
            ERROR("not found fd:%d", fd);
            return -1;
        }

        delete it->second;
        m_eventFdSet.erase(it);

        int ret = epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, NULL);
        if (ret == -1) {
            int err = errno;
            ERROR("epoll_ctl failed. fd:%d, %d:%s", fd, err, strerror(err));
            return -1;
        }
        return 0;
    }

    int CEpollEventMonitor::RemoveTimerEvent(uint64_t uid)
    {
        auto it = m_timerSet.find(uid);
        if (it == m_timerSet.end()) {
            ERROR("not found timer:%d", uid);
            return -1;
        }

        delete it->second;
        m_timerSet.erase(it);
        m_eventHeap.Remove(it->second->Index());

        return 0;
    }

    uint64_t CEpollEventMonitor::ProcessTimerEvent() 
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
                ev->m_cb(ev, EVENT_TIMEOUT, ev->Data());
            }

            if (ev->m_remind >= ev->m_total) {
                RemoveTimerEvent(ev->m_uid);
            } else {
                ev->ResetBegin();
            }
        }

        return sleepTime;
    }

    int CEpollEventMonitor::ToEpollEvent(int events)
    {
        int epollEvents = EPOLLRDHUP | EPOLLHUP; 

        if (events & EVENT_READ) {
            epollEvents |= EPOLLIN;
        }
        
        if (events & EVENT_WRITE) {
            epollEvents |= EPOLLOUT;
        }

        return epollEvents;
    }

    int CEpollEventMonitor::ToEvent(int events)
    {
        int ctmEvent = 0;

        if (events & EPOLLIN) {
            ctmEvent |= EVENT_READ;
        }
        
        if (events & EPOLLOUT) {
            ctmEvent |= EVENT_WRITE;
        }

        if (events & EPOLLHUP) {
            ctmEvent |= EVENT_READ | EVENT_WRITE;
        }

        if (events & EPOLLRDHUP) {
            ctmEvent |=  EVENT_READ | EVENT_WRITE;
        }

        return ctmEvent;
    }

    static void Milli1(Event* ev, int events, void* param)
    {
        DEBUG("Milli 1 id:%d, events:%d, remind:%d", ev->Uid(), events, ev->Remind());
    }

    static void Milli10(Event* ev, int events, void* param)
    {
        DEBUG("Milli 10 id:%d, events:%d, remind:%d", ev->Uid(), events, ev->Remind());
    }

    static void Second(Event* ev, int events, void* param)
    {
        DEBUG("Second 1 id:%d, events:%d, remind:%d", ev->Uid(), events, ev->Remind());
    }

    static void Second5(Event* ev, int events, void* param)
    {
        DEBUG("Second 5 id:%d, events:%d, remind:%d", ev->Uid(), events, ev->Remind());
    }

    static void Second10(Event* ev, int events, void* param)
    {
        DEBUG("Second 10 id:%d, events:%d, remind:%d", ev->Uid(), events, ev->Remind());
    }

    void TestTimerEvent()
    {
        CEpollEventMonitor m;
        if (m.Init() < 0) {
            ERROR("CEpollEventMonitor init failed.");
            return;
        }

        m.AddTimerEvent(10, 10, Milli10, NULL);
        m.AddTimerEvent(1000, 10, Second, NULL);
        m.AddTimerEvent(10000, 1, Second10, NULL);
        m.AddTimerEvent(1, 10, Milli1, NULL);

        bool b;
        while (1)
        {
            m.Dispatch();
            if (!b) {
                m.AddTimerEvent(5000, 10, Second5, NULL); 
                b = true;
            }
        } 

    }
}