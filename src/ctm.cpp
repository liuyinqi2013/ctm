#include "ctm.h"
#include <iostream>
#ifndef WIN32
#include <unistd.h>
#else
#include <Windows.h>
#define sleep(n) Sleep((n) * 1000)
#endif
#include "common/singleton.h"
#include "common/com.h"
#include "thread/thread.h"
#include "thread/mutex.h"

using namespace ctm;
using namespace std;

static int num = 0;
Mutex mutex;

class TestSingleton : public Singleton<TestSingleton>, public Thread
{
public:
	TestSingleton(const std::string& name) : Thread(name)
	{
	}
	
	void hello() { cout<<"hello"<<endl; }

protected:
	int Run()
	{
		cout<<"thread name : "<<GetName()<<endl;
		cout<<"thread status : "<<GetStatus()<<endl;
		cout<<"thread str2int : "<<S2I("123")<<endl;
		cout<<"thread int2str : "<<I2S(10)<<endl;
		cout<<"thread str2double : "<<S2D("123.23")<<endl;
		cout<<"thread double2str : "<<D2S(1.004)<<endl;
		
		while(1)
		{
			{
				LockOwner owner(mutex);
				num++;
				cout<<"thread : "<<GetName()<<" num : "<<num<<endl;
			}
			sleep(1);
		}
		return 0;
	}
	
};

int main(int argc, char **argv)
{
	TestSingleton t1("t1"), t2("t2");
	t1.Start();
	t2.Start();
	t1.Join();
	t2.Join();
	return 0;
}