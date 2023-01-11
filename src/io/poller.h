#ifndef CTM_IO_POLLer_H__
#define CTM_IO_POLLer_H__

#include <unordered_map>
#include "timer.h"
#include "file.h"

namespace ctm
{
    class CPoller : public CTimerMgr, CFile::CHandler
    {
    public:
        CPoller();
        virtual ~CPoller();

        int Init();
        int Dispatch();
        void Run();

        int AddFile(CFile *ev, Event event);
        int UpdateFile(CFile* ev, Event event);
        int RemoveFile(CFile* ev); 
        
        virtual uint64_t AddTimer(uint64_t milliSecond, int count, TimerCallBack cb, void* param);
        
        virtual void OnRead();
        virtual void OnWrite() {};
        virtual void OnError() {};
    private:
        int ToEpollEv(Event events);
        int Wakeup();

    private:
        int m_epollFd;
        CFile* m_pipe[2];
        std::unordered_map<int, CFile*> m_files;
        std::unordered_map<uint64_t, CFile*> m_uidFiles;
    };

    void TestTimer();
}

#endif