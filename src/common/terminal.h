#ifndef CTM_COMMON_TERMINAL_H__
#define CTM_COMMON_TERMINAL_H__
#include <stdio.h>
#include <string>

namespace ctm
{
    enum CharColor
    {
        BLACK = 0x00,
        RED = 0x01,
        GREEN = 0x02,
        YELLOW = 0x03,
        BLUE = 0x04,
        PURPLE = 0x05,
        SKYBLUE = 0x06,
        WHITE = 0x07,
    };

    bool IsCharDevice(FILE* file);

    std::string ColorString(const std::string& strIn, int color);

    inline std::string Red(const std::string& strIn) 
    {
        return ColorString(strIn, CharColor::RED);
    }

    inline std::string Black(const std::string& strIn) 
    {
        return ColorString(strIn, CharColor::BLACK);
    }

    inline std::string Green(const std::string& strIn) 
    {
        return ColorString(strIn, CharColor::GREEN);
    }

    inline std::string Yellow(const std::string& strIn) 
    {
        return ColorString(strIn, CharColor::YELLOW);
    }

    inline std::string Blue(const std::string& strIn) 
    {
        return ColorString(strIn, CharColor::BLUE);
    }

    inline std::string SkyeBlue(const std::string& strIn) 
    {
        return ColorString(strIn, CharColor::SKYBLUE);
    }

    inline std::string White(const std::string& strIn) 
    {
        return ColorString(strIn, CharColor::WHITE);
    }

}

#endif
