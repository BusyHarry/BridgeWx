#Requires AutoHotkey v2.0
#SingleInstance
; BusyHarry 20240727
; Create working dir, clear it and produce the <AutoTest.pos> file
; (any) optional first argument of script: DEBUG_EXE will be set to true
;
DEBUG_EXE := false
if (A_Args.Length > 0)
	DEBUG_EXE := true
;
Exename       := "BridgeWx.exe"
DEBUG         := DEBUG_EXE ? "d" : ""
BridgeWxExe   := A_ScriptDir "\..\vc_x64_mswu" DEBUG "\" Exename
AutoTestPath  := A_ScriptDir "\work\"
AutoTestMatch := "autoTest01"
AutoTestCmd   := " -u -r1 -g1 -k0 -l61 -w " AutoTestMatch " -f " AutoTestPath
StartBridgeWx := BridgeWxExe AutoTestCmd
;
;MsgBox(StartBridgeWx)
if (!FileExist(BridgeWxExe))
{
   MsgBox("Cannot find " BridgeWxExe)
   ExitApp(1)
}

;
if !DirExist(AutoTestPath)
	DirCreate(AutoTestPath)
FileDelete(AutoTestPath "\*")
ProcessClose(Exename)  ; kill process if running: we don't know if its params are correct
run StartBridgeWx
sleep(2000)
SendEvent "!+a"	; hotkey for generating mouse-positions/labels of edit-targets
sleep(3000)   	; give it time to produce wanted file
ProcessClose(Exename)
FileDelete(AutoTestPath AutoTestMatch ".*")
ExitApp(0)
