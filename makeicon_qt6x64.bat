set "cpathvar=%cd%"
call C:\Qt\6.10.2\mingw_64\bin\qtenv2.bat
D:
cd %cpathvar%
C:\Qt\Tools\mingw1120_64\bin\windres gsdocedit.rc -O coff -o gsdocedit64.res