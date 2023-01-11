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
            CHandler() : m_file(NULL) {}
            virtual ~CHandler() {}
            void SetFile(CFile* file) { m_file = file; }
            CFile* GetFile() { return m_file; }

            virtual void OnRead() = 0;
            virtual void OnWrite() = 0;
            virtual void OnError() = 0;
        protected:
            CFile* m_file;
        };

        CFile(int fd, CPoller* poller = NULL) : m_fd(fd), m_closed(false), m_poller(poller), m_event(EvNone), m_handler(NULL) {}
        virtual ~CFile() { Close(); }

        virtual void OnRead() 
        {
            if (m_handler) m_handler->OnRead();
        }

        virtual void OnWrite() 
        {
            if (m_handler) m_handler->OnWrite();
        }

        virtual void OnError()
        {
            if (m_handler) m_handler->OnError();
        }

        int GetFd() { return m_fd; }

        Event GetEvent () const { return m_event; } 
        bool SetEvent(Event events);

        CPoller* GetPoller() { return m_poller; }
        void SetPoller(CPoller* poller) { m_poller = poller; }
        
        void SetHandler(CHandler* handler) 
        { 
            if (handler) handler->SetFile(this);
            m_handler = handler; 
        }

        void Close();
        
        int SetNonBlock() {  return ctm::SetNonBlock(m_fd); }

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

        CPoller *m_poller;
        Event m_event;
        CHandler* m_handler;
    };
}

#endif