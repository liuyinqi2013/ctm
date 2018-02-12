#ifndef _H_H_DBRESULT
#define _H_H_DBRESULT

#include <map>
#include <string>
#include <vector>

using namespace std;

class CUseCount
{
public:
	CUseCount() : num(new int(1))
	{

	}

	CUseCount(const CUseCount& rhs) : num(rhs.num)
	{
		++*num;
	}

	~CUseCount()
	{
		if(--*num == 0)
		{
			delete num;
		}
	}

	CUseCount& operator=(CUseCount& rhs)
	{
		IsDeleleForCopy(rhs);
		return *this;
	}

	bool Only()
	{
		return (1 == *num) ? true : false;
	}

	bool IsDeleleForCopy(CUseCount& rhs)
	{
		++*rhs.num;
		if(--*num == 0)
		{
			delete num;
			num = rhs.num;
			return true;
		}
		num = rhs.num;
		return false;
	}

private:
	int *num;
};

class CDbRow
{
public:
	CDbRow() : 
	  m_pMapCol(new map<string, string*>)
	{

	}

	CDbRow(const CDbRow& rhs) :
		m_Count(rhs.m_Count),
		m_pMapCol(rhs.m_pMapCol)
	{

	}

	~CDbRow()
	{
		if(m_Count.Only())
		{
			Free();
		}
	}

	CDbRow& operator=(CDbRow& rhs)
	{
		if(m_Count.IsDeleleForCopy(rhs.m_Count))
		{
			Free();	
		}
		m_pMapCol = rhs.m_pMapCol;
		return *this;

	}

	string& operator[](const string& key)
	{
		return *(*m_pMapCol)[key];
	}

	string& operator[](const int& index);

	void Insert(const string key, string* pvalue)
	{
		(*m_pMapCol)[key] = pvalue;
	}

	int Size() const 
	{
		return m_pMapCol->size();
	}

private:
	void Free();
private:
	CUseCount m_Count;
	map<string, string*>* m_pMapCol;
};

class CDbResult
{
public:
	CDbResult():
	  m_pVecRows(new vector<CDbRow *>)
	{

	}

	CDbResult(CDbResult& rhs): 
		m_pVecRows(rhs.m_pVecRows),
		m_Count(rhs.m_Count)
		
	{

	}
	
	~CDbResult()
	{
		if(m_Count.Only())
		{
			Free();
		}
	}

	CDbResult& operator=(CDbResult& rhs)
	{
		if(m_Count.IsDeleleForCopy(rhs.m_Count))
		{
			Free();	
		}
		m_pVecRows = rhs.m_pVecRows;
		return *this;
	}

	CDbRow& operator[](int index) const
	{
		return *((*m_pVecRows)[index]);
	}

	void Insert(CDbRow* row)
	{
		m_pVecRows->push_back(row);
	}

	int Size() const
	{
        return m_pVecRows->size();
	}

private:

	void Free();

private:
	CUseCount m_Count;
	vector<CDbRow*>* m_pVecRows;
};

typedef struct ColumnHeader
{
	string strName;
	string strType;
	bool   bIsStr;
} ColumnHeader;

#endif
