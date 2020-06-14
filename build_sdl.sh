#!/bin/bash

TIMEFORMAT=%R

code="$PWD/src/"
opts="-fPIE -pthread -Wno-write-strings -I$code/../"
libs="-lSDL2 -lGL -lGLU"
opt_debug="-g -ggdb -O0"
opt_release="-O3 -DNDEBUG"
cd build > /dev/null

# Debug
time g++ $opts $opt_debug $code/sdl_OpenSolomonsKey.cpp $libs -lasound -o sdl_solomons_key

# # Release
# time g++ $opts $opt_release $code/x11_OpenSolomonsKey.cpp $libs -lasound -o sdl_solomons_key
cd $code > /dev/null
