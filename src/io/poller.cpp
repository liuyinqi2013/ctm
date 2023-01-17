#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "io/io.h"
#include "common/log.h"
#include "common/time_tools.h"

#include "poller.h"


namespace ctm
{
    CPoller::CPoller()
    {
        m_epollFd = -1;
        m_pipe[0] = NULL;
        m_pipe[1] = NULL;
    }

    CPoller::~CPoller()
    {
        if (m_epollFd > 0 ) {
            close(m_epollFd);
            m_epollFd = 0;
        }

        if (m_pipe[0]) {
            delete(m_pipe[0]);
            delete(m_pipe[1]);
        }
    }

    int CPoller::Init()
    {
        m_epollFd = epoll_create1(0);
		if (-1 == m_epollFd) {
            int err = errno;
            ERROR("epoll_create failed %d:%s", err, strerror(err));
			return -1;
		}

        int p[2];
        int ret = pipe(p);
        if (ret < 0) {
            int err = errno;
            ERROR("pipe failed %d:%s", err, strerror(err));
			return -1;
        }

        m_pipe[0] = new CFile(p[0], this);
        m_pipe[1] = new CFile(p[1], this);
        m_pipe[0]->SetNonBlock();
        m_pipe[1]->SetNonBlock();
        m_pipe[0]->SetHandler(this);

        if (!m_pipe[0]->SetEvent(EvRead)) {
            ERROR("set read failed. fd:%d", m_pipe[0]->GetFd());
			return -1;
        }

        return 0;
    }

    int CPoller::Dispatch()
    {
        static const int maxWaitEvents = 1024;
        
        int n = 0;
        int sleep = Execute();
        struct epoll_event eventList[maxWaitEvents];

        while (1) {
            n = epoll_wait(m_epollFd, eventList, maxWaitEvents, sleep);
            if (n == -1) {
                int err = errno;
                if (EINTR == err) continue;

                ERROR("epoll_wait failed %d:%s", err, strerror(err));
                return -1;
            }

            break;
        }

        int fd;
        for (int i = 0; i < n; ++i) {
            fd = eventList[i].data.fd;
            int e = eventList[i].events;
            if ((e & EPOLLIN) || (e & EPOLLHUP) || (e & EPOLLRDHUP)) {
                auto it = m_files.find(fd);
                if (it == m_files.end()) {
                    continue;
                }
                it->second->OnRead();
            }

            if (e & EPOLLOUT) {
                auto it = m_files.find(fd);
                if (it == m_files.end()) {
                    continue;
                }
                it->second->OnWrite();
            }  
            
            if (e & EPOLLERR ){
                auto it = m_files.find(fd);
                if (it == m_files.end()) {
                    continue;
                }
                it->second->OnError();
                RemoveFile(it->second);
            }
        }

        return 0;
    }

    void CPoller::Run() 
    {
        while (!Dispatch());
    }

    int CPoller::AddFile(CFile *file, Event event)
    {
        if (event <= 0 || file->GetFd() < 0) {
            return -1;
        }

        int op = 0 ;
        int fd = file->GetFd();

        auto it = m_files.find(fd);
        if (it != m_files.end()) {
            if (file != it->second) {
                ERROR("add event failed. already exist fd:%d, ptr:%0x", fd, (uint64_t)it->second);
                return 1;
            }
            op = EPOLL_CTL_MOD;
        } else {
            op = EPOLL_CTL_ADD;
            m_files[fd] = file;
        }

        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = ToEpollEv(event);
        if (epoll_ctl(m_epollFd, op, fd, &ev) < 0) {
            int err = errno;
            ERROR("epoll_ctl failed %d:%s", err, strerror(err));
            m_files.erase(fd);
            return err;
        }

        return 0;
    }

