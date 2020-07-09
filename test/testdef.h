#ifndef CTM_TEST_TESTDEF_H__
#define CTM_TEST_TESTDEF_H__
#include <unordered_map>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <ctype.h>
#include <iostream>

#include "common/singleton.h"
#include "common/string_tools.h"
#include "common/time_tools.h"
#include "common/msg.h"
#include "common/com.h"
#include "common/log.h"
#include "common/random.h"
#include "common/inifile.h"
#include "common/message.h"

#include "net/socket.h"
#include "ipc/mmap.h"
#include "ipc/semaphore.h"
#include "ipc/sharememory.h"
#include "module/timer.h"

#include "md5/md5.h"
#include "thread/thread.h"

using namespace ctm;
using namespace std;

typedef int (*TestFuncion)(int argc, char** argv);
extern unordered_map<string, TestFuncion> TestFunctionMap;
extern void RegisterTestFunc(const string name, TestFuncion func);

#define DECLARE_FUNC(FuncName)\
static int FuncName(int argc, char** argv);\
class Register##FuncName\
{\
public:\
    Register##FuncName() { RegisterTestFunc(#FuncName, FuncName); }\
};\
static Register##FuncName *globalRegister##FuncName = new Register##FuncName();\
static int FuncName(int argc, char** argv)

inline void WaitEnd()
{
	cout<<"Please enter any to exit!"<<endl;
	int a;
	cin >> a;
}

#endif