#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "ipc_common.h"

namespace ctm
{
    int IpcKeyId(const char* fileName)
    {
        if (!fileName) return -1;
		std::string filePathName = std::string("/tmp/") + fileName;
		if (access(filePathName.c_str(), F_OK) == -1)
		{
			FILE* fp = fopen(filePathName.c_str(), "wb");
			if (!fp){
				return -1;
			}
			fclose(fp);
		}
        
		return ftok(filePathName.c_str(), 0x666);
    }
}