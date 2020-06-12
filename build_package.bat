@echo off

if not exist dist mkdir dist

call build_clean
xcopy build\solomons_key.exe dist\ /Y
xcopy *.osk dist\ /Y
xcopy res dist\res\ /E /Y
set CURRENT_DATE=%DATE:~-4%-%DATE:~-10,2%-%DATE:~-7,2%
echo 7z a -bt -mx9 -mmt2 -r -- "osk-%CURRENT_DATE%.7z" dist
rmdir /S /Q dist
