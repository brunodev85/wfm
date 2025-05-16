if exist wfm.exe del wfm.exe
set PATH=C:\w64devkit\bin
if exist obj del /s /q obj
make -j8
start wfm.exe
pause