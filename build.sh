#!/bin/bash

code="$PWD"
opts="-g -lGL -lGLU -lX11"
cd build > /dev/null
g++ $opts $code/linux_OpenSolomonsKey.cpp -o solomons_key
cd $code > /dev/null
