#ifndef CTM_IPC_MMAP_H__
#define CTM_IPC_MMAP_H__
#include <string>
#include "common/macro.h"

namespace ctm
{
	class CMmap
	{
	
	NOCOPY(CMmap)
		
	public:
		CMmap(size_t len);
		CMmap(const char* fileName, size_t len);
		~CMmap();
		
		void* Open();
		int Sync();
		void Close();

		size_t Size() const 
		{
			return m_nLen;
		}
		
	private:
		size_t m_nLen;
		std::string m_strName;
		void* m_ptr;
	};
}

#endif

