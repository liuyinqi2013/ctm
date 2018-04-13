#include "platform.h"

#ifdef WIN32
#include <Windows.h>
#define  getpid() GetCurrentProcessId()
#define  getppid() GetCurrentProcessId()

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
