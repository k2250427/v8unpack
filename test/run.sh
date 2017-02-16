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
DIFFLOG=diff.log

rm -rf $TMPFILE $OUTDIRNAME

echo 'build/parse test...'

# Собираем файл, распаковываем и сравниваем каталоги
$UNPACK -build $DIRNAME $TMPFILE >/dev/null
$UNPACK -parse $TMPFILE $OUTDIRNAME >/dev/null
diff -r $DIRNAME $OUTDIRNAME >$DIFFLOG 2>&1

if [ $? -ne 0 ];
then
	echo Failed
	exit 1
fi

echo Passed

rm -rf $OUTDIRNAME

echo 'build/unpack/pack/parse test...'

$UNPACK -unpack $TMPFILE $OUTDIRNAME >/dev/null
rm $TMPFILE
$UNPACK -pack $OUTDIRNAME $TMPFILE >/dev/null
rm -rf $OUTDIRNAME
$UNPACK -parse $TMPFILE $OUTDIRNAME >/dev/null
diff -r $DIRNAME $OUTDIRNAME >$DIFFLOG 2>&1

if [ $? -ne 0 ];
then
	echo Failed
	exit 1
fi

echo Passed

rm -rf $DIRNAME
rm -rf $OUTDIRNAME
rm $TMPFILE
rm $DIFFLOG
