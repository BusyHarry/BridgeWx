@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

:: set wxwin to the base of your wxWidgets installation

set wxwin=D:\wxWidgets_3.2.7

:: check if path exists, if not: show warning and clear wxwin
if not exist %wxwin%\locale (..\tools\msgbox.exe Warning "Environment variable wxwin='%wxwin%'" "Target does not exist!" "Set correct value in %~f0" & set wxwin=)
