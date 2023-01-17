#ifndef CTM_IO_FILE_H__
#define CTM_IO_FILE_H__

#include <stdint.h>

#include "io.h"

namespace ctm
{
    enum Event {
        EvNone      = 0,
        EvRead      = 1,
        EvWrite     = 2,
        EvReadWrite = 3,
    }; 

    class CPoller;

    class CFile
    {
    public:
        class CHandler 
        {
        public:
            virtual ~CHandler() {}

            virtual void OnRead(CFile* file)  { DEBUG("on read. fd:%d", file->GetFd()); }
            virtual void OnWrite(CFile* file) { DEBUG("on write. fd:%d", file->GetFd()); };
            virtual void OnError(CFile* file) { DEBUG("on error. fd:%d", file->GetFd()); file->Close(); };
        };

        CFile(int fd, CPoller* poller = NULL) : m_fd(fd), m_closed(false), m_event(EvNone), m_poller(poller), m_handler(NULL) {}
        virtual ~CFile() 
        { 
            Close();
        }

        virtual void OnRead() 
        {
            if (m_handler) m_handler->OnRead(this);
        }

        virtual void OnWrite() 
        {
            if (m_handler) m_handler->OnWrite(this);
        }

        virtual void OnError()
        {
            if (m_handler) m_handler->OnError(this);
        }

        int GetFd() 
        { 
            return m_fd; 
        }

        Event GetEvent () const 
        { 
            return m_event; 
        } 
        bool SetEvent(Event events);

        CPoller* GetPoller() 
        { 
            return m_poller; 
        }
        
        void SetPoller(CPoller* poller) 
        { 
            m_poller = poller; 
        }
        
        void SetHandler(CHandler* handler) 
        { 
            m_handler = handler; 
        }

        void Close();
        
        int SetNonBlock() 
        {  
            return ctm::SetNonBlock(m_fd); 
        }

        int Read(void* buf,  uint32_t len) 
        { 
            if (m_closed) return IO_EOF;
            return ctm::Read(m_fd, buf, len); 
        }

        int Write(void* buf, uint32_t len) 
        {
            if (m_closed) return IO_EOF;
            return ctm::Write(m_fd, buf, len); 
        }

        int Read(Buffer* buf) 
        {
            if (m_closed) return IO_EOF;
            return ctm::Read(m_fd, buf); 
        }

        int Write(Buffer* buf) 
        {
            if (m_closed) return IO_EOF;
            return ctm::Write(m_fd, buf); 
        }

        int ReadFull(Buffer* buf) 
        {
            if (m_closed) return IO_EOF;
            return ctm::ReadFull(m_fd, buf);
        }

        int WriteFull(Buffer* buf) 
        {
            if (m_closed) return IO_EOF;
            return ctm::WriteFull(m_fd, buf); 
        }

        int ReadAll(std::string & out) 
        {
            if (m_closed) return IO_EOF;
            return ctm::ReadAll(m_fd, out); 
        }

        bool IsClosed() const 
        { 
            return m_closed; 
        }

        void Detach() 
        { 
            SetEvent(EvNone); 
        }

    private:
        int m_fd;
        bool m_closed; 

        Event m_event;
        CPoller* m_poller;
        CHandler* m_handler;
    };
}

#endif