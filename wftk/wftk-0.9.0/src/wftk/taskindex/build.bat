rem @echo off
perl ..\..\lpml.pl TASKINDEX_stdout.xml
perl ..\..\lpml.pl TASKINDEX_odbc.xml
call ..\..\msvc.bat
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoTASKINDEX_stdout.obj TASKINDEX_stdout.c > errors.txt
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoTASKINDEX_odbc.obj TASKINDEX_odbc.c >> errors.txt
notepad errors.txt
