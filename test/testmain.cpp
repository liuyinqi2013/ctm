#include "testdef.h"

unordered_map<string, TestFuncion> TestFunctionMap;
void RegisterTestFunc(const string name, TestFuncion func)
{
    TestFunctionMap[name] = func;
}

DECLARE_FUNC(Usage)
{
    printf("Usage:%s [name] [parm...]\n", argv[0]);
    printf("  name list:\n");
    unordered_map<string, TestFuncion>::iterator it = TestFunctionMap.begin();
    for(; it != TestFunctionMap.end(); it++)
    {
        printf("  %s\n", it->first.c_str());
    }

    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 2) 
    {
       return  Usage(argc, argv);
    }

    unordered_map<string, TestFuncion>::iterator it = TestFunctionMap.find(argv[1]);
    if (it == TestFunctionMap.end())
    {
        return Usage(argc, argv);
    }

    return it->second(argc, argv);
}