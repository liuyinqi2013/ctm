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

		typedef enum
		{
			t_succeed = 0,
			t_error   = 1,
		} terrno;
		
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
		
		virtual ~Thread(){}
		
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

		unsigned int GetThreadId() const
		{
			return m_thread;
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

