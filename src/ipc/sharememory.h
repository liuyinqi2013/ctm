#ifndef CTM_IPC_SHAREMEMORY_H__
#define CTM_IPC_SHAREMEMORY_H__
#include <string>

namespace ctm
{
	class CShareMemory
	{
	public:
		CShareMemory(int key);
		CShareMemory(const std::string& name);
		~CShareMemory();

		bool Create(int size);
		
		bool Open(int size, bool creat = false);

		int Size() const
		{
			return m_Size;
		}

		char* Head() const
		{
			return m_shmHead;
		}

		char* Tail() const
		{
			return (m_shmHead + m_Size);
		}

		bool Destroy();
	
	private:

		int KeyId(const std::string& name);

		void GetSize();

	private:
		int m_iKey;
		std::string m_strName;
		int m_shmId;
		int m_Size;
		char* m_shmHead;
	};
}
#endif

