RM=rm -rf
CP=cp -rf
CC=g++
CFLAGS= -c -Wall -g

INC=-I/usr/include/mysql/
LIB_PATH=/usr/lib64/mysql/libmysqlclient.a
LIB= -lpthread -ldl -lz
C_SRC=$(wildcard *.c)
C_OBJ=$(patsubst %.c, %.o, $(C_SRC))
CPP_SRC=$(wildcard *.cpp)
CPP_OBJ=$(patsubst %.cpp, %.o, $(CPP_SRC))
OBJS=$(C_OBJ) $(CPP_OBJ)

PROJECT_1=write
PROJECT_2=read

INSTALL=

VPATH=./
#---------------------------------------------------------------------------------------------

.PHONY:all
	
all:$(PROJECT_1) $(PROJECT_2) 
	
$(PROJECT):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LIB_PATH) $(LIB)
$(CPP_OBJ):%.cpp:%.o
	$(CC) $(CFLAGS) $(INC) $< -o $@
$(C_OBJ):%.c:%.o
	$(CC) $(CFLAGS) $(INC) $< -o $@

install:
	
clean:
	$(RM) $(PROJECT) $(OBJS)
