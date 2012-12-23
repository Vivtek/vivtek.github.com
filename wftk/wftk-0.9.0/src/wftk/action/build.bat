rem @echo off
perl ..\..\lpml.pl ACTION_wftk.xml
perl ..\..\lpml.pl ACTION_system.xml
call ..\..\msvc.bat
cl /c /nologo /DDEBUG /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoACTION_wftk.obj ACTION_wftk.c > errors.txt
cl /c /nologo /DDEBUG /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoACTION_system.obj ACTION_system.c >> errors.txt
notepad errors.txt
