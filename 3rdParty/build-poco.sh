#!/bin/bash

rm -rf ../poco-static
rm -rf cmake-build
cmake -H. -Bcmake-build -DBUILD_SHARED_LIBS=NO -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../poco-static -DENABLE_DATA_SQLITE=ON -DENABLE_ACTIVERECORD=OFF -DENABLE_MONGODB=OFF -DENABLE_DATA_ODBC=OFF -DENABLE_REDIS=OFF $1 $2 $3 $4 $5
cmake --build cmake-build --target all -- -j
cmake --build cmake-build --target install
