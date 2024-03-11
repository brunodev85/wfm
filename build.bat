if exist wfm.exe del wfm.exe
if exist debug.txt del debug.txt
set PATH=C:\w64devkit\bin
make -j8
start wfm.exe
pause