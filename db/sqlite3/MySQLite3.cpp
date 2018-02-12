#include "MySQLite3.h"
#include "StringFunc.h"
#include "TinyXML/tinyxml.h"
#include "TinyXML/XmlCom.h"
#include "MutexLock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#ifndef STL_STRING_2_CSTR
#define STL_STRING_2_CSTR
#define C2S(ptr) ((ptr)?string((ptr)):string(""))
#define S2C(str) ((str).c_str())
#endif

std::string GetFileName(const std::string& filePathName)
{
    std::string fileName;
    int pos = filePathName.find_last_of("/\\");
    if(pos != filePathName.npos && pos + 1 != filePathName.size() - 1)
    {
        fileName = filePathName.substr(pos + 1);
    }

    return fileName;
}

bool CMySQLite3::Open()
{
	SYNC::CMutexLock lock(GetFileName(m_DbName));
    SYNC::CMutexLock::Owner own(lock);
	Close();
	return SQLITE_OK == sqlite3_open(m_DbName.c_str(), &m_DbHandle) ? (m_bIsOpen = true) : false;
}

int CMySQLite3::Exectue(const char* strSQL)
{

	if(!m_bIsOpen)
	{
		return -1;
	}

	SYNC::CMutexLock lock(GetFileName(m_DbName));
    SYNC::CMutexLock::Owner own(lock);

	char *errMsg = NULL;
	int iRet = sqlite3_exec(m_DbHandle, strSQL, NULL, NULL, &errMsg);
	if(SQLITE_OK != iRet)
	{
		m_strErr = errMsg;
		return -1;
	}

	return 0;
}

int CMySQLite3::Query(const char* strSQL, char*** result, int* nRow, int* column)
{

	if(!m_bIsOpen)
	{
		return -1;
	}

	SYNC::CMutexLock lock(GetFileName(m_DbName));
    SYNC::CMutexLock::Owner own(lock);

	char *errMsg = NULL;
	int iRet = sqlite3_get_table(m_DbHandle, strSQL, result, nRow, column, &errMsg);
	if(SQLITE_OK != iRet)
	{
		m_strErr = errMsg;
		return -1;
	}

	return 0;
}

int CMySQLite3::Query(const char* strSQL, CDbResult& outRes)
{
	if(!m_bIsOpen)
	{
		return -1;
	}

	SYNC::CMutexLock lock(GetFileName(m_DbName));
    SYNC::CMutexLock::Owner own(lock);

	char *errMsg = NULL;
	char **result;
	int nRow(0);
	int column(0);

	int iRet = sqlite3_get_table(m_DbHandle, strSQL, &result, &nRow, &column, &errMsg);
	if(SQLITE_OK != iRet)
	{
		m_strErr = errMsg;
		return -1;
	}
	

	int index = column;

	for(int i = 0; i < nRow; ++i)
	{
		CDbRow *pRow = new CDbRow;
		for(int j = 0; j < column; ++j)
		{
			string *pValue(NULL);

			if(NULL == result[index])
			{
				pValue = new string("");
			}
			else
			{
				pValue = new string(result[index]);
			}

			pRow->Insert(string(result[j]), pValue);
			index++;
		}
		outRes.Insert(pRow);
	}

	sqlite3_free_table(result);

	return 0;
}

int CMySQLite3::Query(const char* strSQL, CDbResult& outRes, vector<ColumnHeader>& vecColHeader)
{
	if(!m_bIsOpen)
	{
		return -1;
	}
	SYNC::CMutexLock lock(GetFileName(m_DbName));
    SYNC::CMutexLock::Owner own(lock);
	sqlite3_stmt* stmt = NULL;
	int iRet = sqlite3_prepare(m_DbHandle, strSQL, strlen(strSQL), &stmt, NULL);
	if(SQLITE_OK != iRet)
	{
		m_strErr = sqlite3_errmsg(m_DbHandle);
		return -1;
	}

	ColumnHeader header;
	int colNum = sqlite3_column_count(stmt);
	for(int i = 0; i < colNum; i++)
	{
		header.strName = sqlite3_column_name(stmt, i);
		vector<string> vec;
		const char* type = sqlite3_column_decltype(stmt, i);
	    CutString(C2S(type), vec);
		if(vec.size()) 
		{
			header.strType = vec[0];
		}
		header.bIsStr = IsStrType(header.strType);
		vecColHeader.push_back(header);
	}

	while(SQLITE_DONE != sqlite3_step(stmt))
	{
		CDbRow *pRow = new CDbRow;
		for(int i = 0; i < colNum; i++)
		{
			header.strName = sqlite3_column_name(stmt, i);
			string *pValue(NULL);
			const char* p = (const char* )sqlite3_column_text(stmt, i);
			if(NULL == p)
			{
				pValue = new string("");
			}
			else
			{
				pValue = new string(p);
			}

			pRow->Insert(sqlite3_column_name(stmt, i), pValue);
		}
		outRes.Insert(pRow);
	}

	sqlite3_finalize(stmt);
	return 0;
}

