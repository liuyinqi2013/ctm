#ifndef CTM_COMMON_INIFILE_H__
#define CTM_COMMON_INIFILE_H__
#include <string>
#include <map>
#include <vector>
#include "com.h"

namespace ctm
{
	using namespace std;
	class CIniFile;
	class CIniValue;

	class CIniValue
	{
	public:

		~CIniValue();
		
		CIniValue& operator = (int val);
		CIniValue& operator = (double val);
		CIniValue& operator = (const string& val);

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

		string ToString() const;

		CIniValue& operator[] (const string& key);

	private:
		enum EIniValueType {
			ENodeType = 0,
			EIntType = 1,
			EDoubleType = 2,
			EStringType = 3,
			ECommentType = 4,
			ESectionType = 5,
			EOtherType = 6,
		};

		CIniValue(CIniValue* parent = NULL) : m_type(ENodeType), m_parent(parent) {};
		CIniValue(int val, CIniValue* parent = NULL) : m_type(EIntType), m_value(I2S(val)), m_parent(NULL) {}
		CIniValue(double val, CIniValue* parent = NULL) : m_type(EDoubleType), m_value(D2S(val)), m_parent(NULL) {}
		CIniValue(const string& key, const string& val, CIniValue* parent = NULL) : m_type(EStringType), m_key(key), m_value(val), m_parent(NULL) {}
		CIniValue(int type, const string& key, const string& val, CIniValue* parent = NULL) : m_type(type), m_key(key), m_value(val), m_parent(NULL) {}
		CIniValue(const CIniValue& other);
		
		void AddChild(CIniValue* pChild);

	private:
		int m_type;
		string m_value;
		string m_key;
		CIniValue* m_parent;
		map<string, CIniValue*> m_mapChild;
		vector<CIniValue*> m_vecChild;

		friend class CIniFile;
	};
	
	class CIniFile
	{
	public:
		CIniFile(const string& fileName) :
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

		bool Load(const string& fileName);

		bool Save()
		{
			return Save(m_fileName);
		}

		bool Save(const string& fileName);

		void Clear();

		string ToString() const;

		CIniValue& operator[] (const string& key);
		
	private:
		string m_fileName;
		map<string, CIniValue*> m_keyMap;
		vector<CIniValue*> m_vecIniValue;
	};
}

#endif

