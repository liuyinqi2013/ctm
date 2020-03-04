#ifndef CTM_COMMON_MACRO_H__
#define CTM_COMMON_MACRO_H__
#include "log.h"
#include "time_tools.h"

namespace ctm
{

#define DISABLE_COPY_ASSIGN(CLASS_NAME) \
protected:\
	CLASS_NAME(const CLASS_NAME&);\
	CLASS_NAME& operator= (const CLASS_NAME&);
}

#endif