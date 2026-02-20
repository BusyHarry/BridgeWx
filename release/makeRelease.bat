@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

pushd .
cd %~dp0

:: first (optional) param = version like v10.0.0
echo version=%1

:: set environment var 'wxwin' for wxWidgets base to get the wx internal translations
set wxwin=
call SetWxWin.bat

set version=
set zipfile=BridgeWxBin
del %zipfile%*.zip >nul 2>&1

if not [%1] == [] set version=_%1
set zipfile=%zipfile%%version%.zip

:: Assumption: you have build the non-dll release version of BridgeWx.exe

:: create needed folders

if not exist .\BridgeWx\locales\nl mkdir .\BridgeWx\locales\nl
if not exist .\BridgeWx\tools      mkdir .\BridgeWx\tools


:: delete old files, if there
echo Y | del /s .\BridgeWx\* >nul 2>&1

:: copy non-dll release to destination
copy ..\vc_x64_mswu\BridgeWx.exe .\BridgeWx\

:: copy translations for each language
:: English, remark: no wx locale needed, default is English, rest of texts: buildin

:: Dutch
if not [%wxwin%] == [] copy %wxwin%\locale\nl.mo .\BridgeWx\locales\nl\wxstd.mo
copy ..\locales\nl.mo .\BridgeWx\locales\nl\BridgeWx.mo


set tools=AutoHotkey64,BuildDate,cleanpo,msgbox,zip,shell_sqlite
for %%a in (%tools%) copy ..\tools\%%a.exe BridgeWx\tools\

:: copy vc-redist dll's
set vc_redistribution=vc_redist
if exist %vc_redistribution% copy %vc_redistribution%\*.dll BridgeWx

:: update release notes
notepad .\releasenotes.md
copy    .\releasenotes.md BridgeWx\

..\tools\zip.exe -r -9 %zipfile% BridgeWx\*

set version=
set zipfile=
set wxwin=
set vc_redistribution=

popd
