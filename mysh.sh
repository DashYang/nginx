#! /bin/bash

make clean
. auto/configure --add-module=src/myModule
make
echo "920524" | sudo -S make install
