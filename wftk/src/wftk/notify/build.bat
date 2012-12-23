rem @echo off
perl ..\..\lpml.pl NOTIFY_smtp.xml
call ..\..\msvc.bat
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /DWINDOWS /FoNOTIFY_smtp.obj NOTIFY_smtp.c > errors.txt
notepad errors.txt
