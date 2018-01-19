RM=rm -rf
CP=cp -rf
CC=g++
CFLAGS= -c -Wall -g

TARGET=test_unit
BIN_PATH=../bin
INSTALL=

SUBDIRS=$(shell find . -type d)
SRCDIRS=$(shell find ../src -type d)
INC_SUBDIRS=$(shell find . -type d | awk -F' ' '{print "-I"$$0}')
INC_SRCDIS=$(shell find ../src -type d | awk -F' ' '{print "-I"$$0}')
INC_PATH=$(INC_SUBDIRS) $(INC_SRCDIS)
LIB_PATH=/usr/lib64/mysql/libmysqlclient.a
LIB= -lpthread -ldl -lz

C_SRC=$(shell find . -type f | grep .c$$ | grep -v grep)
#C_SRC=$(notdir $(C_SRC_NAME)) 
C_OBJ=$(patsubst %.c, %.o, $(C_SRC))
CPP_SRC=$(shell find . -type f | grep .cpp$$ | grep -v grep)
#CPP_SRC=$(notdir $(CPP_SRC_NAME)) 
CPP_OBJ=$(patsubst %.cpp, %.o, $(CPP_SRC)) ../src/common/string_tools.o ../src/common/time_tools.o \
	../src/net/socket.o \
	../src/net/select.o \
	../src/common/refcount.o \
	../src/common/msg.o
OBJS=$(C_OBJ) $(CPP_OBJ)

VPATH=$(SUBDIRS) $(SRCDIRS)
#---------------------------------------------------------------------------------------------

.PHONY:all
	
all:$(TARGET)
	
$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LIB_PATH) $(LIB)
	$(CP) $@ $(BIN_PATH)
$(CPP_OBJ):%.o:%.cpp
	$(CC) $(CFLAGS) $(INC_PATH) $< -o $@
$(C_OBJ):%.o:%.c
	$(CC) $(CFLAGS) $(INC_PATH) $< -o $@

install:
	
clean:
	$(RM) $(TARGET) $(OBJS)
