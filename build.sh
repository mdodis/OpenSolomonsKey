#!/bin/bash

TIMEFORMAT=%R

code="$PWD/src/"
opts="-g -ggdb -lGL -lGLU -lX11 -lportaudio -lpthread -O0 -no-pie -I$code/../"
cd build > /dev/null
time g++ $opts $code/x11_OpenSolomonsKey.cpp -o solomons_key
cd $code > /dev/null
