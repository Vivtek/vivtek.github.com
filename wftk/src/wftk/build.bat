rem @echo off
perl ..\lpml.pl wftk_util.xml
perl ..\lpml.pl wftk_config.xml
perl ..\lpml.pl wftk_lib.xml
perl ..\lpml.pl wftk_adaptors.xml

call ..\msvc.bat
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /Fowftk_adaptors.obj wftk_adaptors.c > errors.txt
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /Foconfig.obj config.c >> errors.txt
cl /c /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /Fowftk_lib.obj wftk_lib.c >> errors.txt
lib /nologo /OUT:wftk.lib wftk_lib.obj config.obj wftk_adaptors.obj dsrep/DSREP_localxml.obj pdrep/PDREP_localxml.obj user/USER_localxml.obj perms/PERMS_localxml.obj taskindex/TASKINDEX_stdout.obj taskindex/TASKINDEX_odbc.obj notify/NOTIFY_smtp.obj action/ACTION_wftk.obj action/ACTION_system.obj datastore/DATASTORE_role.obj datatype/DATATYPE_option.obj >> errors.txt

cl /nologo /I%MSVCPATH%\include /I%WFTKPATH%\include /I. /Fewftk.exe wftk_util.c ..\xmlapi\xmlapi.obj wftk.lib ..\expat\lib\xmlparse.lib odbc32.lib wsock32.lib /link /LIBPATH:%MSVCPATH%\lib  >> errors.txt

notepad errors.txt
