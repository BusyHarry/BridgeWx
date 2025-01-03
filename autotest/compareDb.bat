@echo off
::you may use any other compare-tool!
:: if a parameter is present, its interpreted as a language-type
:: default: en
set language=en
if not [%1] == [] set language=%1
set src="%~dp0\ref_autoTest01.db.%language%"
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
