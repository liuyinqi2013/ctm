#include "define.h"
#include <algorithm>

static unordered_map<string, TestFuncion>* pTestFunctionMap = NULL;
void RegisterTestFunc(const string name, TestFuncion func)
{
    if (pTestFunctionMap == NULL) pTestFunctionMap = new unordered_map<string, TestFuncion>;
    (*pTestFunctionMap)[name] = func;
}

int Usage(int argc, char** argv)
{
    printf("Usage:%s [cmd] [parm...]\n", argv[0]);
    printf("cmd list:");

    int col = 5;
    int row = (pTestFunctionMap->size() + col - 1) / col;
    CCharTable cmdtab(row, col);

    CStyle* style = cmdtab.CreateStyle();
	style->SetHorAlign(CStyle::LIFT);
    cmdtab.SetStyle(style);
    cmdtab.SetGapCout(0);

    vector<string> sortVec;
    unordered_map<string, TestFuncion>::iterator it = pTestFunctionMap->begin();
    for(; it != pTestFunctionMap->end(); it++) 
    {
        sortVec.push_back(it->first);
    }

    sort(sortVec.begin(), sortVec.end());

    for (size_t i = 0; i < sortVec.size(); ++i)
    {
        cmdtab.Write(i / col, i % col, sortVec[i]);
        cmdtab.Column(i % col)->SetWidth(max(cmdtab.Column(i % col)->Width(), sortVec[i].size()));
    }

    cmdtab.Print();

    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 2) 
    {
       return  Usage(argc, argv);
    }

    unordered_map<string, TestFuncion>::iterator it = pTestFunctionMap->find(argv[1]);
    if (it == pTestFunctionMap->end()) 
    {
        return Usage(argc, argv);
    }

    return it->second(--argc, ++argv);
}