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
			int iRet = pthread_create(&m_thread, NULL, Thread::ThreadEnterFun, this);
			if( iRet != 0) {
				fprintf(stderr, "errno : %d", iRet);
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
			int iRet = pthread_cancel(m_thread);
			if(iRet != 0) {
				fprintf(stderr, "errno : %d", iRet);
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
			int iRet = pthread_join(m_thread, NULL);
			if(iRet != 0) {
				fprintf(stderr, "errno : %d", iRet);
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
			int iRet = pthread_detach(m_thread);
			if(iRet != 0) {
				fprintf(stderr, "errno : %d", iRet);
				return t_error;
			}

			m_bDetach = true;
		}

		return t_succeed;
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
		return t_succeed;
	}

}






