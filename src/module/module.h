#ifndef CTM_MODULE_MODULE_H__
#define CTM_MODULE_MODULE_H__
#include "common/macro.h"

namespace ctm
{
    class CMessage;
    class CMessgaeQueue;

    class CModule
    {
        DISABLE_COPY_ASSIGN(CModule);
    public:
        CModule(CModule* parent = NULL) : m_parent(NULL), m_outMessageQueue(0) {}
        virtual ~CModule() {}
        virtual int Init() { return 0; }
        virtual int UnInit() { return 0; }
        virtual int OnRunning() { return 0; }

        void SetOutMessageQueue(CMessgaeQueue* messageQueue)
        {
            m_outMessageQueue = messageQueue;
        }
    protected:
        CModule* m_parent;
        CMessgaeQueue* m_outMessageQueue;
    };
};

#endif