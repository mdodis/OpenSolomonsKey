#!/bin/bash

code="$PWD/src/"
opts="-g -lGLEW -lGL -lGLU -lX11 -O0 -no-pie -I$code/../"
cd build > /dev/null
g++ $opts $code/x11_OpenSolomonsKey.cpp -o solomons_key
cd $code > /dev/null
