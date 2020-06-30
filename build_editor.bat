@echo off

if not exist build mkdir build

set code=%cd%\editor
set opts=-EHsc -FC -MP -nologo -W1 -WX
set debug=-Od -Zi
set release=-O2 -DNDEBUG
set incl=%cd%\incl
set libs=%cd%\lib

echo %libs%

pushd build
del build-editor.ctime 2> NUL
ctime -begin build-editor.ctime

set SOURCES=%code%/imgui/imgui.cpp %code%/imgui/imgui_draw.cpp %code%/imgui/imgui_widgets.cpp %code%/imgui/imgui-SFML.cpp %code%/imgui/imgui_demo.cpp %code%/editor.cpp
set link_opts=%libs%\sfml-system.lib %libs%\sfml-graphics.lib %libs%\sfml-main.lib %libs%\sfml-window.lib %libs%\freetype.lib Comdlg32.lib OLE32.lib shell32.lib

REM Debug
cl %opts% -I%incl% -I%code%\..\src\ %debug% %SOURCES% -Fe:osked.exe /link -incremental:no %link_opts% opengl32.lib winmm.lib gdi32.lib 

ctime -end build-editor.ctime
popd

