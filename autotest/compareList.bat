@echo off
::you may use any other compare-tool!

set src="%~dp0\ref_list"
set dst="%~dp0\work\list"

where winmerge >nul 2>&1
if %errorlevel% == 0 call winmerge >nul %src% %dst% & goto end
where diff >nul 2>&1
if %errorlevel% == 0 call diff -c3 -p %src% %dst%   & goto end
::use fc
fc %src% %dst% & goto end

:end
set src=
set dst=