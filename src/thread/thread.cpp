#include "thread.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>

namespace ctm
{

	int Thread::Start()
	{
		if (m_iStatus == t_stop)
		{
			bool bRet = thread_create(m_thread, Thread::ThreadEnterFun, this);
			if(!bRet) {
				fprintf(stderr, "create thread failed");
				return t_error;
			}
			m_iStatus = t_run;
		}
		
		return t_succeed;
	}

	int Thread::Stop()
	{
		if(m_iStatus != t_stop)
		{
			bool bRet = thread_stop(m_thread);
			if(!bRet) {
				fprintf(stderr, "stop thread failed");
				return t_error;
			}
			m_iStatus = t_stop;
		}
		return t_succeed;
	}

	int Thread::Join()
	{
		if(!m_bDetach)
		{
			bool bRet = thread_join(m_thread);
			if(!bRet) {
				fprintf(stderr, "join thread failed");
				return t_error;
			}
		}
		else
		{
			return t_error;
		}
			
		return t_succeed;
	}

	int Thread::Detach()
	{
		if(!m_bDetach)
		{
			bool bRet = thread_detach(m_thread);
			if(!bRet) {
				fprintf(stderr, "detach thread failed");
				return t_error;
			}

			m_bDetach = true;
		}

		return t_succeed;
	}

#ifdef WIN32
	DWORD Thread::ThreadEnterFun(LPVOID arg)
#else
	void* Thread::ThreadEnterFun(void* arg)
#endif
	{
		Thread *pThread = static_cast<Thread *>(arg);
		if(pThread)
		{
			pThread->Run();
		}
		
		pThread->m_iStatus = t_stop;
		
		return NULL;
	}


	int Thread::Run()
	{
		return t_succeed;
	}

}






