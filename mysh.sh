#! /bin/bash

make clean
echo $?;
if [ $? != 0 ]; then
	echo "clean error";
fi

. auto/configure --add-module=src/myModule
if [ $? != 0 ]; then
	echo "configure error";
	exit 1;
fi

make
if [ $? != 0 ]; then
	echo "make error";
	exit 1;
fi

echo "dash1234" | sudo -S make install

echo "update nginx server!"
