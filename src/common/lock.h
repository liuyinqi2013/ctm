#ifndef CTM_COMMON_LOCK_H__
#define CTM_COMMON_LOCK_H__

namespace ctm
{
	class BaseLock
	{
	public:
		BaseLock(){}
		virtual ~BaseLock(){}
		
		virtual bool Lock() = 0;
		virtual bool TryLock() = 0;
		virtual bool UnLock() = 0;
	};
	
	class LockOwner
	{
	public:
		LockOwner(BaseLock& lock) : m_lock(lock)
		{
			m_lock.Lock();
		}
		
		~LockOwner()
		{
			m_lock.UnLock();
		}
		
	private:
		BaseLock& m_lock;
	};
	
};

#endif