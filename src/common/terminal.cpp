#include "terminal.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ctm
{
    bool IsCharDevice(FILE* file)
    {
        if (file == NULL) {
            return false;
        }

        int fd = fileno(file);
        struct stat sb;
        if (fstat(fd, &sb) == -1) {
            return false;
        }

        if (S_IFCHR == (sb.st_mode & S_IFMT)) {
            return true;
        }

        return false;
    }

    std::string ColorString(const std::string& strIn, int color)
    {
        switch (color)
        {
        case BLACK:
            return "\033[30m" + strIn +"\033[0m";
        case RED:
            return "\033[31m" + strIn +"\033[0m";
        case GREEN:
            return "\033[32m" + strIn +"\033[0m";
        case YELLOW:
            return "\033[33m" + strIn +"\033[0m";
        case BLUE:
            return "\033[34m" + strIn +"\033[0m";
        case PURPLE:
            return "\033[35m" + strIn +"\033[0m";
        case SKYBLUE:
            return "\033[36m" + strIn +"\033[0m";
        case WHITE:
            return "\033[37m" + strIn +"\033[0m";
        default:
            break;
        }

        return strIn;
    }
}
