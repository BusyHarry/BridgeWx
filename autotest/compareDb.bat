@echo off
::you may use any other compare-tool!
:: if any parameter == -b, then butler-score is set
:: if any parameter == -q, then SQLite storage is set
:: any other param is interpreted as a language-type
:: default language   : en
:: default storage    : db, i.e. non-SQLite
:: default calculation:  %, i.e. non-butler

set language=en
set       db=db
set   butler=

rem try to set db type automatically, preference for .db
if exist "%~dp0\work\autoTest01.sqlite" set db=sq
if exist "%~dp0\work\autoTest01.db"     set db=db

:handleArgs
if [%1] == [] goto done
  if [%1] == [-b] (set butler=.butler) & shift & goto handleArgs
  if [%1] == [-q] (set     db=sq)      & shift & goto handleArgs
  set language=%1
  shift
  goto handleArgs

:done
if %db% == sq (
  set src="%~dp0\ref_autoTest01.sq.%language%%butler%"
  set dst="%~dp0\work\autoTest01.sqlite"
  rem now get content of db's as text in order to compare
  rem create input-redirection for ..\tools\shell_sqlite.exe to dump the db as text
  set tmp1=%src%.txt
  set tmp2=%dst%.txt
  set sqlcmd=sqldump.cmds
  echo .dump  >%sqlcmd%
  echo .quit >>%sqlcmd%
  ..\tools\shell_sqlite.exe %src% < %sqlcmd% >%tmp1%
  ..\tools\shell_sqlite.exe %dst% < %sqlcmd% >%tmp2%
  set src=%tmp1%
  set dst=%tmp2%
) else (
  set src="%~dp0\ref_autoTest01.db.%language%%butler%"
  set dst="%~dp0\work\autoTest01.db"
)

where winmerge >nul 2>&1
if %errorlevel% == 0 call winmerge >nul %src% %dst% & goto end
where diff >nul 2>&1
if %errorlevel% == 0 call diff -c3 -p %src% %dst%   & goto end
::use fc
fc %src% %dst% & goto end

:end
del %sqlcmd% >nul 2>&1
del %tmp1%   >nul 2>&1
del %tmp2%   >nul 2>&1

set      src=
set      dst=
set language=
set   butler=
set     tmp1=
set     tmp2=
