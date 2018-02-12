#ifndef __H__H_TCMOPERATORDB_H
#define __H__H_TCMOPERATORDB_H

#include <string>
#include <vector>
#include <map>

#include "mysql.h"

using namespace std;

typedef std::vector<std::map<std::string, std::string> > CStrMapArr;

class CMySQLDB
{
public:

	CMySQLDB() :
		m_Host(""),
		m_Port(0),
		m_DBName(""),
		m_UserName(""),
		m_PassWord(""),
        m_DBHandle(NULL),
		m_DBStmt(NULL),
		m_DBRes(NULL),
        m_IsConnect(false)
	{
	}

	CMySQLDB(
		const char* hostName, 
		const int port, 
		const char* DbName,
		const char* userName,
		const char* passWord):
		m_Host(hostName),
		m_Port(port),
		m_DBName(DbName),
		m_UserName(userName),
		m_PassWord(passWord),
        m_DBHandle(NULL),
		m_DBStmt(NULL),
		m_DBRes(NULL),
        m_IsConnect(false)

	{
	}

	~CMySQLDB()
	{
		FreeResult();
        DisConnect();
	}

	bool Connect();

	bool Connect(const char* hostName, 
		const int port, 
		const char* DbName,
		const char* userName,
		const char* passWord)
	{
		m_Host = hostName;
		m_Port = port;
		m_DBName = DbName;
		m_UserName = userName;
		m_PassWord = passWord;
		return Connect();
	}

    bool IsConnect() const
    {
        return m_IsConnect;
    }

	bool Query(const char* sql);

	bool Excute(const char* sql);

	bool Begin()
	{
		return Excute("BEGIN");
	}

	bool Commit()
	{
		return Excute("COMMIT");
	}

	bool Rollback()
	{
		return Excute("ROLLBACK");
	}
	
	int StmtPrepare(const char* sql);

	int StmtBindParamInt(int iparamNum, int iVal);

	int StmtBindParamStr(int iparamNum, const string& strVal);

	int StmtBind(MYSQL_BIND* bind);

	string StmtSQLStatus();

	int StmtExecute();

	int GetTabRowCount(const string& tableName);

	int ClearTable(const string& tableName);

	int Export(const string& tableName, string& strOutSQL, int rowOffset = 0, int rowCount = -1);

	void DisConnect()
	{
        if(m_IsConnect) {
            mysql_close(m_DBHandle);
            m_IsConnect = false;
        }
	}

	string GetLastErr();

	MYSQL* GetMySQLHandle() const ;

	void FreeResult()
	{
		if(m_DBRes) {
			mysql_free_result(m_DBRes);
			m_DBRes = NULL;
		}
	}
	
	void GetFields(std::vector<std::string>& outFieldsVec);

	void GetValues(std::vector<std::map<std::string, std::string> >& outVaulesVec);

	bool Ping()
	{
		return (!mysql_ping(m_DBHandle));
	}

	static string EscapeString(const string& strIn);

	bool SelectDB(const string& db);

	unsigned long InsertId() const
	{
		return mysql_insert_id(m_DBHandle);
	}

	unsigned long AffectedRows() const
	{
		return mysql_affected_rows(m_DBHandle);
	}

private:
	bool IsStringType(int columnType);

	string AddFlag(const char* value);

	void ConvertInsertSQL(MYSQL_RES* result, std::vector<std::string>& outSQLVec);

	void ConvertUpdateSQL(MYSQL_RES* result, std::vector<std::string>& outSQLVec);

	int GetRowCount(const string& SQL);
	
private:
	string m_Host;
	int    m_Port;
	string m_DBName;
	string m_UserName;
	string m_PassWord;
	MYSQL* m_DBHandle;
	MYSQL_STMT* m_DBStmt;
	MYSQL_RES*  m_DBRes;
	bool   m_IsConnect;
};

#endif
