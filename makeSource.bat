@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

pushd .
cd %~dp0

:: param 1: version
echo version=%1
set version=
set zipfile=.\release\BridgeWxSrc
del %zipfile%*.zip >nul 2>&1

if not [%1] == [] set version=_%1
set zipfile=%zipfile%%version%.zip
zip -r -S -9 %zipfile% * -x \*.log \*.zip* *.exe *.pot \BridgeWx_*_suppressions.cfg BridgeWx_solution_suppressions.cfg  *.vcxproj.user release\BridgeWx\* .git\* .vs\* *\work\* vc_x64_msw* *.mo

set version=
set zipfile=
popd
