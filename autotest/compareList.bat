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
set src="%~dp0\ref_list.%db%.%language%%butler%"
set dst="%~dp0\work\list"

where winmerge >nul 2>&1
if %errorlevel% == 0 call winmerge >nul %src% %dst% & goto end
where diff >nul 2>&1
if %errorlevel% == 0 call diff -c3 -p %src% %dst%   & goto end
::use fc
fc %src% %dst% & goto end

:end
set      src=
set      dst=
set language=
set   butler=
set       db=
