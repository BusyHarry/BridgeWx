@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

:: called from 'Pre-Build Event' in VisualStudio
:: param 1 = $(Configuration) == '[DLL ]Debug' or '[DLL ]Release'
:: param 2 = $(OutDir)        == directory for executable
:: param 3 = $(wxwin)         == path to wxWidgets installation, SHOULD NOT be empty
echo PreBuild.bat: Debug/Release = '%1', outdir = '%2', wxwin = '%3'

SETLOCAL EnableExtensions

if not [%3] == [] if exist %3\locale goto OkWxwin
  .\tools\msgbox.exe ERROR! "environment var wxwin '%3' is empty or does not exist" "" "its needed for building!"
  exit /B 1
:OkWxwin

::echo creating folders
if not exist %2\locales    mkdir %2\locales
if not exist %2\locales\nl mkdir %2\locales\nl
if not exist %2\locales\en mkdir %2\locales\en

set msgbox=.\tools\msgbox.exe
set srcEnMo=.\locales\en.mo
set dstEnMo=%2\locales\en\BridgeWx.mo
if exist %dstEnMo% goto OkEnMo
  echo copying '%srcEnMo%' to '%dstEnMo%'
  if not exist %srcEnMo% %msgbox% Message "please compile %srcEnMo:.mo=.po%" "or copy en\BridgeWx.mo from BridgeWxBin_v*.zip to %dstEnMo%" & goto OkEnMo
  echo F|xcopy /D /Y %srcEnMo% %dstEnMo% >nul
:OkEnMo

set srcNlMo=%3\locale\nl.mo
set dstNlMo=%2\locales\nl\wxstd.mo
if not exist %dstNlMo% echo copying '%srcNlMo%' to '%dstNlMo%'
  if not exist %srcNlMo% %msgbox% Warning "Strange, '%srcNlMo%' does not exist!" & goto OkNlMo
  echo F|xcopy /D /Y %srcNlMo% %dstNlMo% >nul
:OkNlMo

:: now create the builddate.h if we have a release version

if /I [%1] == [Debug]     goto OkBuildDate
if /I [%1] == [DLL Debug] goto OkBuildDate
  echo creating .\src\builddate.h
  .\tools\builddate.exe > .\src\buildDate.h
:OkBuildDate

set srcEnMo=
set dstEnMo=
set srcNlMo=
set dstNlMo=
set  msgbox=

echo prebuild done!
exit /B 0