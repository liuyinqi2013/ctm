#ifndef CTM_COMMON_MACRO_H__
#define CTM_COMMON_MACRO_H__
#include "log.h"
#include "time_tools.h"

namespace ctm
{

#define NOCOPY(T) \
protected:\
	T(const T&);\
	T& operator= (const T&);
}

#endif