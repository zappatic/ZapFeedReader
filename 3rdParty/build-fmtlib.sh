#!/bin/bash

rm -rf build
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../fmtlib-install
cmake --build build --target all -- -j
cmake --build build --target install
