rem @echo off
perl ..\..\lpml.pl PERMS_localxml.xml
call ..\..\msvc.bat
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoPERMS_localxml.obj PERMS_localxml.c > errors.txt
notepad errors.txt
