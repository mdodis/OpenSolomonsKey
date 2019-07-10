#!/bin/bash

code="$PWD"
opts="-g -lGL -lGLU -lX11 -no-pie"
cd build > /dev/null
g++ $opts $code/x11_OpenSolomonsKey.cpp -o solomons_key
cd $code > /dev/null
