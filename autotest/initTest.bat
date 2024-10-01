@echo off
pushd .
rem goto batch folder
cd %~dp0
rem if any param given, the debug version of BridgeWx.exe will be used
if not [%1] == [] echo Running the debug version of BridgeWx.exe
if     [%1] == [] echo Running the release version of BridgeWx.exe

set ahkfolder=
where AutoHotkey64.exe >nul 2>&1
if %errorlevel% == 0 goto run_it
if not exist ..\tools\AutoHotkey64.exe goto error
set ahkfolder=..\tools\

:run_it
  start /wait %ahkfolder%AutoHotkey64.exe InitTest.ahk %1
  if %errorlevel% == 0 ECHO mousepositions/labels generated
  goto end

:error
  echo error finding autohotkey64.exe
  goto end

:end
popd
