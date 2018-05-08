#ifndef CTM_COMMON_RANDOM_H__
#define CTM_COMMON_RANDOM_H__
#include <time.h>
#include <stdlib.h>

namespace ctm
{
	class CRandom
	{
	public:
		static void SetSeed(unsigned int seed = time(NULL))
		{
			return srand(seed);
		}

		static int Random()
		{
			return rand();
		}
	
		static float Random0()
		{
			return ((float)rand()) / RAND_MAX;
		}
		
		static int Random(int beg, int end)
		{
			return beg + rand() % (abs(end - beg) + 1);
		}
	};
};

#endif

