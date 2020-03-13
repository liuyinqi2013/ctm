#ifndef CTM_COMMON_RANDOM_H__
#define CTM_COMMON_RANDOM_H__
#include <time.h>
#include <stdlib.h>

namespace ctm
{
	class CRandom
	{

	public:
		static inline void SetSeed(unsigned int seed = time(NULL))
		{
			return srand(seed);
		}

		static inline int IntRandom()
		{
			return rand();
		}

		// 产生[beg, end]的int随机数
		static inline int IntRandom(int beg, int end)
		{
			return beg + rand() % (abs(end - beg) + 1);
		}

		static unsigned int UIntRandom(unsigned int beg, unsigned int end);

		// 产生[beg, end]的double随机数，ratio为精度
		static double DoubleRandom(double beg, double end, unsigned int ratio = 1000);

		// 产生[0,1]随机小数
		static inline double Random0_1()
		{
			DoubleRandom(0.0, 1.0);
		}

	private:
		CRandom() { SetSeed(); }
		static CRandom global_random;
	};
};

#endif

