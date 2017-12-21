#include "Thread.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>

namespace ctm
{

	bool Thread::Start()
	{
		if (m_iStatus == t_stop)
		{
			int iRet = pthread_create(&m_thread, NULL, Thread::ThreadEnterFun, this);
			if( iRet != 0) {
				fprintf(stderr, "errno : %d, errmsg : %s\n", errno, strerror(errno));
				return false;
			}
			m_iStatus = t_run;
		}
		
		return true;
	}

	bool Thread::Stop()
	{
		if(m_iStatus != t_stop)
		{
			int iRet = pthread_cancel(m_thread);
			if(iRet != 0) {
				fprintf(stderr, "errno : %d, errmsg : %s\n", errno, strerror(errno));
				return false;
			}
			m_iStatus = t_stop;
		}
		return true;
	}

	bool Thread::Join()
	{
		if(!m_bDetach)
		{
			int iRet = pthread_join(m_thread, NULL);
			if(iRet != 0) {
				fprintf(stderr, "errno : %d, errmsg : %s\n", errno, strerror(errno));
				return false;
			}
		}
		else
		{
			return false;
		}
			
		return true;
	}

	bool Thread::Detach()
	{
		if(!m_bDetach)
		{
			int iRet = pthread_detach(m_thread);
			if(iRet != 0) {
				fprintf(stderr, "errno : %d, errmsg : %s\n", errno, strerror(errno));
				return false;
			}
		}

		return true;
	}

	void* Thread::ThreadEnterFun(void* arg)
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
		return 0;
	}

}






