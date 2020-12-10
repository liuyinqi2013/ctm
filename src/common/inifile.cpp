#include "inifile.h"
#include "string_tools.h"
#include <fstream>
#include <assert.h>
#include <iostream>

namespace ctm
{
	CIniValue::~CIniValue()
	{
		for (size_t i = 0; i < m_vecChild.size(); ++i)
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

		for (size_t i = 0; i < m_vecChild.size(); ++i)
		{
			centont += "\n" + m_vecChild[i]->ToString();
		}

		return centont;
	}

	CIniValue& CIniValue::operator[] (const string& key)
	{
		assert(m_type == ESectionType);

		if (m_mapChild.find(key) == m_mapChild.end())
		{
			AddChild(new CIniValue(key, "", this));
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

		m_fileName = fileName;
		
		fstream fileIni(fileName,  std::fstream::in | std::fstream::out);

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

			if (*pHead == '#') // 解析注释
			{
				pValue = new CIniValue(CIniValue::ECommentType, "", ++pHead, pParent);
				if (pParent) {
					pParent->AddChild(pValue);
				}
				else {
					m_vecIniValue.push_back(pValue);
				}
			}
			else if (*pHead == '[') // 解析局域
			{
				strBuf = pHead;
				size_t pos = strBuf.find("]", 2);
				assert(pos != string::npos);
				key = strBuf.substr(1, pos - 1);
				key = Trimmed(key);
				pParent = new CIniValue(CIniValue::ESectionType, key, "", NULL);
				m_vecIniValue.push_back(pParent);
				m_keyMap[key] = pParent;
			}
			else // 解析key=value
			{
				strBuf = pHead;
				size_t pos = strBuf.find("=");
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
		return true;
	}
	
	void CIniFile::Clear()
	{
		for (size_t i = 0; i < m_vecIniValue.size(); ++i)
		{
			delete m_vecIniValue[i];
		}
		m_vecIniValue.clear();
		m_keyMap.clear();
	}

	string CIniFile::ToString() const
	{
		string global;
		string centont;
		if (m_vecIniValue.size())
		{
			for (size_t i = 0; i < m_vecIniValue.size(); ++i)
			{	
				if (m_vecIniValue[i]->m_parent == NULL && m_vecIniValue[i]->m_type != CIniValue::ESectionType) {
					
					global += m_vecIniValue[i]->ToString() + "\n";
				}
				else {
					centont += m_vecIniValue[i]->ToString() + "\n";
				}
			}
		}

		return global + centont;
	}

	CIniValue& CIniFile::operator[] (const string & key)
	{
		if (m_keyMap.find(key) == m_keyMap.end())
		{
			CIniValue* pValue = new CIniValue(key, "");
			m_vecIniValue.push_back(pValue);
			m_keyMap[pValue->m_key] = pValue;
		}

		return *m_keyMap[key];
	}

	void CIniFile::Get(const string& key, int& val)
	{
		const CIniValue* p = GetIniValue(key);
		if (p) val = p->AsInt();
	}

	void CIniFile::Get(const string& key, double& val)
	{
		const CIniValue* p = GetIniValue(key);
		if (p) val = p->AsFloat();
	}

	void CIniFile::Get(const string& key, string& val)
	{
		const CIniValue* p = GetIniValue(key);
		if (p) val = p->AsString();
	}

	void CIniFile::Get(const string& section, const string& key, int& val)
	{
		const CIniValue* p = GetIniValue(section, key);
		if (p) val = p->AsInt();
	}

	void CIniFile::Get(const string& section, const string& key, double& val)
	{
		const CIniValue* p = GetIniValue(section, key);
		if (p) val = p->AsFloat();
	}

	void CIniFile::Get(const string& section, const string& key, string& val)
	{
		const CIniValue* p = GetIniValue(section, key);
		if (p) val = p->AsString();
	}

	CIniValue* CIniFile::GetIniValue(const string& key)
	{
		map<string, CIniValue*>::iterator it = m_keyMap.find(key);
		if (it != m_keyMap.end())
		{
			return it->second;
		}

		return NULL;
	}

	CIniValue* CIniFile::GetIniValue(const string& section, const string& key)
	{
		CIniValue* p = GetIniValue(section);
		if (p && p->m_type == CIniValue::ESectionType)
		{
			map<string, CIniValue*>::iterator it = p->m_mapChild.find(key);
			if (it != p->m_mapChild.end()) {
				return it->second;
			}
		}

		return NULL;
	}
}
