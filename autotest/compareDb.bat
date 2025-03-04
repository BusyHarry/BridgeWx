@echo off
::you may use any other compare-tool!
:: if first/second parameter == -b, then butler-score is set
:: any other param is interpreted as a language-type
:: default language   : en
:: default calculation: %, i.e. non-butler

set language=en
set   butler=

:handleArgs
if [%1] == [] goto done
  if [%1] == [-b] (set butler=.butler) else set language=%1
  shift
  goto handleArgs

:done
set src="%~dp0\ref_autoTest01.db.%language%%butler%"
set dst="%~dp0\work\autoTest01.db"

where winmerge >nul 2>&1
if %errorlevel% == 0 call winmerge >nul %src% %dst% & goto end
where diff >nul 2>&1
if %errorlevel% == 0 call diff -c3 -p %src% %dst%   & goto end
::use fc
fc %src% %dst% & goto end

:end
set src=
set dst=
set language=
set butler=
