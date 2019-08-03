@echo off

set incl=%cd%\incl
set opts=-FC -GR- -EHsc -nologo -Zi -Od -std:c++latest -I"%incl%"
set code=%cd%\src
set link_dir=%cd%\lib
set link_opts=opengl32.lib User32.lib Gdi32.lib %link_dir%\glew32.lib -SUBSYSTEM:WINDOWS
pushd build
cl %opts% -I%incl% %code%\win32_OpenSolomonsKey.cpp -Fesolomons_key /link  %link_opts%
popd
