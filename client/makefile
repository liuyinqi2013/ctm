RM=rm -rf
CP=cp -rf
CC=g++
CFLAGS= -c -Wall -g

TARGET0=game_client

BIN_PATH=../bin
INSTALL=

SUBDIRS=$(shell find . -type d)
SRCDIRS=$(shell find ../src -type d)
INC_SUBDIRS=$(shell find . -type d | awk -F' ' '{print "-I"$$0}')
INC_SRCDIS=$(shell find ../src -type d | awk -F' ' '{print "-I"$$0}')
INC_PATH=$(INC_SUBDIRS) $(INC_SRCDIS)
LIB_PATH=/usr/lib64/mysql/libmysqlclient.a
LIB= -lpthread -ldl -lz

C_SRC=$(shell find . -type f | grep "\.c$$" | grep -v grep)
#C_SRC=$(notdir $(C_SRC_NAME)) 
C_OBJ=$(patsubst %.c, %.o, $(C_SRC))
CPP_SRC=$(shell find . -type f | grep "\.cpp$$" | grep -v grep)
#CPP_SRC=$(notdir $(CPP_SRC_NAME))

COM_OBJ=../src/common/string_tools.o ../src/common/time_tools.o \
	../src/net/socket.o \
	../src/net/select.o \
	../src/common/refcount.o \
	../src/common/msg.o \
	../src/common/log.o \
	../src/common/platform.o \
	../src/ipc/mmap.o \
	../src/thread/sem.o \
	../src/ipc/semaphore.o \
	../src/ipc/sharememory.o \
	../src/jsoncpp/jsoncpp.o \
	../src/net/netclient.o \
	../src/thread/thread.o \
	../src/net/netmsg.o \
	../src/game/gamemsg.o \
	../src/common/inifile.o \
	../src/game/card.o 

	
CPP_OBJ=$(patsubst %.cpp, %.o, $(CPP_SRC)) $(COM_OBJ)
	
	
OBJS0=$(COM_OBJ) gameclient.o main.o

VPATH=$(SUBDIRS) $(SRCDIRS)
#---------------------------------------------------------------------------------------------

.PHONY:all
	
all:$(TARGET0) 
	
$(TARGET0):$(OBJS0)
	$(CC) -o $@ $(OBJS0) $(LIB_PATH) $(LIB)
	
$(CPP_OBJ):%.o:%.cpp
	$(CC) $(CFLAGS) $< -o $@ $(INC_PATH)
$(C_OBJ):%.o:%.c
	$(CC) $(CFLAGS) $< -o $@ $(INC_PATH)

install:
	
clean:
	$(RM) $(TARGET0) $(TARGET1) $(TARGET2) $(CPP_OBJ)
