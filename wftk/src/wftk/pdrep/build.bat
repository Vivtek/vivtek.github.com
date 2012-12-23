rem @echo off
perl ..\..\lpml.pl PDREP_localxml.xml
call ..\..\msvc.bat
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoPDREP_localxml.obj PDREP_localxml.c > errors.txt
notepad errors.txt
