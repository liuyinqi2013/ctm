#include "random.h"

namespace ctm
{
    CRandom CRandom::global_random;
    
    unsigned int CRandom::UIntRandom(unsigned int beg, unsigned int end)
    {
        if (beg > end)
        {
            unsigned int tmp = beg;
            beg = end;
            end = tmp;
        }
        return beg + rand() % (end - beg + 1);
    }

    double CRandom::DoubleRandom(double beg, double end, unsigned int ratio)
    {
        unsigned int dep = abs((end - beg) * ratio) + 1;
        return beg + rand() % dep / (double)ratio;
    }

}; // namespace ctm
