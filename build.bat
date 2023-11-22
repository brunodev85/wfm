if exist debug.txt del debug.txt
if exist wfm.exe del wfm.exe
mingw32-make.exe all
wfm.exe
pause