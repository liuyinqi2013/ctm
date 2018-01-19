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
			m_lock.Lock();
		}
		
		~CLockOwner()
		{
			m_lock.UnLock();
		}
		
	private:
		CLock& m_lock;
	};
	
};

#endif