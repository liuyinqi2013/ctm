#include "ctm.h"
#include <iostream>
#include <unistd.h>
#include "common/singleton.h"
#include "common/com.h"
#include "thread/thread.h"

using namespace ctm;
using namespace std;

class TestSingleton : public Singleton<TestSingleton>, public Thread
{
public:
	void hello() { cout<<"hello"<<endl; }

protected:
	int Run()
	{
		cout<<"thread id : "<<GetThreadId()<<endl;
		cout<<"thread self : "<<pthread_self()<<endl;
		cout<<"thread name : "<<GetName()<<endl;
		cout<<"thread status : "<<GetStatus()<<endl;
		cout<<"thread str2int : "<<S2I("123")<<endl;
		cout<<"thread int2str : "<<I2S(10)<<endl;
		cout<<"thread str2double : "<<S2D("123.23")<<endl;
		cout<<"thread double2str : "<<D2S(1.004)<<endl;
		
		while(1)
		{
			cout<<"I am TestSingleton thread"<<endl;
			sleep(1);
		}
		return 0;
	}
};

int main(int argc, char **argv)
{
	TestSingleton::GetInstance()->hello();
	TestSingleton::GetInstance()->Start();
	TestSingleton::GetInstance()->Join();
	return 0;
}