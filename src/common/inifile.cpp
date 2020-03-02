#include "inifile.h"
#include "string_tools.h"
#include <fstream>
#include <assert.h>

namespace ctm
{
	CIniValue::~CIniValue()
	{
		for (int i = 0; i < m_vecChild.size(); ++i)
		{
			delete m_vecChild[i];
		}
	}

	CIniValue& CIniValue::operator = (int val)
	{
		m_type = EIntType;
		m_value = I2S(val);
		return *this;
	}

	CIniValue& CIniValue::operator = (double val)
	{
		m_type = EDoubleType;
		m_value = D2S(val);
		return *this;
	}

	CIniValue& CIniValue::operator = (const string& val)
	{
		m_type = EStringType;
		m_value = val;
		return *this;
	}

	string CIniValue::ToString() const
	{
		string centont;

		switch (m_type)
		{
		case EIntType:
		case EDoubleType:
		case EStringType:
			centont = m_key + "=" + m_value;
			break;
		case ECommentType:
			centont = "#" + m_value;
			break;
		case ESectionType:
			centont = "[" + m_key + "]";
			break;
		case EOtherType:
			centont = m_value;
			break;
		default:
			break;
		}

		for (int i = 0; i < m_vecChild.size(); ++i)
		{
			centont += "\n" + m_vecChild[i]->ToString() + "\n";
		}

		return centont;
	}

	CIniValue& CIniValue::operator[] (const string& key)
	{
		assert(m_type == ESectionType);

		if (m_mapChild.find(key) == m_mapChild.end())
		{
			CIniValue* pValue = new CIniValue;
			pValue->m_key = key;
			m_vecChild.push_back(pValue);
			m_mapChild[pValue->m_key] = pValue;
		}

		return *m_mapChild[key];
	}

	void CIniValue::AddChild(CIniValue* pChild)
	{
		if (pChild)
		{
			m_vecChild.push_back(pChild);
			if (pChild->m_type == EDoubleType || pChild->m_type == EStringType || pChild->m_type == EIntType)
				m_mapChild[pChild->m_key] = pChild;
		}
	}

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
		char* pHead = NULL;
		CIniValue* pParent = NULL;
		CIniValue* pValue = NULL;
		while(!fileIni.eof())
		{
			fileIni.getline(buf, sizeof(buf) - 1);
			pHead = buf;
			while(isspace(*pHead)) ++pHead;
			
			if (*pHead == '\0') continue;

			if (*pHead == '#') // ½âÎö×¢ÊÍ
			{
				pValue = new CIniValue(CIniValue::ECommentType, "", buf, pParent);
				if (pParent) pParent->AddChild(pValue);
				else m_vecIniValue.push_back(pValue);
			}
			else if (*pHead == '[') // ½âÎö¶Î
			{
				strBuf = pHead;
				int pos = strBuf.find("]", 2);
				assert(pos != string::npos);
				key = strBuf.substr(1, pos);
				key = Trimmed(key);
				pParent = new CIniValue(CIniValue::ESectionType, key, "", NULL);
				m_vecIniValue.push_back(pParent);
				m_keyMap[key] = pParent;
			}
			else // ½âÎökey=value
			{
				strBuf = pHead;
				int pos = strBuf.find("=");
				assert(pos != string::npos);

				key = strBuf.substr(0, pos);
				value = strBuf.substr(pos + 1);

				key = Trimmed(key);
				value = Trimmed(value);

				pValue = new CIniValue(key, value, pParent);
				if (pParent) {
					pParent->AddChild(pValue);
				}
				else {
					m_vecIniValue.push_back(pValue);
					m_keyMap[key] = pValue;	
				}
			}
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

	string CIniFile::ToString() const
	{
		string centont;
		if (m_vecIniValue.size())
		{
			int i = 0;
			for (; i < m_vecIniValue.size() - 1; ++i)
			{
				centont += m_vecIniValue[i]->ToString() + "\n";
			}
			centont += m_vecIniValue[i]->ToString();
		}

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
