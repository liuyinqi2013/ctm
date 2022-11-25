#include <stdio.h>

#include "io.h"
#include "buffer.h"

using namespace ctm;

int TestIO(int argc, char** argv) 
{
    TestReadAll(argv[1]);
    TestReadFull(argv[1]);
    return 0;
}

void TestReadAll(const char* fileName) 
{
    FILE* fd = fopen(fileName, "rb");
    if (!fd) {
        fprintf(stderr, "open file failed. filename:%s\n", fileName);
        return;
    }

    std::string out; 
    int err = ReadAll(fileno(fd), out);
    if (err != IO_OK) {
        fprintf(stderr, "read all failed. filename:%s, errcode:%d\n", fileName, err);
        return;
    }

    printf("read all ok. size:%ld\n", out.size());

}

void TestWrite(const char* fileName, Buffer* buf) 
{
    FILE* fd = fopen(fileName, "w+");
    if (!fd) {
        fprintf(stderr, "open file failed. filename:%s\n", fileName);
        return;
    }

    int err = WriteFull(fileno(fd), buf);
    if (err != IO_OK) {
        fprintf(stderr, "write failed. filename:%s, errcode:%d\n", fileName, err);
        return;
    }
    fclose(fd);

    printf("write ok. size:%ld\n", buf->Len());
}


void TestReadFull(const char* fileName) 
{
    FILE* fd = fopen(fileName, "rb");
    if (!fd) {
        fprintf(stderr, "open file failed. filename:%s\n", fileName);
        return;
    }

    Buffer buf(100);
    int err = ReadFull(fileno(fd), &buf);
    if (err != IO_OK) {
        fprintf(stderr, "read full failed. filename:%s, errcode:%d\n", fileName, err);
        return;
    }

    printf("read full ok. buf:%s, size:%ld\n", buf.Data(), buf.Len());

    std::string outFileName = std::string(fileName) + ".bak";
    TestWrite(outFileName.c_str(), &buf);
}