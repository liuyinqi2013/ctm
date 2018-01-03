#ifndef CTM_THREAD_THREAD_H__
#define CTM_THREAD_THREAD_H__

#include "common/macro.h"

#include <string>
#ifdef WIN32
#include <Windows.h>
typedef DWORD (*ThreadFun)(LPVOID);
#define thread_t HANDLE 
#define thread_create(tid, func, param) ((tid = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)param, 0, NULL)) ? true : false)
#define thread_join(tid)   (WaitForSingleObject(tid, INFINITE) ? true : false)
#define thread_detach(tid) (true)
#define thread_stop(tid)   (TerminateThread(tid, 0))
#define thread_close(tid)  (CloseHandle(tid))

#else
#include <pthread.h>
typedef void* (*ThreadFun)(void*);
#define thread_t pthread_t
#define thread_create(tid, func, param) (!pthread_create(&tid, NULL, func, param) ? true : false)
#define thread_join(tid)   (!pthread_join(tid, NULL) ? true : false)
#define thread_detach(tid) (!pthread_detach(tid) ? true : false)
#define thread_stop(tid)   (!pthread_cancel(tid) ? true : false)
#define thread_close(tid)  
#endif

namespace ctm
{
	class Thread
	{
		NOCOPY(Thread)		
	public:
		typedef enum
		{
			T_STOP = 0,
			T_RUN  = 1,
			T_PAUSE = 2,
			T_SUSPEND = 3
		} tstatus;

		typedef enum
		{
			T_OK = 0,
			T_ERR   = 1
		} terrno;
		
		Thread() : 
			m_iStatus(T_STOP), 
			m_bDetach(false), 
			m_strName("")
		{
		}

		Thread(const std::string& name) : 
			m_iStatus(T_STOP),
			m_bDetach(false), 
			m_strName(name)
		{
		}
		
		virtual ~Thread()
		{
			thread_close(m_thread);
		}
		
		int Start();
		
		int Stop();

		int Join();

		int Detach();

		int GetStatus() const 
		{
			return m_iStatus;
		}

		const std::string& GetName() const
		{
			return m_strName;
		}

		void SetName(const std::string& name)
		{
			m_strName = name;
		}

	protected:
		
#ifdef WIN32
		static DWORD ThreadEnterFun(LPVOID arg);
#else
		static void* ThreadEnterFun(void* arg);
#endif
		
		virtual int Run();
	private:
		thread_t m_thread;
		
		int  m_iStatus;
		bool m_bDetach;
		std::string m_strName;
	};
}

#endif