    int CPoller::UpdateFile(CFile* file, Event event) 
    {
        if (event == EvNone) {
            return RemoveFile(file);
        }
        
        auto it = m_files.find(file->GetFd());
        if (it == m_files.end()) {
            ERROR("update event failed. not find fd:%d", file->GetFd());
            return -1;
        } 

        if (file != it->second) {
            ERROR("update event failed. event not equal fd:%d, ptr1:%0x, ptr2:%0x", file->GetFd(), (uint64_t)it->second, (uint64_t)file);
            return -1;
        }

        struct epoll_event ev;
        ev.data.fd = file->GetFd();
        ev.events = ToEpollEv(event);

        if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, file->GetFd(), &ev) < 0) {
            int err = errno;
            ERROR("epoll_ctl failed %d:%s", err, strerror(err));
            return -1;
        }

        return 0;
    }

    int CPoller::RemoveFile(CFile* file) 
    {
        DEBUG("remove file. fd:%d, event:%d", file->GetFd(), file->GetEvent());

        auto it = m_files.find(file->GetFd());
        if (it == m_files.end() ) {
            ERROR("remove event failed. not found fd:%d", file->GetFd());
            return 0;
        }

        if (file != it->second) {
            ERROR("remove event failed. fd:%d", file->GetFd());
            return -1;
        }

        m_files.erase(it);

        if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, file->GetFd(), NULL) < 0) {
            int err = errno;
            ERROR("remove event failed. fd:%d, %d:%s", file->GetFd(), err, strerror(err));
            return -1;
        }

        return 0;
    }

    uint64_t CPoller::AddTimer(uint64_t milliSecond, int count, TimerCallBack cb, void* param) 
    {
        uint64_t id = CTimerMgr::AddTimer(milliSecond, count, cb, param);
        Wakeup();
        return id;
    }

    int CPoller::Wakeup()
    {
        char cmd = 'x';
        int n = m_pipe[1]->Write(&cmd, 1);
        if (n < 0) {
            int err = errno;
            ERROR("write failed. fd:%d, %d:%s", m_pipe[1]->GetFd(), err, strerror(err));
        }
        return n;
    }

    void CPoller::OnRead(CFile* file)
    {
        char buf[1024];
        int n = file->Read(buf, 1024);
        if (n < 0) {
            int err = errno;
            ERROR("read failed. fd:%d, %d:%s", file->GetFd(), err, strerror(err));
        }
        DEBUG("pipe read fd:%d, len:%d", file->GetFd(), n);
    }

    int CPoller::ToEpollEv(Event events)
    {
        int epollEvents = EPOLLRDHUP | EPOLLHUP; 

        if (events & EvRead) {
            epollEvents |= EPOLLIN;
        }
        if (events & EvWrite) {
            epollEvents |= EPOLLOUT;
        }

        return epollEvents;
    }

    void Milli1(uint64_t timerId, uint32_t remind, void* param)
        {
            DEBUG("Milli 1 id:%d, remind:%d", timerId, remind);
        }

    void Milli10(uint64_t timerId, uint32_t remind, void* param)
        {
            DEBUG("Milli 10 id:%d, remind:%d", timerId, remind);
        }

    void Second(uint64_t timerId, uint32_t remind, void* param)
        {
            DEBUG("Second 1 id:%d, remind:%d", timerId, remind);
        }

    void Second5(uint64_t timerId, uint32_t remind, void* param)
        {
            DEBUG("Second 5 id:%d, remind:%d", timerId, remind);
        }

    void Second10(uint64_t timerId, uint32_t remind, void* param)
        {
            DEBUG("Second 10 id:%d, remind:%d", timerId, remind);
        }

    void TestTimer()
    {
        CPoller p;
        p.Init();
        p.AddTimer(10, 10, Milli10, NULL);
        p.AddTimer(1000, 10, Second, NULL);
        p.AddTimer(10000, 1, Second10, NULL);
        p.AddTimer(1, 10, Milli1, NULL);

        bool b;
        while (1)
        {
            p.Dispatch();
            if (!b) {
                p.AddTimer(5000, 10, Second5, NULL); 
                b = true;
            }
        } 
    }
}