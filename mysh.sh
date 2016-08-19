#! /bin/bash

. auto/configure --add-module=src/myModule
make clean
make
echo "920524" | sudo -S make install
