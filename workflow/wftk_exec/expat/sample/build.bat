@echo off
set LIB=e:\devstudio\lib
set LIB=..\xmlparse\Release;..\lib;%LIB%
set PATH=e:\devstudio\bin;e:\devstudio\sharedide\bin;%PATH%
set INCLUDE=e:\devstudio\include
cl /nologo /DXMLTOKAPI=__declspec(dllimport) /DXMLPARSEAPI=__declspec(dllimport) /I..\xmlparse /Fe..\bin\elements elements.c xmlparse.lib
@echo Run it using: ..\bin\elements ^<..\expat.html
