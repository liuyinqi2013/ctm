
#include "common/log.h"

#include "file.h"
#include "poller.h"

namespace ctm
{
    bool CFile::SetEvent(Event event) 
    {
        if (!m_poller || m_fd == -1) {
            ERROR("set events failed. poller:%x, event:%d, fd:%d", m_poller, event, m_fd);
            return false;
        }

        int ret = 0;
        if (m_event == EvNone) {
            if (event > 0) {
                SetNonBlock();
                ret = m_poller->AddFile(this, event);
            }
        } else {
            if (event == EvNone) {
                ret = m_poller->RemoveFile(this);
            } else {
                ret = m_poller->UpdateFile(this, event);
            }
        }

        if (ret != 0) {
            ERROR("set events failed. ret:%d", ret);
            return false;
        }

        m_event = event;

        return true;
    }

    void CFile::Close() 
    {
        if (!m_closed) {
            DEBUG("close file. fd:%d", m_fd);
            SetEvent(EvNone);
            ctm::Close(m_fd);
            m_closed = true;
            m_fd = -1;
        }
    }
}