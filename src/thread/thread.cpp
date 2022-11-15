#include "thread.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

namespace ctm
{
	int CThread::Start()
	{
		if (m_iStatus == T_STOP)
		{
			bool bRet = thread_create(m_thread, CThread::ThreadEnterFun, this);
			if(!bRet) {
				fprintf(stderr, "create thread failed");
				return T_ERR;
			}
			m_iStatus = T_RUN;
		}
		
		return T_OK;
	}

	int CThread::Stop()
	{
		if(m_iStatus != T_STOP)
		{
			bool bRet = thread_stop(m_thread);
			if(!bRet) {
				fprintf(stderr, "stop thread failed");
				return T_ERR;
			}
			m_iStatus = T_STOP;
		}
		return T_OK;
	}

	int CThread::Join()
	{
		if(!m_bDetach && m_iStatus == T_RUN)
		{
			bool bRet = thread_join(m_thread);
			if(!bRet) {
				fprintf(stderr, "join thread failed");
				return T_ERR;
			}
		}
		else
		{
			return T_ERR;
		}
			
		return T_OK;
	}

	int CThread::Detach()
	{
		if(!m_bDetach)
		{
			bool bRet = thread_detach(m_thread);
			if(!bRet) {
				fprintf(stderr, "detach thread failed");
				return T_ERR;
			}

			m_bDetach = true;
		}

		return T_OK;
	}

#ifdef WIN32
	DWORD CThread::ThreadEnterFun(LPVOID arg)
	{
#else
	void* CThread::ThreadEnterFun(void* arg)
	{
		sigset_t sig_mask;
		sigfillset(&sig_mask);
		sigdelset(&sig_mask, SIGILL);
    	sigdelset(&sig_mask, SIGFPE);
    	sigdelset(&sig_mask, SIGSEGV);
		pthread_sigmask(SIG_BLOCK, &sig_mask, NULL);
#endif
		CThread *pThread = static_cast<CThread *>(arg);
		if(pThread)
		{
			pThread->Run();
		}
		
		pThread->m_iStatus = T_STOP;
		
		return NULL;
	}

	int CThread::Run()
	{
		return T_OK;
	}
}






