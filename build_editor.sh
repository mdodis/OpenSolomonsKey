#!/bin/bash

TIMEFORMAT=%R

code="$PWD/editor/"
opts="-pthread -I$code/imgui/ -I./incl/ -Wno-conversion-null"
libs="-lGL -lGLU -lsfml-graphics -lsfml-window -lsfml-system"
SOURCES="$code/imgui/imgui.cpp $code/imgui/imgui_draw.cpp $code/imgui/imgui_widgets.cpp $code/imgui/imgui-SFML.cpp $code/imgui/imgui_demo.cpp $code/editor.cpp"
cd build

# Debug
time g++ $opts $SOURCES $libs -o osked

cd $code
