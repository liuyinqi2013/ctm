#include "io/io.h"
#include "conn.h"

namespace ctm {

    int BaseConn::Read(void *buf, int len)
    {
        if (m_closed) {
            return IO_EOF;
        }
        return ctm::Read(m_fd, buf, len);
    }

    int BaseConn::Write(void *buf, int len)
    {
        if (m_closed) {
            return IO_EOF;
        }
        return ctm::Write(m_fd, buf, len);
    }

    int BaseConn::Read(Buffer* buf)
    {
        if (m_closed) {
            return IO_EOF;
        }
        return ctm::Read(m_fd, buf);
    }

    int BaseConn::Write(Buffer* buf)
    {
        if (m_closed) {
            return IO_EOF;
        }
        return ctm::Write(m_fd, buf);
    }


    int BaseConn::ReadFull(Buffer* buf)
    {
        if (m_closed) {
            return IO_EOF;
        }
        return ctm::ReadFull(m_fd, buf);
    }

    int BaseConn::WriteFull(Buffer* buf)
    {
        if (m_closed) {
            return IO_EOF;
        }
        return ctm::WriteFull(m_fd, buf);
    }

    int BaseConn::SetNonBlock()
    {
        return ctm::SetNonBlock(m_fd);
    }

    void BaseConn::Close()
    {
        if (m_closed) {
            return;
        }

        ctm::Close(m_fd);
        m_closed = true;
    }

}