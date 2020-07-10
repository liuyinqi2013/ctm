#ifndef CTM_TEST_TESTDEF_H__
#define CTM_TEST_TESTDEF_H__
#include <string.h>
#include <signal.h>
#include <iostream>
#include <ctype.h>
#include <iostream>
#include <map>
#include <unordered_map>

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
	cout << clock.RunInfo() << endl;\
	return 0;\
}\
static int FuncName##Ex(int argc, char** argv)\

#define CHECK_PARAM(argc, min, info)  { if (argc < min) {printf("%s\n", info); return -1; } }

inline void WaitEnd()
{
	cout<<"Please enter any to exit!"<<endl;
	int a;
	cin >> a;
}



#endif