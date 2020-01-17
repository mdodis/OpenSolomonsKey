#!/bin/bash

TIMEFORMAT=%R

code="$PWD/src/"
opts="-lGL -lGLU -lX11 -pthread -no-pie -Wno-write-strings -I$code/../"
opt_debug="-g -ggdb -O0"
opt_release="-O3 -DNDEBUG"
cd build > /dev/null

# Debug
time g++ $opts $opt_debug $code/x11_OpenSolomonsKey.cpp -lasound -o solomons_key

# Release
#time g++ $opts $opt_release $code/x11_OpenSolomonsKey.cpp -lasound -ljack /home/miked/Desktop/portaudio/lib/.libs/libportaudio.a -o solomons_key
cd $code > /dev/null
