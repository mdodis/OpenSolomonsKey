@echo off

set opts=-FC -GR- -EHa- -nologo -Zi -Od
set link_opts=opengl32.lib User32.lib Gdi32.lib -SUBSYSTEM:WINDOWS
set code=%cd%
pushd build
cl %opts% %code%\win32_OpenSolomonsKey.cpp -Fesolomons_key /link %link_opts%
popd
