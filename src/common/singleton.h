#ifndef CTM_COMMON_SINGLETON_H__
#define CTM_COMMON_SINGLETON_H__
#include <stdlib.h>
namespace ctm
{
	template <class T>  
	class CSingleton  
	{
	public:  
	    static T* GetInstance()  
	    {  
	        if (m_pInstance == NULL) {
	            m_pInstance = new T(); 
			}
	        return m_pInstance;  
	    } 
		
	protected:  
	    CSingleton(){}  
	    ~CSingleton(){}  
		
	private:  
	    static T* m_pInstance;  
		
	    class Garbo   
	    {  
	    public:  
	        ~Garbo()  
	        {  
	            if (CSingleton::m_pInstance)
	            {  
	                delete CSingleton::m_pInstance;  
	                CSingleton::m_pInstance = NULL;  
	            }  
	        }  
	    };
		
	    static Garbo garbo; 
	};  
	  
	template <class T>  
	T* CSingleton<T>::m_pInstance = NULL;
}

#endif
