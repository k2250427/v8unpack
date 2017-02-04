#!/usr/bin/env bash

function create_data {
	if [ $1 -eq 0 ];
	then
		dd if=/dev/urandom of=$2 bs=1 count=4096 >/dev/null 2>&1
	else
		mkdir $2
		for i in `seq 0 9`;
		do
			create_data `expr $1 - 1` "$2/$i"
		done
	fi

}

DIRNAME='in-test'
rm -rf $DIRNAME
depth=3
create_data $depth $DIRNAME

UNPACK=../bin/Release/v8unpack
OUTDIRNAME='out-test'
TMPFILE=file.tmp

rm -rf $TMPFILE $OUTDIRNAME

# Собираем файл, распаковываем и сравниваем каталоги
$UNPACK -build $DIRNAME $TMPFILE >/dev/null
$UNPACK -parse $TMPFILE $OUTDIRNAME >/dev/null
diff -r $DIRNAME $OUTDIRNAME

if [ $? -eq 0 ];
then

	rm -rf $DIRNAME
	rm -rf $OUTDIRNAME

fi

rm $TMPFILE

