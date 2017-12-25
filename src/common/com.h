#ifndef CTM_COMMON_COM_H__
#define CTM_COMMON_COM_H__
#include <stdlib.h>
#include <sstream>
#include <string>

namespace ctm
{
	template <class T>
	std::string AnyToString(const T& val)
	{
		std::stringstream ss;
		ss<<val;
		return ss.str();
	}

	inline int Str2Int(const std::string& val)
	{
		return atoi(val.c_str());
	}

	inline long Str2Long(const std::string& val)
	{
		return atol(val.c_str());
	}

	inline long long Str2Longlong(const std::string& val)
	{
		return atoll(val.c_str());
	}
	
	inline double Str2Double(const std::string& val)
	{
		return atof(val.c_str());
	}
	
	inline std::string Int2Str(const int& val)
	{
		return AnyToString<int>(val);
	}

	inline std::string Long2Str(const long& val)
	{
		return AnyToString<long>(val);
	}

	inline std::string Longlong2Str(const long long& val)
	{
		return AnyToString<long long>(val);
	}

	inline std::string Double2Str(const double& val)
	{
		return AnyToString<double>(val);
	}

	#define S2I(x) Str2Int((x))
	#define I2S(x) Int2Str((x))
	#define S2L(x) Str2Long((x))
	#define L2S(x) Long2Str((x))
	#define S2LL(x) Str2Longlong((x))
	#define LL2S(x) Longlong2Str((x))
	#define S2D(x) Str2Double((x))
	#define D2S(x) Double2Str((x))	
};

#endif