#ifndef CTM_COMMON_MACRO_H__
#define CTM_COMMON_MACRO_H__
#include <stdio.h>
#include "time_tools.h"

namespace ctm
{

#define NOCOPY(T) \
protected:\
	T(const T&);\
	T& operator= (const T&);

#define DEBUG_LOG(format,...) fprintf(stdout, "[%s][debug][%s:%d][%s]:"format"\n", DateTime().c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define ERROR_LOG(format,...) fprintf(stderr, "[%s][error][%s:%d][%s]:"format"\n", DateTime().c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

}

#endif