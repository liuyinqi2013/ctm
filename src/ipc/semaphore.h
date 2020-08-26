#ifndef CTM_IPC_SEMAPHORE_H__
#define CTM_IPC_SEMAPHORE_H__
#include <string>

namespace ctm
{

#ifndef _SEMUN

	union semun
	{
	    int val;  
	    struct semid_ds *buf;  
	    unsigned short  *array;  
	    struct seminfo  *__buf;   
	};

#endif

	class CSemaphore
	{
	public:
		CSemaphore(int key, int semSize = 1);
		CSemaphore(const std::string& name, int semSize = 1);
		~CSemaphore();
		
		bool Open();
		bool Destroy();

		bool SetVal(int val, int semNum = 0);
		bool Post(int semNum = 0);
		bool Wait(int semNum = 0);

		int GetSemId() const { return m_semHandle; }
		int GetSemSize() const { return m_semSize; }

	private:
		bool SemOpt(int opt, int semNum = 0);
	private:
		int m_iKey;
		std::string m_strName;
		int m_semHandle;
		int m_val;
		int m_semSize;
	};
}
#endif

