#!/bin/bash
SRC_CPPS=`find . -type f | grep .cpp$`
echo $SRC_CPPS
cat /dev/null > obj.tmp
for name in $SRC_CPPS
do
	OBJ=`echo $name | awk -F'/' '{print $NF}' | sed s/.cpp/.o/g`
	echo "$OBJ:$name" >> obj.tmp
	echo "	g++ -c $name -o $OBJ" >> obj.tmp
done

OBJS=`cat obj.tmp`
echo $OBJS
