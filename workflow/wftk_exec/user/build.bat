@echo off
perl ..\lpml.pl user.xml
set PATH=c:\progra~1\devstudio\vc\bin;c:\progra~1\devstudio\sharedide\bin;%PATH%
set LIB=c:\progra~1\devstudio\vc\lib;.
set INCLUDE=c:\progra~1\devstudio\vc\include;.
cl /nologo /I. /Feuser.exe main.c user.c xmlapi.c xmlparse.lib > errors.txt
notepad errors.txt
