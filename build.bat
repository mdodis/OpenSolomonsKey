@echo off


if not exist build mkdir build

REM NDEBUG to disable drawing collisions

set incl=%cd%\incl
set opts= -GR- -EHsc -nologo -std:c++latest -I"%incl%"
set debug=-Od -MTd -Zi
set release=-O2 -MT -DNDEBUG
set code=%cd%\src
set link_opts=opengl32.lib User32.lib Gdi32.lib Winmm.lib Advapi32.lib -SUBSYSTEM:WINDOWS


pushd build
del build.ctime 2> NUL


ctime -begin build.ctime
REM Debug
cl %opts% %debug%   -I%incl% -I..\ %code%\win32_OpenSolomonsKey.cpp -Fesolomons_key /link   %link_opts%

REM Release
REM cl %opts% %release% -I%incl% -I..\ %code%\win32_OpenSolomonsKey.cpp -Fesolomons_key /link  %link_opts%

ctime -end build.ctime

popd
