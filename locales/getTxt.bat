@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

:: extract translatable texts from the .cpp and .h sources to a .pot file

:: check if running 4dos, if so, restart batchfile in cmd.exe
set test_4dos=%@EVAL[1]
if [1] == [%test_4dos%] echo 4dos running, re-starting cmd & set test_4dos=& cmd.exe /c %0 %* & exit /b

:: all commandline params are handed over to xgettext.exe

pushd .
cd %~dp0

set base=Program Files (x86)\Poedit
set base64=Program Files\Poedit
set pExe=Poedit.exe
set drive=

set all_drives=C:,D:,E:,F:

::test first if x64 version of Poedit exists
for %%a in (%all_drives%) do (
  if exist "%%a\%base64%\%pExe%" set base=%base64%
)

for %%a in (%all_drives%) do (
  if exist "%%a\%base%\%pExe%" (echo Poedit     found on drive: '%%a' & set drive=%%a& goto found)
  echo Poedit not found on drive: '%%a'
)

call ..\tools\msgbox.exe Warning "in %~dpf0" "Can't find Poedit installation on %all_drives%"
goto end

:found
set gt=%drive%\%base%\GettextTools\bin\xgettext.exe

if exist "%gt%" goto ok
  call ..\tools\msgbox.exe Warning "in %~dpf0" "not found '%gt%', exiting"
  goto end

:ok
set gt="%gt%"  %*
set potfile=BridgeWx.pot
del %potfile% >nul 2>&1
echo Getting translatable text through %gt%
:: search for strings marked with _() in all .cpp and .h files in directory ..\src
:: the generated file is called '%potFile%' and is located in the current folder

set errorlog=error.log
%gt% 2>%errorlog% --no-wrap --sort-by-file --from-code=UTF-8 --default-domain=BridgeWx --keyword=_ --output=%potfile% --add-comments=TRANSLATORS ..\src\*.cpp ..\src\*.h
if not exist %potfile% goto error
  echo %potfile% generated, starting Poedit
  call "%drive%\%base%\%pExe%"
  goto end

:error
  set param1=
  set param2=
  for /f "tokens=*" %%a in (%errorlog%) do (set param=%%a & call :GetError)
  call ..\tools\msgbox.exe "Error!" "%potfile% NOT created:" "" "%param1%" "%param2%"
  set param1=
  set param2=
  goto end

:GetError
  :: for param1 remove .exe part from errormessage
  :: for param2 remove leading spaces
  if [%param1%] == [] (set param1=%param:*: =% & exit /b)
  if [%param2%] == [] (for /f "tokens=* delims= " %%a in ("%param%") do set param=%%a) & set param2=%param%
  exit /b

:end
del >nul 2>&1 %errorlog%
set         gt=
set       base=
set       pExe=
set      drive=
set     base64=
set    potfile=
set   errorlog=
set  test_4dos=
set all_drives=
popd
