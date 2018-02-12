 #include "DbResult.h"


void CDbRow::Free()
{
	if(m_pMapCol == NULL)
	{
		return ;
	}

	map<string, string*>::iterator it = m_pMapCol->begin();
	for(;it != m_pMapCol->end(); it++)
	{
		delete it->second;
	}

	delete m_pMapCol;
	m_pMapCol = NULL;
}

string& CDbRow::operator[](const int& index)
{	
	if(index < 0 || index > m_pMapCol->size())
	{
		return string("");
	}
	map<string, string*>::iterator it = m_pMapCol->begin();
	int i = 0;
	while(i < index && it != m_pMapCol->end()) it++;

	return *(it->second);
}

void CDbResult::Free()
{
	if(NULL == m_pVecRows)
	{
		return;
	}
	vector<CDbRow *>::iterator it = m_pVecRows->begin();
	for(; it != m_pVecRows->end(); it++)
	{
		delete *it;
	}

	delete m_pVecRows;
	m_pVecRows = NULL;
}
