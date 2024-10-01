@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

:: extract translatable texts from the .cpp and .h sources to a .pot file
pushd .
cd %~dp0

set base=Program Files (x86)\Poedit
set pExe=Poedit.exe
set drive=

set all_drives=C:,D:,E:,F:
for %%a in (%all_drives%) do (
  ::echo testing drive: '%%a'
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
set gt="%gt%"
set potfile=BridgeWx.pot
del %potfile% >nul 2>&1
echo Getting translatable text through %gt%
:: search for strings marked with _() in all .cpp and .h files in directory ..\src
:: the generated file is called '%potFile%' and is located in the current folder

set errorlog=error.log
%gt% 2>%errorlog% --sort-by-file --from-code=UTF-8 --default-domain=BridgeWx --keyword=_ --output=%potfile% --add-comments=TRANSLATORS ..\src\*.cpp ..\src\*.h
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
set errorlog=
set  potfile=
popd
