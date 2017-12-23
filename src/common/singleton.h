#ifndef CTM_COMMON_SINGLETON_H__
#define CTM_COMMON_SINGLETON_H__

#include "macro.h"
#include <stdio.h>

namespace ctm
{

	template <class T>  
	class Singleton  
	{

		NOCOPY(Singleton)
	
	public:  
	    static T* GetInstance()  
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
