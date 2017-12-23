#ifndef CTM_COMMON_OBJECT_H__
#define CTM_COMMON_OBJECT_H__

namespace ctm
{


#define NOCOPY(T) \
protected:\
	T(const T&){}\
	T& operator= (const T&){ return *this; }

}

#endif