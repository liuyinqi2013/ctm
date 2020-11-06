#!/bin/sh

ctm_home=
head_path=/usr/local/include/ctm
lib_path=/usr/local/lib

set -e

if [ $# -eq 1 ]; then
	mkdir -p $1
	if [ ! -d $1 ]; then
		exit 1
	fi

	ctm_home="$1"
	head_path="${ctm_home}/include/ctm"
	lib_path="${ctm_home}/lib"
fi

if [ -d ${head_path} ]; then
	while ((1))
	do
		read -p "files already exists re-install [y/n]:" yes
		if [ ${yes} == "y" ]; then
			rm -r ${head_path}/*
			break
		elif [ ${yes} == "n" ]; then
			exit 2
		fi
	done
fi

mkdir -p ${head_path}
mkdir -p ${lib_path}

cp ./src/*.so ${lib_path}
cp ./src/*.a  ${lib_path}

head_path=`readlink -f ${head_path}`

cd src

head_dir_list=`find * -type d`

for name in ${head_dir_list}
do
	echo "mkdir -p ${head_path}/$name"
	mkdir -p ${head_path}/$name
done

head_file_list=`find * -type f | grep "\.h$"`

for name in ${head_file_list}
do
	echo "cp $name ${head_path}/$name"
	cp $name ${head_path}/$name
done

cd ..

echo "Complete!"
