@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

:: compare work-sources in this folder to another set of sources
:: %1 is the type to compare, if its a folder, its content is compared
:: if %2 is not empty, it is interpreted as the global running logfile
:: environment var %cmpTarget% (if set) denotes the root to compare to. Default=D:\gitTestBase\BridgeWx
:: FOR NOW: please don't use folders with spaces

if [%1] == [] (echo missing source & exit /b)

:: check if running 4dos, if so, restart batchfile in cmd.exe
:: 4dos has problems reading lines from files containing a % character
set test_4dos=%@EVAL[1]
if [1] == [%test_4dos%] echo 4dos running, re-starting cmd & cmd.exe /c %0 %* & exit /b

pushd .
set  batchDir=%~dp0
set  batchDir=%batchDir:~0,-1%
set sourceDir=%batchDir%
set       src=%1
set    subDir=.

cd %batchDir%
if exist %batchDir%\%src% set subDir=%src%
set sourceDir=%batchDir%\%subDir%
set textFile=%sourceDir%\files_%src%.txt
if not exist %textFile% (echo missing '%textFile%', exiting & goto end)

if [%cmpTarget%] == [] (set targetDir=D:\gitTestBase\BridgeWx\%subDir%) else set targetDir=%cmpTarget%\%subDir%

::echo sourceDir='%sourceDir%'
::echo targetDir='%targetDir%'

::next cmds  MUST be on separate line, else [log] value not set YET!
if     [%2]           == [] set log=.\diflog.log
if     [%2]           == [] del %log% >nul 2>&1
if NOT [%2]           == [] set log=%2
if     [%DifMissing%] == [] del .\DifMissing.log >nul 2>&1
if     [%DifMissing%] == [] set DifMissing=.\DifMissing.log
set diffiles=.\diffiles_%src%.log
echo ;list of files that have differences from "%sourceDir%" and '%targetDir%' > %diffiles%

echo comparing files from "%sourceDir%" and "%targetDir%"
for /F %%a in (%textFile%) do (
  rem use of env variable to prevent problems if param has a space in it!
  set file=%%a
  set poFile=%%a

  :: cleanup po/.pot files, will create 2 temp-files
  for %%b in (%%a) do if [.pot] == [%%~xb] call :cleanPoFile
  for %%b in (%%a) do if [.po]  == [%%~xb] call :cleanPoFile
  call :compare
)

:: after comparing all, remove tempfiles and show log if no main log requested
del %sourceDir%\*.po*.tmp %targetDir%\*.po*.tmp >nul 2>&1
set list=list.exe
where list.exe >nul 2>&1
if NOT %errorlevel% == 0 set list=notepad.exe
if [%2] == [] if exist %log% (%list% %log%) else echo no dif found
set list=
goto end

:: used subroutines
:compare
  echo comparing %file%
  if not exist "%sourceDir%\%file%" echo missing  : "%sourceDir%\%file%">> %DifMissing%
  if not exist "%targetDir%\%file%" echo missing  : "%targetDir%\%file%">> %DifMissing%

  if not exist "%sourceDir%\%file%" echo missing file: "%sourceDir%\%file%">> %log% & echo.>>%log% & exit /b
  if not exist "%targetDir%\%file%" echo missing file: "%targetDir%\%file%">> %log% & echo.>>%log% & exit /b

  if not [%file:.exe=%] == [%file%] exit /b & rem don't compare .exe file
  if not [%file:.jpg=%] == [%file%] exit /b & rem don't compare .jpg file
  if not [%file:.ico=%] == [%file%] exit /b & rem don't compare .ico file
  if not [%file:.mo=%]  == [%file%] exit /b & rem don't compare .mo  file

  fc "%sourceDir%\%file%" "%targetDir%\%file%" >nul
  if %errorlevel% == 0 exit /b
  echo differing: "%sourceDir%\%file%" >> %DifMissing%
  echo %sourceDir%\%file% >> %diffiles%
  echo %sourceDir%\%file% >> %log%"
  fc "%sourceDir%\%file%" "%targetDir%\%file%" >> %log%
  exit /b

:cleanPoFile
  :: remove all lines starting with '#' (most containing lineinfo) which disturb comparision
  :: create tempfiles "%sourceDir%\%file%.tmp" for comparing in current folder and "%targetDir%" folder

  if not exist "%sourceDir%\%file%" exit /b
  if not exist "%targetDir%\%file%" exit /b
  echo calling cleanPoFile for '%file%'
  ::old: echo. >"%sourceDir%\%file%.tmp"
  ::old: echo. >"%targetDir%\%file%.tmp"
  ::new: use a tool to clean the language files, 100 times faster!
  .\tools\cleanpo.exe "%sourceDir%\%file%" > "%sourceDir%\%file%.tmp"
  .\tools\cleanpo.exe "%targetDir%\%file%" > "%targetDir%\%file%.tmp"

  ::old:  for /f "eol=# tokens=*" %%c in (%sourceDir%\%file%) do call :RemoveDateSrc %%c 
  ::old:  for /f "eol=# tokens=*" %%c in (%targetDir%\%file%) do call :RemoveDateTgt %%c
  set file=%file%.tmp
  exit /b

:RemoveDateSrc
  :: only append %1 to outputfile if it does NOT contain "(Creat)ion/(Revis)ion-Date: " 
  echo %1| findstr /c:"ion-Date: " >nul
  if %errorlevel% == 1 echo %1 >>"%sourceDir%\%file%.tmp"
  exit /b

:RemoveDateTgt
  :: only append %1 to outputfile if it does NOT contain "(Creat)ion/(Revis)ion-Date: " 
  echo %1| findstr /c:"ion-Date: " >nul
  if %errorlevel% == 1 echo %1 >>"%targetDir%\%file%.tmp"
  exit /b

:end
:: remove used variables
set       log=
set       src=
set      file=
set    poFile=
set    subDir=
set sourceDir=
set  batchDir=
set  diffiles=
set targetDir=
set test_4dos=
popd
