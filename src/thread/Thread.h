#ifndef _h_ctm_thread_h
#define _h_ctm_thread_h

#include "common/macro.h"

#include <string>
#include <pthread.h>

namespace ctm
{
	class Thread
	{
		NOCOPY(Thread)
			
	public:
		typedef void* (*ThreadFun)(void*);
		
		typedef enum
		{
			t_stop = 0,
			t_run  = 1,
			t_pause = 2,
			t_suspend = 3,
		} tstatus;
		
		Thread() : 
			m_iStatus(t_stop), 
			m_bDetach(false), 
			m_strName("")
		{
		}

		Thread(std::string& name) : 
			m_iStatus(t_stop),
			m_bDetach(false), 
			m_strName(name)
		{
		}
		
		virtual ~Thread(){};
		
		bool Start();
		
		bool Stop();

		bool Join();

		bool Detach();

		int GetStatus() const 
		{
			return m_iStatus;
		}

		const std::string& GetName() const
		{
			return m_strName;
		}
		
	protected:
		static void* ThreadEnterFun(void* arg);
		
		virtual int Run();
	private:
		pthread_t m_thread;
		
		int  m_iStatus;
		bool m_bDetach;
		std::string m_strName;
	};
}

#endif

