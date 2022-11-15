#include <stdio.h>

#include "io.h"
#include "buffer.h"
#include "defer.h"

using namespace ctm;

typedef int (*F)(FILE*);


void TestReadAll(const char* fileName);
void TestReadFull(const char* fileName);
void TestDefer();

int TestIO(int argc, char** argv) 
{
    TestDefer();
    // TestReadAll(argv[1]);
    // TestReadFull(argv[1]);
    return 0;
}

void TestReadAll(const char* fileName) 
{
    FILE* fd = fopen(fileName, "rb");
    if (!fd) {
        fprintf(stderr, "open file failed. filename:%s\n", fileName);
        return;
    }

    auto d = Defer(fclose, fd);

    string out; 
    int err = ReadAll(fileno(fd), out);
    if (err != IO_OK) {
        fprintf(stderr, "read all failed. filename:%s, errcode:%d\n", fileName, err);
        return;
    }

    printf("read all ok. size:%ld\n", out.size());
}

void TestReadFull(const char* fileName) 
{
    FILE* fd = fopen(fileName, "rb");
    if (!fd) {
        fprintf(stderr, "open file failed. filename:%s\n", fileName);
        return;
    }

    auto d = Defer(fclose, fd);

    Buffer buf(100);
    int err = ReadFull(fileno(fd), &buf);
    if (err != IO_OK) {
        fprintf(stderr, "read full failed. filename:%s, errcode:%d\n", fileName, err);
        return;
    }

    printf("read full ok. buf:%s, size:%ld\n", buf.Data(), buf.Len());
}

void Func1(int arg)
{
    printf("run func1. arg:%d\n", arg);
}

void Func2(int arg, char* arg1)
{
    printf("run func2. arg:%d, arg1:%s\n", arg, arg1);
}

class A
{
public:
    A(const char* name) : m_cnt(0), m_name(name) {}
    void func() 
    {
        printf("run class A func, cnt:%d, name:%s\n", ++m_cnt, m_name.c_str());
    }

    int m_cnt;  
    string m_name;
};


void TestDefer() 
{
    /*
    auto d1 = Defer(Func1, 1);
    auto d2 = Defer(Func2, 1, "name");
    */

    A a("laoda");
 
    auto d1 = DeferObj(a, &A::func);
    auto d2 = DeferObj(a, &A::func);
    defer(a, &A::func);

    printf("test defer.\n");
}