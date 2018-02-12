#include "MySQLDB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MS2C(str) ((str).c_str())
#define MC2S(ptr) ((!(ptr))?string(""):string((ptr)))

bool CMySQLDB::Connect()
{
    DisConnect();

	m_DBHandle = mysql_init(NULL);
	if (!m_DBHandle)
	{
		return false;
	}

	char value = 1;
	if (mysql_options(m_DBHandle, MYSQL_OPT_RECONNECT, (char *)&value))
	{
		return false;
	}

	mysql_options(m_DBHandle, MYSQL_SET_CHARSET_NAME, "utf8");
	if (!mysql_real_connect(m_DBHandle, m_Host.c_str(), m_UserName.c_str(), m_PassWord.c_str(), m_DBName.c_str(), m_Port, NULL, 0))
	{
		return false;
	}

	mysql_query(m_DBHandle, "SET NAMES UTF8");
    m_IsConnect = true;
	m_DBStmt =  mysql_stmt_init(m_DBHandle);
	return true;
}


bool CMySQLDB::Query(const char* sql)
{
	if(mysql_query(m_DBHandle, sql) != 0)
	{
		return false;
	}

	FreeResult();
	
	m_DBRes =  mysql_store_result(m_DBHandle);

	return true;
}

int CMySQLDB::StmtPrepare(const char* sql)
{
	int iRet = 0;
	iRet = mysql_stmt_prepare(m_DBStmt, sql, strlen(sql));
	return iRet;
}

int CMySQLDB::StmtBindParamInt(int iparamNum, int iVal)
{
	MYSQL_BIND buf = {0};
	memset(&buf, 0, sizeof(buf));
	buf.buffer = (void*)&iVal;
	buf.buffer_length = sizeof(int);
	buf.buffer_type = MYSQL_TYPE_LONG;
	buf.is_null = 0;
	int iRet = 0;
	iRet = mysql_stmt_bind_param(m_DBStmt, &buf);
	return iRet;
}

int CMySQLDB::StmtBindParamStr(int iparamNum, const string& strVal)
{
	MYSQL_BIND buf = {0};
	memset(&buf, 0, sizeof(buf));
	buf.buffer = (void*)strVal.c_str();
	buf.buffer_length = strVal.length();
	buf.buffer_type = MYSQL_TYPE_STRING;
	buf.is_null = 0;
	int iRet = 0;
	iRet = mysql_stmt_bind_param(m_DBStmt, &buf);
	return iRet;
}

int CMySQLDB::StmtBind(MYSQL_BIND* bind)
{
	int iRet = 0;
	iRet = mysql_stmt_bind_param(m_DBStmt, bind);
	return iRet;
}

int CMySQLDB::StmtExecute()
{
	int iRet = 0;
	iRet = mysql_stmt_execute(m_DBStmt);
	return iRet;
}

string CMySQLDB::StmtSQLStatus()
{
	return string(mysql_stmt_sqlstate(m_DBStmt));
}

int CMySQLDB::GetTabRowCount(const string& tableName)
{
    string SQL = "SELECT COUNT(*) FROM " + tableName;
    return GetRowCount(SQL);
}

