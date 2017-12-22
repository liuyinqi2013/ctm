#include "common/Singleton.h"
#include "common/string_tools.h"
#include "common/time_tools.h"

#include <iostream>
#include <ctype.h>

using namespace ctm;
using namespace std;

class TestSingleton : public Singleton<TestSingleton>
{
public:
	void hello() { cout<<"hello"<<endl; }
};

int main(int argc, char **argv)
{
	string a("\r\n**}&\n");
	cout<<Trimmed(a)<<endl;
	cout<<Sec()<<endl;
	cout<<Usec()<<endl;
	cout<<DateTime()<<endl;
	TestSingleton::getInstance()->hello();
	return 0;
}
