@echo off
:: Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
:: Distributed under the MIT License (http://opensource.org/licenses/MIT)

echo deleting old log-files...
if exist diffiles_*.log del diffiles_*.log
if exist logall.log     del logall.log
if exist diflog.log     del diflog.log
if exist DifMissing.log del DifMissing.log