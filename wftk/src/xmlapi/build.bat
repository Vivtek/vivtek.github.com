rem @echo off
perl ..\lpml.pl xmlapi.xml
copy *.h ..\..\include
call ..\msvc.bat
cl /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /Fetester.exe test.c xmlapi.c ..\expat\lib\xmlparse.lib /link /LIBPATH:%MSVCPATH%\lib > errors.txt
notepad errors.txt
