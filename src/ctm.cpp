#include "ctm.h"
#include "common/Singleton.h"
#include <iostream>
using namespace ctm;
using namespace std;

class TestSingleton : public Singleton<TestSingleton>
{
public:
	void hello() { cout<<"hello"<<endl; }
};

int main(int argc, char **argv)
{
	TestSingleton::getInstance()->hello();
	return 0;
}