RM=rm -rf
CP=cp -rf
CC=g++
MKDIR=mkdir -p
CFLAGS= -c -Wall -g -fPIC -std=c++11 -fno-elide-constructors

STATIC_LIB=libctm.a
DYNAMIC_LIB=libctm.so

BIN_PATH=../bin
INSTALL=../lib
INSTALL_INC=
HEADERS=headers

SUBDIRS=$(shell find ./* -type d)
INC_SUBDIRS=$(shell find ./* -type d | awk -F' ' '{print "-I"$$0}')
INC_PATH=$(INC_SUBDIRS) -I./ -I../third/include
LIB_PATH=
LIB=

C_SRC=$(shell find . -type f | grep "\.c$$" | grep -v grep)
C_OBJ=$(patsubst %.c, %.o, $(C_SRC))
CPP_SRC=$(shell find . -type f | grep "\.cpp$$" | grep -v grep)
CPP_OBJ=$(patsubst %.cpp, %.o, $(CPP_SRC))
OBJS=$(C_OBJ) $(CPP_OBJ)

VPATH=$(SUBDIRS)
#---------------------------------------------------------------------------------------------

.PHONY:all
	
all:$(STATIC_LIB) $(DYNAMIC_LIB) $(HEADERS)
	
$(STATIC_LIB):$(OBJS)
	ar -cr $@ $(OBJS) $(LIB_PATH) $(LIB)
$(DYNAMIC_LIB):$(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(LIB_PATH) $(LIB)
$(CPP_OBJ):%.o:%.cpp
	$(CC) $(CFLAGS) $(INC_PATH) $< -o $@
$(C_OBJ):%.o:%.c
	$(CC) $(CFLAGS) $(INC_PATH) $< -o $@
$(HEADERS):
	./header.sh > ctm.h

install:
	$(CP) $(STATIC_LIB) $(DYNAMIC_LIB) $(INSTALL)
	$(MKDIR) $(INSTALL_INC)
	$(CP) *.h $(INSTALL_INC)

clean:
	$(RM) $(STATIC_LIB) $(DYNAMIC_LIB) $(OBJS)
