#ifndef _H_H_MYSQLITE3
#define _H_H_MYSQLITE3
#include "sqlite3.h"
#include "DbResult.h"
#include <string>
#include <vector>

class CMySQLite3
{
public:
	CMySQLite3() :
	  m_bIsOpen(false),
	  m_DbHandle(NULL),
	  m_DbName(""),
	  m_strErr("No open database")
	  {

	  }

	  CMySQLite3(const char* dbName) :
        m_bIsOpen(false),
        m_DbHandle(NULL),
        m_DbName(dbName),
		m_strErr("No open database")
	  {

	  }

	  ~CMySQLite3()
	  {
		  Close();
	  }

	  bool Open();

	  bool Open(const char* dbName)
	  {
		  m_DbName = dbName;
		  return Open();
	  }

	  bool IsOpen()
	  {
		  return m_bIsOpen;
	  }

	  void Close()
	  {
		  if(m_bIsOpen)
		  {
			  sqlite3_close(m_DbHandle);
			  m_DbHandle = NULL;
			  m_bIsOpen = false;
		  }
	  }

	  int Exectue(const char* strSQL);

	  int Query(const char* strSQL, char*** result, int* nRow, int* column);

	  int Query(const char* strSQL, CDbResult& outRes);

	  int Query(const char* strSQL, CDbResult& outRes, vector<ColumnHeader>& vecColHeader);

	  int Export(const string& strTableName, string& strOutSQL, int rowOffset = 0, int rowCount = -1);

	  int ExportToXml(const string& strTableName, string& strOutXml, int rowOffset = 0, int rowCount = -1);

	  int GetRowNum(const string& tableName);

	  int ClearTable(const string& tableName);

	  int GetCreateSQL(const string& tableName, string& strOutSQL);

	  std::string GetError() const
	  {
		  return m_strErr;
	  }

	  void Test();

	  bool CMySQLite3::IsExist(const string& strTabName);

private:
	bool IsStrType(const string& typeName);

private:
	bool m_bIsOpen;
	sqlite3 *m_DbHandle;
	std::string m_DbName;
	std::string m_strErr;
};

#endif