int CMySQLDB::GetRowCount(const string& SQL)
{
    if(!Query(SQL.c_str()))
    {
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(m_DBRes);
    int ret = 0;
    if(row && row[0])
    {
        ret = atoi(row[0]);
    }
    return ret;
}

int CMySQLDB::ClearTable(const string& tableName)
{
	std::string SQL = "DELETE FROM " + tableName;
	return (Excute(SQL.c_str()) ? 0 : -1);
}


int CMySQLDB::Export(const string& tableName, string& strOutSQL, int rowOffset, int rowCount)
{
	char SQL[128] = {0};
	if(rowCount > 0)
	{
		sprintf(SQL, "SELECT * FROM %s LIMIT %d,%d", tableName.c_str(), rowOffset, rowCount);
	}
	else
	{
		sprintf(SQL, "SELECT * FROM %s LIMIT %d,%d", tableName.c_str(), rowOffset, GetTabRowCount(tableName) - rowOffset);
	}

	if(!Query(SQL))
	{
		return -1;
	}

	vector<string> vecSQL;
	ConvertInsertSQL(m_DBRes, vecSQL);
	for(int i = 0; i < vecSQL.size(); ++i)
	{
		strOutSQL += vecSQL[i] + "\n";
	}

	return 0;
}

bool  CMySQLDB::Excute(const char* sql)
{
	int res = mysql_query(m_DBHandle, sql);	
	if(res != 0)
	{
		return false;
	}

	return true;
}

string CMySQLDB::GetLastErr()
{
	char buf[1024] = {0};
#ifndef WIN32
    snprintf(buf, sizeof(buf) - 1, "Error code : %d msg : %s\n", mysql_errno(m_DBHandle), mysql_error(m_DBHandle));
#else
	sprintf(buf, "Error code : %d msg : %s\n", mysql_errno(m_DBHandle), mysql_error(m_DBHandle));
#endif
	return buf;
}

MYSQL* CMySQLDB::GetMySQLHandle() const 
{
	return m_DBHandle;
}

bool CMySQLDB::IsStringType(int columnType)
{
	bool b = true;
	switch (columnType)
	{
	case MYSQL_TYPE_SHORT:
	case MYSQL_TYPE_LONG:
	case MYSQL_TYPE_FLOAT:
	case MYSQL_TYPE_INT24:
	case MYSQL_TYPE_NULL:
	case MYSQL_TYPE_LONGLONG:
	case MYSQL_TYPE_BIT:
	case MYSQL_TYPE_ENUM:
	case MYSQL_TYPE_SET:
		b = false;
		break;
	default:
		break;
	}

	return b;
}

void CMySQLDB::ConvertInsertSQL(MYSQL_RES* result, std::vector<std::string>& outSQLVec)
{
	if(!result) return;
	int rowNum = mysql_num_rows(result);
	int colNum = mysql_num_fields(result);

	if( 0 == rowNum || 0 == colNum)
	{
		return ;
	}

	MYSQL_ROW row(NULL);
	MYSQL_FIELD* fileds = mysql_fetch_field(result);
	if(!fileds || !fileds[0].table) 
	{
		return;
	}

	std::string tableName = fileds[0].table;
	std::string colNameList;
	std::string valueList;

	std::string SQL;
	for(int i = 0; i < rowNum; ++i)
	{
		colNameList.clear();
		valueList.clear();

		row = mysql_fetch_row(result);
		if(!row) continue;

		for(int k = 0; k < colNum; ++k)
		{
			if(!row[k]) continue;

			if(k)
			{
				colNameList += ",";
				valueList += ",";
			}

			colNameList += fileds[k].org_name;
			valueList += IsStringType(fileds[k].type) ? AddFlag(row[k]) : MC2S(row[k]);
		}

		SQL = "INSERT INTO " + tableName + "(" + colNameList + ") VALUES(" + valueList + ");";
		outSQLVec.push_back(SQL);
	}
}

void CMySQLDB::ConvertUpdateSQL(MYSQL_RES* result, std::vector<std::string>& outSQLVec)
{
	if(!result) return;
	int rowNum = mysql_num_rows(result);
	int colNum = mysql_num_fields(result);

	if( 0 == rowNum || colNum < 2)
	{
		return ;
	}

	MYSQL_ROW row(NULL);
	MYSQL_FIELD* fileds = mysql_fetch_field(result);
	if(!fileds || !fileds[0].table) 
	{
		return;
	}

	std::string tableName = fileds[0].table;
	std::string colNameList;
	std::string strWhere;

	std::string SQL;
	for(int i = 0; i < rowNum; ++i)
	{
		colNameList.clear();
		row = mysql_fetch_row(result);
		if(!row) continue;

		for(int k = 1; k < colNum; ++k)
		{
			if(!row[k]) continue;

			if(k != 1)
			{
				colNameList += ",";
			}

			colNameList += std::string(fileds[k].org_name) + " = " + (IsStringType(fileds[k].type) ? AddFlag(row[k]) : MC2S(row[k]));
		}
		strWhere = std::string(fileds[0].org_name) + " = " + (IsStringType(fileds[0].type) ? AddFlag(row[0]) : MC2S(row[0]));
		SQL = "UPDATE " + tableName + " SET " + colNameList + " WHERE " + strWhere + ";";
		outSQLVec.push_back(SQL);
	}
}


void CMySQLDB::GetFields(std::vector<std::string>& outFieldsVec)
{
	outFieldsVec.clear();
	
	if(!m_DBRes) return;
	int colNum = mysql_num_fields(m_DBRes);

	if(0 == colNum)
	{
		return ;
	}
	
	MYSQL_FIELD* fileds = mysql_fetch_fields(m_DBRes);
	if(!fileds) 
	{
		return;
	}

	for(int i = 0; i < colNum; ++i)
	{
		outFieldsVec.push_back(MC2S(fileds[i].name));
	}
}

void CMySQLDB::GetValues(std::vector<std::map<std::string, std::string> >& outVaulesVec)
{
	outVaulesVec.clear();

	if(!m_DBRes) return;
	int rowNum = mysql_num_rows(m_DBRes);
	int colNum = mysql_num_fields(m_DBRes);

	if( 0 == rowNum || 0 == colNum)
	{
		return ;
	}

	MYSQL_ROW row(NULL);
	MYSQL_FIELD* fileds = mysql_fetch_fields(m_DBRes);
	
	std::map<std::string, std::string> keyMap;
	for(int i = 0; i < rowNum; ++i)
	{
		row = mysql_fetch_row(m_DBRes);
		if(!row) continue;
		keyMap.clear();
		for(int k = 0; k < colNum; ++k)
		{
			keyMap[MC2S(fileds[k].name)] = MC2S(row[k]);
		}
		
		outVaulesVec.push_back(keyMap);
	}

	mysql_data_seek(m_DBRes, 0);
}

string CMySQLDB::EscapeString(const string& strIn)
{
	int len = strIn.size() * 2;
	char *buf = new char[len];
	if (!buf) return string("");
	
	memset(buf, 0, len);
	len = mysql_escape_string(buf, strIn.data(), strIn.size());
	if(len < 0)
	{
		delete[] buf;
		return string("");
	}
	string strRet(buf, len);
	delete[] buf;
	return strRet;
}

bool CMySQLDB::SelectDB(const string& db)
{
	return !mysql_select_db(m_DBHandle, MS2C(db));
}

string CMySQLDB::AddFlag(const char* value)
{
	if(!value) return string("''");
	return (string("'") + value + string("'"));
}