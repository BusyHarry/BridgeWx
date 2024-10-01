@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

:: compare all interesting files in current folder(s) to %cmpTarget%
:: %cmpTarget% is used in CompareCommon.bat
set target=%cmpTarget%
if [%cmpTarget%] == [] set cmpTarget=D:\gitTestBase\BridgeWx

cd %~dp0
call delCompareLogs.bat  ::remove old logs to prevent confusion
set        logall=.\logall.log
set DifMissing=.\DifMissing.log

for %%a in (autotest,locales,main,resource,src,tools) do (
   echo.
   pushd .
   echo calling CompareCommon.bat %%a %logall%
   echo comparing '%%a' >>%DifMissing%
   call CompareCommon.bat %%a "%logall%"
   popd
   )

::if exist "%logall%" list "%logall%" else echo No diffs found!
set list=list.exe
where list.exe >nul 2>&1
if NOT %errorlevel% == 0 set list=notepad.exe
if exist "%logall%" (%list% "%logall%") else echo no diffs found
set list=



set  cmpTarget=%target%
set     target=
set     logall=
set DifMissing=
