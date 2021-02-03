#ifndef __H__H_TCMOPERATORDB_H
#define __H__H_TCMOPERATORDB_H

#include <string>
#include <vector>
#include <map>
#include <mysql.h>

typedef std::vector<std::map<std::string, std::string> > CStrMapArr;

class CMySQLDB
{
public:
	CMySQLDB() :
		m_Host(""),
		m_Port(0),
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
		const char* userName,
		const char* passWord):
		m_Host(hostName),
		m_Port(port),
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
		const char* userName,
		const char* passWord)
	{
		m_Host = hostName;
		m_Port = port;
		m_UserName = userName;
		m_PassWord = passWord;
		return Connect();
	}

    bool IsConnect() const
    {
        return m_IsConnect;
    }

	bool Query(const char* sql);

	bool Query(const char* sql, CStrMapArr& out);

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

	int StmtBindParamStr(int iparamNum, const std::string& strVal);

	int StmtBind(MYSQL_BIND* bind);

	std::string StmtSQLStatus();

	int StmtExecute();

	int GetTabRowCount(const std::string& tableName);

	int ClearTable(const std::string& tableName);

	int Export(const std::string& tableName, std::string& strOutSQL, int rowOffset = 0, int rowCount = -1);

	void DisConnect()
	{
        if(m_IsConnect) {
            mysql_close(m_DBHandle);
            m_IsConnect = false;
        }
	}

	std::string GetLastErr();

	MYSQL* GetMySQLHandle() const ;

	void FreeResult()
	{
		if(m_DBRes) {
			mysql_free_result(m_DBRes);
			m_DBRes = NULL;
		}
	}
	
	void GetFields(MYSQL_RES* result, std::vector<std::string>& outFieldsVec);

	void GetValues(MYSQL_RES* result, CStrMapArr& outVaulesVec);

	bool Ping()
	{
		return (!mysql_ping(m_DBHandle));
	}

	static std::string EscapeString(const std::string& strIn);

	bool SelectDB(const std::string& db);

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

	std::string AddFlag(const char* value);

	void ConvertInsertSQL(MYSQL_RES* result, std::vector<std::string>& outSQLVec);

	void ConvertUpdateSQL(MYSQL_RES* result, std::vector<std::string>& outSQLVec);

	int GetRowCount(const std::string& SQL);
	
private:
	std::string m_Host;
	int    m_Port;
	std::string m_UserName;
	std::string m_PassWord;
	MYSQL* m_DBHandle;
	MYSQL_STMT* m_DBStmt;
	MYSQL_RES*  m_DBRes;
	bool   m_IsConnect;
};

#endif
