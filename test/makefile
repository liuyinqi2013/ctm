RM=rm -rf
CP=cp -rf
CC=g++
CFLAGS= -c -Wall -g -std=c++11

TARGET0=testdemo
TARGET1=serverdemo
TARGET2=clientdemo

BIN_PATH=../bin
INSTALL=

SUBDIRS=$(shell find ./* -type d)
SRCDIRS=$(shell find ../src -type d)
INC_SUBDIRS=$(shell find ./* -type d | awk -F' ' '{print "-I"$$0}')
INC_SRCDIS=$(shell find ../src -type d | awk -F' ' '{print "-I"$$0}')
INC_PATH=$(INC_SUBDIRS) $(INC_SRCDIS) -I./ -I../third_party/include
LIB_PATH=-L../third_party/lib/
STATIC_LIB=../third_party/lib/libjsoncpp.a ../third_party/lib/libmd5.a
DYNAMIC_LIB=-lpthread -ldl
LIB=$(STATIC_LIB) $(DYNAMIC_LIB)

C_SRC=$(shell find . -type f | grep "\.c$$" | grep -v grep)
#C_SRC=$(notdir $(C_SRC_NAME)) 
C_OBJ=$(patsubst %.c, %.o, $(C_SRC))
CPP_SRC=$(shell find . -type f | grep "\.cpp$$" | grep -v grep)
#CPP_SRC=$(notdir $(CPP_SRC_NAME))

COM_OBJ=../src/common/string_tools.o ../src/common/time_tools.o \
	../src/net/socket.o \
	../src/common/refcount.o \
	../src/common/msg.o \
	../src/common/log.o \
	../src/common/platform.o \
	../src/ipc/mmap.o \
	../src/thread/sem.o \
	../src/ipc/semaphore.o \
	../src/ipc/ipc_common.o \
	../src/ipc/sharememory.o \
	../src/net/netclient.o \
	../src/thread/thread.o \
	../src/net/netmsg.o \
	../src/common/inifile.o \
	../src/common/random.o \
	../src/common/message.o \
	../src/module/timer.o \
	../src/net/tcpserver.o \
	../src/net/tcpclient.o \
	../src/net/netpacket.o
	
CPP_OBJ=$(patsubst %.cpp, %.o, $(CPP_SRC)) $(COM_OBJ)
	
	
OBJS0=$(COM_OBJ) testdemo.o testmain.o
OBJS1=$(COM_OBJ) serverdemo.o
OBJS2=$(COM_OBJ) clientdemo.o

VPATH=$(SUBDIRS) $(SRCDIRS)
#---------------------------------------------------------------------------------------------

.PHONY:all
	
all:$(TARGET0) $(TARGET1) $(TARGET2)
	
$(TARGET0):$(OBJS0)
	$(CC) -o $@ $(OBJS0) $(LIB_PATH) $(LIB)

$(TARGET1):$(OBJS1)
	$(CC) -o $@ $(OBJS1) $(LIB_PATH) $(LIB)

$(TARGET2):$(OBJS2)
	$(CC) -o $@ $(OBJS2) $(LIB_PATH) $(LIB)
	
$(CPP_OBJ):%.o:%.cpp
	$(CC) $(CFLAGS) $< -o $@ $(INC_PATH)
$(C_OBJ):%.o:%.c
	$(CC) $(CFLAGS) $< -o $@ $(INC_PATH)

install:
	
clean:
	$(RM) $(TARGET0) $(TARGET1) $(TARGET2) $(CPP_OBJ)
