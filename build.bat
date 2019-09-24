@echo off

if not exist build mkdir build

set incl=%cd%\incl
set opts=-FC -GR- -EHsc -nologo -Zi -Od  -MTd -std:c++latest -I"%incl%"
set code=%cd%\src
set link_opts=opengl32.lib User32.lib Gdi32.lib Winmm.lib Advapi32.lib -SUBSYSTEM:WINDOWS
pushd build
cl %opts% -I%incl% -I..\ %code%\win32_OpenSolomonsKey.cpp -Fesolomons_key /link  %link_opts%
popd
