#ifndef CTM_COMMON_MACRO_H__
#define CTM_COMMON_MACRO_H__
#include "log.h"
#include "com.h"

namespace ctm
{

#define DISABLE_COPY_ASSIGN(CLASS_NAME) \
protected:\
	CLASS_NAME(const CLASS_NAME&);\
	CLASS_NAME& operator= (const CLASS_NAME&);
}

#define DELETE(ptr) { if (ptr) { delete ptr; ptr = NULL; } }
#define DELETE_ARRAY(ptr) { if (ptr) { delete[] ptr; ptr = NULL; } }

typedef int int32;
typedef short int16;
typedef unsigned char uchar;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

#endif