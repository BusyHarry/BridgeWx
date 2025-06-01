@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

:: generate outputfiles without location info (tend to generate lots of diffs)
pushd .
cd %~dp0
call getTxt.bat --no-location
popd
