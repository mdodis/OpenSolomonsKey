@echo off
pushd build
del *.obj *.pdb *.exp *.lib *.ilk *.sln *.ctime  2>nul
rmdir /S /Q .vs 2>nul
popd