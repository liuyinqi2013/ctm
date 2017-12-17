#ifndef _h_ctm_common_singleton_h
#define _h_ctm_common_singleton_h

#include "macro.h"
#include <stdio.h>

namespace ctm
{

	template <class T>  
	class Singleton  
	{

	NOCOPY(Singleton)
	
	public:  
	    static T* getInstance()  
	    {  
	        if (m_pInstance == NULL) {
	            m_pInstance = new T(); 
			}
	        return m_pInstance;  
	    } 
		
	protected:  
	    Singleton(){}  
	    ~Singleton(){}  
		
	private:  
	    static T* m_pInstance;  
		
	    class Garbo   
	    {  
	    public:  
	        ~Garbo()  
	        {  
	            if (Singleton::m_pInstance)
	            {  
	                delete Singleton::m_pInstance;  
	                Singleton::m_pInstance = NULL;  
	            }  
	        }  
	    };
		
	    static Garbo garbo; 
	};  
	  
	template <class T>  
	T* Singleton<T>::m_pInstance = NULL;

}

#endif
