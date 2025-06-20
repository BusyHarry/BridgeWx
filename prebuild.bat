@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

:: called from 'Pre-Build Event' in VisualStudio
:: param 1 = $(Configuration) == '[DLL ]Debug' or '[DLL ]Release'
:: param 2 = $(OutDir)        == directory for executable
:: param 3 = $(wxwin)         == path to wxWidgets installation, SHOULD NOT be empty
echo PreBuild.bat: Debug/Release = '%~1', outdir = '%~2', wxwin = '%~3'

SETLOCAL EnableExtensions

if not [%3] == [] if exist "%~3\locale" goto OkWxwin
  .\tools\msgbox.exe ERROR! "variable 'wxwin' ('%~3') is empty or does not exist" "" "its needed for building!" "use environment variables or BridgeWx.props"
  exit /B 1
:OkWxwin

:: remove double quotes around params
set p1=%~1
set p2=%~2
set p3=%~3

set builddateLanguage=en-GB
set msgbox=.\tools\msgbox.exe
set language=nl
call :setupLanguage

::handle more languages, if wanted/needed
::english is buildin in BridgeWx and wxWidgets itself
::set language=xy
::call :setupLanguage

goto builddate

:setupLanguage
  :: input = %language%, %p1%, %p2% and %p3%
  :: create needed folders and copy translation files

  if not exist "%p2%\locales"            mkdir "%p2%\locales"
  if not exist "%p2%\locales\%language%" mkdir "%p2%\locales\%language%"

  ::copy translation file
  set srcMo=".\locales\%language%.mo"
  set dstMo="%p2%\locales\%language%\BridgeWx.mo"
  if not exist %srcMo% (
     %msgbox% Message "please compile %srcMo:.mo=.po%" "or copy %language%\BridgeWx.mo from BridgeWxBin_v*.zip to %dstMo%"
     exit /b
  )
  if not exist %dstMo% echo copying %srcMo% to %dstMo%
  echo F|xcopy /D /Y %srcMo% %dstMo% >nul

  :: now copy wxstd if present
  set srcStdMo="%p3%\locale\%language%.mo"
  if not exist %srcStdMo% (
    %msgbox% Warning "Strange, %srcStdMo% does not exist!"
    exit /b
  )
  set dstStdMo="%p2%\locales\%language%\wxstd.mo"
  if not exist %dstStdMo% echo copying %srcStdMo% to %dstStdMo%
  echo F|xcopy /D /Y %srcStdMo% %dstStdMo% >nul
  exit /b
::end of :setupLanguage()

:builddate
:: now create the builddate.h if we have a release version

if /I [%p1%] == [Debug]     goto DoneBuildDate
if /I [%p1%] == [DLL Debug] goto DoneBuildDate
  echo creating .\src\builddate.h
  .\tools\builddate.exe %builddateLanguage% > .\src\buildDate.h
:DoneBuildDate

set       p1=
set       p2=
set       p3=
set    dstMo=
set    srcMo=
set   msgbox=
set dstStdMo=
set srcStdMo=
set language=

echo prebuild done!
exit /B 0