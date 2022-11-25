#ifndef CTM_NET_CONN_H__
#define CTM_NET_CONN_H__

namespace ctm {

    class Buffer;

    class Conn {
    public:
        virtual int Read(void *buf, int len) = 0;
        virtual int Write(void *buf, int len) = 0;
        
        virtual int Read(Buffer* buf) = 0;
        virtual int Write(Buffer* buf) = 0;

        virtual int ReadFull(Buffer* buf) = 0;
        virtual int WriteFull(Buffer* buf) = 0;

        virtual int SetNonBlock() = 0;

        virtual void Close() = 0;
        virtual bool IsClosed() = 0;
    };

    class BaseConn : public Conn {
    public:
        BaseConn(int fd) : m_fd(fd), m_closed(false) {}
        virtual ~BaseConn() { Close(); }

        virtual int Read(void *buf, int len);
        virtual int Write(void *buf, int len);

        virtual int Read(Buffer* buf);
        virtual int Write(Buffer* buf);

        virtual int ReadFull(Buffer* buf);
        virtual int WriteFull(Buffer* buf);

        virtual int SetNonBlock();

        virtual void Close();

        virtual bool IsClosed() { return m_closed; };

        int GetFd() { return m_fd; }

    protected:
        int m_fd;
        bool m_closed;
    };
}

#endif