int CMySQLite3::Export(const string& strTableName, string& strOutSQL, int rowOffset, int rowCount)
{
	if(!m_bIsOpen)
	{
		return -1;
	}

	char SQL[128] = {0};
	sprintf(SQL, "SELECT * FROM %s LIMIT %d OFFSET %d", strTableName.c_str(), rowCount, rowOffset);

	CDbResult outRes;
	vector<ColumnHeader> vecColHeader;
	if(-1 == Query(SQL, outRes, vecColHeader))
	{
		return -1;
	}

	string strSQL;
	string nameList;
	string valueList;
	for(int i = 0; i < outRes.Size(); ++i)
	{
		nameList.clear();
		valueList.clear();
		for(int k = 0; k < vecColHeader.size(); ++k)
		{
			if(outRes[i][vecColHeader[k].strName].empty())
			{
				continue;
			}

			if(k)
			{
				nameList += ",";
				valueList += ",";
			}

			nameList += vecColHeader[k].strName;
			if(vecColHeader[k].bIsStr)
			{
				valueList += "'" + outRes[i][vecColHeader[k].strName] + "'";
			}
			else
			{
				valueList += outRes[i][vecColHeader[k].strName];
			}
		}

		strSQL = "INSERT INTO " + strTableName + "(" + nameList + ")" + " VALUES(" + valueList + ");";
		strOutSQL += strSQL + "\n";
	}
	return 0;
}

int CMySQLite3::ExportToXml(const string& strTableName, string& strOutXml, int rowOffset, int rowCount)
{
	if(!m_bIsOpen)
	{
		return -1;
	}

	char SQL[128] = {0};
	sprintf(SQL, "SELECT * FROM %s LIMIT %d OFFSET %d", strTableName.c_str(), rowCount, rowOffset);

	CDbResult outRes;
	vector<ColumnHeader> vecColHeader;
	if(-1 == Query(SQL, outRes, vecColHeader))
	{
		return -1;
	}

	TiXmlElement* pTableElem = new TiXmlElement("table");
	CXmlCom::SetAttribtue(pTableElem, "name", strTableName);

	for(int i = 0; i < outRes.Size(); ++i)
	{
		TiXmlElement* pRowElem = new TiXmlElement("row");
		for(int k = 0; k < vecColHeader.size(); ++k)
		{
			TiXmlElement* pColumnElem = new TiXmlElement("column");
			CXmlCom::SetAttribtue(pColumnElem, "name", vecColHeader[k].strName);
			CXmlCom::SetAttribtue(pColumnElem, "type", vecColHeader[k].strType);
			CXmlCom::SetAttribtue(pColumnElem, "isstr", vecColHeader[k].bIsStr ? 1 : 0);
			CXmlCom::SetText(pColumnElem, outRes[i][vecColHeader[k].strName]);
			CXmlCom::AddChildNode(pRowElem, pColumnElem);
		}
		CXmlCom::AddChildNode(pTableElem, pRowElem);
	}

	strOutXml = CXmlCom::ToString(pTableElem);
	pTableElem->Clear();
	delete pTableElem;

	return 0;
}

int CMySQLite3::GetRowNum(const string& tableName)
{
	if(!m_bIsOpen)
	{
		return -1;
	}
	string SQL = "SELECT COUNT(*) FROM " + tableName;
	CDbResult outRes;
	if(-1 == Query(SQL.c_str(), outRes))
	{
		return -1;
	}

	int RowNum = 0;
	if(outRes.Size() > 0 && !outRes[0][0].empty())
	{
		RowNum = atoi(outRes[0][0].c_str());
	}

	return RowNum;
}

int CMySQLite3::ClearTable(const string& tableName)
{
	std::string SQL = "DELETE FROM " + tableName;
	return Exectue(SQL.c_str());
}

int CMySQLite3::GetCreateSQL(const string& tableName, string& strOutSQL)
{
	if(!m_bIsOpen)
	{
		return -1;
	}

	string SQL = "SELECT sql FROM sqlite_master WHERE type = 'table' AND name = '" + tableName + "'";
	CDbResult outRes;
	if(-1 == Query(SQL.c_str(), outRes))
	{
		return -1;
	}

	int ret = -1;
	if(outRes.Size() > 0 && !outRes[0]["sql"].empty())
	{
		strOutSQL = outRes[0]["sql"];
		ret = 0;
	}

	return ret;
}

 void CMySQLite3::Test()
 {
	  char* sql = "select * from tab_User";

	  CDbResult outRes; 
	  vector<ColumnHeader> vecColHeader;
	  Query(sql, outRes, vecColHeader);
	  for(int i = 0; i < vecColHeader.size(); ++i)
	  {
		  cout<<vecColHeader[i].strName<<" "
			  <<vecColHeader[i].strType<<" "
			  <<vecColHeader[i].bIsStr<<" ";
	  }
	  cout<<endl;

	  //cout<<GetRowNum("tab_User")<<endl;
	  //string strXml;
	  //ExportToXml("tab_User", strXml, 0, 1);
	  //cout<<strXml;

	  //string strOutSQL;
	  //cout<<GetCreateSQL("tab_User", strOutSQL)<<endl;
	  //cout<<strOutSQL;
	  //for(int k = 0; k < outRes.Size(); ++k)
	  //{
		 // for(int i = 0; i < vecColHeader.size(); ++i)
		 // {
			//  cout<<outRes[k][vecColHeader[i].strName]<<" ";
		 // }
		 // cout<<endl;
	  //}
 }


 bool CMySQLite3::IsStrType(const string& typeName)
 {
	 if("INT" == typeName ||
		"INTEGER" == typeName ||
		"FLOAT" == typeName ||
		"NUMERIC" == typeName ||
		"REAL" == typeName)
	 {
		 return false;
	 }

	 return true;
 }

bool CMySQLite3::IsExist(const string& strTabName)
{
    std::string SQL = "SELECT COUNT(*) FROM sqlite_master WHERE  type = 'table' AND name = '" + strTabName +"'";
    bool bRet = false;
    CDbResult outRes;
    if(0 == Query(SQL.c_str(), outRes) &&
            outRes.Size() > 0 &&
            atoi(outRes[0][0].c_str()) >= 1)
    {
        bRet = true;
    }
    return bRet;
}