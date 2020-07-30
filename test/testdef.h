#ifndef CTM_TEST_TESTDEF_H__
#define CTM_TEST_TESTDEF_H__
#include <string.h>
#include <signal.h>
#include <iostream>
#include <ctype.h>
#include <iostream>
#include <map>
#include <unordered_map>

#include "ctm.h"
#include "md5/md5.h"

using namespace ctm;
using namespace std;

typedef int (*TestFuncion)(int argc, char** argv);
extern void RegisterTestFunc(const string name, TestFuncion func);

#define DECLARE_FUNC(FuncName)\
static int FuncName(int argc, char** argv);\
class Register##FuncName\
{\
public:\
    Register##FuncName() { RegisterTestFunc(#FuncName, FuncName); delete this; }\
};\
static Register##FuncName *globalRegister##FuncName = new Register##FuncName();\
static int FuncName(int argc, char** argv)\

#define DECLARE_FUNC_EX(FuncName)\
static int FuncName##Ex(int argc, char** argv);\
DECLARE_FUNC(FuncName) {\
	CClock clock;\
	FuncName##Ex(argc, argv);\
	cerr << clock.RunInfo() << endl;\
	return 0;\
}\
static int FuncName##Ex(int argc, char** argv)\

#define CHECK_PARAM(argc, min, info)  { if (argc < min) {fprintf(stderr, "%s\n", info); return -1; } }

inline void WaitEnd()
{
	cerr<<"Please enter any to exit!"<<endl;
	int a;
	cin >> a;
}



#endif