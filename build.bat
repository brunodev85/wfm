if exist wfm.exe del wfm.exe
if exist obj del /s /q obj
set PATH=C:\w64devkit\bin
make -j8
start wfm.exe
pause