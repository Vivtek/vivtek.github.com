rem @echo off
perl ..\..\lpml.pl USER_localxml.xml
call ..\..\msvc.bat
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoUSER_localxml.obj USER_localxml.c > errors.txt
notepad errors.txt
