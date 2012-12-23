rem @echo off
perl ..\..\lpml.pl DSREP_localxml.xml
call ..\..\msvc.bat
cl /c /nologo /DDEBUG /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoDSREP_localxml.obj DSREP_localxml.c > errors.txt
notepad errors.txt
