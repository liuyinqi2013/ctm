#ifndef CTM_COMMON_LOCK_H__
#define CTM_COMMON_LOCK_H__

namespace ctm
{
	class CLock
	{
	public:
		CLock(){}
		virtual ~CLock(){}
		
		virtual bool Lock() = 0;
		virtual bool TryLock() = 0;
		virtual bool UnLock() = 0;
	};
	
	class CLockOwner
	{
	public:
		CLockOwner(CLock& lock) : m_lock(lock)
		{
			m_success = m_lock.Lock();
		}
		
		~CLockOwner()
		{
			if (m_success) m_lock.UnLock();
		}
		
	private:
		bool m_success;
		CLock& m_lock;
	};
	
};

#endif