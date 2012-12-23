rem @echo off
perl ..\..\lpml.pl DATASTORE_role.xml
call ..\..\msvc.bat
cl /c /nologo /DDEBUG /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /FoDATASTORE_role.obj DATASTORE_role.c > errors.txt
notepad errors.txt
