#include "platform.h"

#ifdef WIN32
#include <Windows.h>
#include <process.h>

#define  getpid() _getpid()
#define  getppid() _getpid()

#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace ctm
{
	unsigned int GetPid()
	{
		return  getpid();
	}
	
	unsigned int GetPPid()
	{
		return getppid();
	}
};
