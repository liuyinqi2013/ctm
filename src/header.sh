echo "#ifndef CTM__HEAD_FILES_H__"
echo "#define CTM__HEAD_FILES_H__"
echo 
headers=`find ./ -type f  -name "*.h"`
for file in $headers
do
	if [ "$file" != "./ctm.h" ]
	then
		echo "#include \"$file\""
	fi
done
echo
echo "#endif"
