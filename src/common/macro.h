#ifndef _h_ctm_common_object_h
#define _h_ctm_common_object_h

namespace ctm
{


#define NOCOPY(T) \
protected:\
	T(const T&){}\
	T& operator= (const T&){ return *this; }

}

#endif