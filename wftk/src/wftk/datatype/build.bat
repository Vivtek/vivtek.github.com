rem @echo off
perl ..\..\lpml.pl DATATYPE_option.xml
call ..\..\msvc.bat
cl /c /nologo /DDEBUG /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoDATATYPE_option.obj DATATYPE_option.c > errors.txt
notepad errors.txt
