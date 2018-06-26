#ifndef CTM_COMMON_INIFILE_H__
#define CTM_COMMON_INIFILE_H__
#include <string>
#include <map>
#include <vector>
#include "com.h"

namespace ctm
{
	using namespace std;

	class CIniValue
	{
	public:
		CIniValue() {};
		CIniValue(int val) : m_value(I2S(val)) {}
		CIniValue(double val) : m_value(D2S(val))  {}
		CIniValue(const string & val) : m_value(val){}
		CIniValue(const CIniValue & other) : m_value(other.m_value){}

		~CIniValue(){}
		
		CIniValue & operator = (int val)
		{
			m_value = I2S(val);
			return *this;
		}

		CIniValue & operator = (double val)
		{
			m_value = D2S(val);
			return *this;
		}

		CIniValue & operator = (const string & val)
		{
			m_value = val;
			return *this;
		}
		
		int AsInt() const
		{
			return S2I(m_value);
		}
		
		double AsFloat() const
		{
			return S2D(m_value);
		}
		
		string AsString() const
		{ 
			return m_value; 
		}

		string ToString() const
		{
			if (m_key.empty())
				return m_value;
			else
				return m_key + "=" + m_value;
		}
		
	public:
		string m_value;
		string m_key;
	};
	
	class CIniFile
	{
	public:
		CIniFile(const string & fileName) :
			m_fileName(fileName)
		{
		}
		
		~CIniFile()
		{
			Clear();
		}

		bool Load()
		{
			return Load(m_fileName);
		}

		bool Load(const string & fileName);

		bool Save()
		{
			return Save(m_fileName);
		}

		bool Save(const string & fileName);

		void Clear();

		string ToString();

		CIniValue & operator[] (const string & key);
		
	private:
		string m_fileName;
		map<string, CIniValue*> m_keyMap;
		vector<CIniValue*> m_vecIniValue;
	};
}

#endif

