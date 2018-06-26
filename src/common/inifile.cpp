#include "inifile.h"
#include "string_tools.h"
#include <fstream>

namespace ctm
{
	bool CIniFile::Load(const string & fileName)
	{
		if (fileName.empty())
		{
			return false;
		}

		Clear();
		
		fstream fileIni("conf.ini",  std::fstream::in | std::fstream::out);

		if (!fileIni.is_open())
		{
			return false;
		}

		char buf[1024] = {0};
		string strBuf;
		string key;
		string value;
		char * pHead = NULL;
		CIniValue * pValue = NULL;
		while(!fileIni.eof())
		{
			fileIni.getline(buf, sizeof(buf) - 1);
			pHead = buf;
			while(isspace(*pHead)) ++pHead;
			if (*pHead == '#' || *pHead == '\0')
			{
				m_vecIniValue.push_back(new CIniValue(buf));
				continue;
			}
			
			strBuf = pHead;
			int pos = strBuf.find("=");
			if (pos == string::npos)
			{
				m_vecIniValue.push_back(new CIniValue(buf));
				continue;
			}
			
			key = strBuf.substr(0, pos);
			value = strBuf.substr(pos + 1);
			key = Trimmed(key);
			value = Trimmed(value);
			pValue = new CIniValue(value);	
			pValue->m_key = key;
			m_vecIniValue.push_back(pValue);
			m_keyMap[pValue->m_key] = pValue;
		}

		return true;
	}


	bool CIniFile::Save(const string & fileName)
	{
		if (fileName.empty())
		{
			return false;
		}

		ofstream out(fileName.c_str());
		out << ToString();
	}
	
	void CIniFile::Clear()
	{
		for (int i = 0; i < m_vecIniValue.size(); ++i)
		{
			delete m_vecIniValue[i];
		}

		m_vecIniValue.clear();
		m_keyMap.clear();
	}

	string CIniFile::ToString()
	{
		string centont;
		int i = 0;
		for (;i < m_vecIniValue.size() - 1; ++i)
		{
			centont += m_vecIniValue[i]->ToString() + "\n";
		}
		centont += m_vecIniValue[i]->ToString();
		return centont;
	}

	CIniValue & CIniFile::operator[] (const string & key)
	{
		if (m_keyMap.find(key) == m_keyMap.end())
		{
			CIniValue * pValue = new CIniValue;
			pValue->m_key = key;
			m_vecIniValue.push_back(pValue);
			m_keyMap[pValue->m_key] = pValue;
		}

		return *m_keyMap[key];
	}
}
