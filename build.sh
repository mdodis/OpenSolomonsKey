#!/bin/bash

TIMEFORMAT=%R

code="$PWD/src/"
opts="-pthread -Wno-write-strings -I$code/../"
libs="-lGL -lGLU -lGLX -lX11"
opt_debug="-g -ggdb -O0"
opt_release="-O3 -DNDEBUG"
cd build > /dev/null

# Debug
time g++ $opts $opt_debug $code/x11_OpenSolomonsKey.cpp $libs -lasound -o solomons_key

# Release
#time g++ $opts $opt_release $code/x11_OpenSolomonsKey.cpp -lasound -lasound /home/miked/Desktop/portaudio/lib/.libs/libportaudio.a -o solomons_key
cd $code > /dev/null
