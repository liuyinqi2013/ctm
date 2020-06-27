#ifndef CTM_IPC_SEMAPHORE_H__
#define CTM_IPC_SEMAPHORE_H__
#include <string>

namespace ctm
{
	class CSemaphore
	{
	public:
		CSemaphore(int key);
		CSemaphore(const std::string& name);
		~CSemaphore();
		
		bool Open();
		bool SetVal(int val);
		bool Destroy();
		bool Post();
		bool Wait();

	private:
		bool SemOpt(int opt);
	private:
		int m_iKey;
		std::string m_strName;
		int m_semHandle;
		int m_val;
	};
}
#endif

