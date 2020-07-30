#include "testdef.h"

static unordered_map<string, TestFuncion>* pTestFunctionMap = NULL;
void RegisterTestFunc(const string name, TestFuncion func)
{
    if (pTestFunctionMap == NULL) pTestFunctionMap = new unordered_map<string, TestFuncion>;
    (*pTestFunctionMap)[name] = func;
}

int Usage(int argc, char** argv)
{
    printf("Usage:%s [cmd] [parm...]\n", argv[0]);
    printf("cmd list:\n");
    unordered_map<string, TestFuncion>::iterator it = pTestFunctionMap->begin();
    for(; it != pTestFunctionMap->end(); it++) 
    {
        if (IsCharDevice(stdout))
            printf("    %s\n", ColorString(it->first, SKYBLUE).c_str());
        else
            printf("    %s\n", it->first.c_str());
    }

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