#Requires AutoHotkey v2.0
#SingleInstance
; BusyHarry 20240727
; Create working dir, clear it and produce the <AutoTest.pos> file
;
; -b as first argument sets butler-calculation
; (any) other first/second argument of script: DEBUG_EXE will be set to true
;
DEBUG_EXE := false
BUTLER    := ""  ; false  ; default % calculation

if (A_Args.Length > 0)
{
  if A_Args[1] == "-b"
  {
    BUTLER := " -b1 " ; true
    if (A_Args.Length > 1)
      	DEBUG_EXE := true
  }
  else
    DEBUG_EXE := true
}
;
Exename       := "BridgeWx.exe"
DEBUG         := DEBUG_EXE ? "d" : ""
BridgeWxExe   := A_ScriptDir "\..\vc_x64_mswu" DEBUG "\" Exename
AutoTestPath  := A_ScriptDir "\work\"
AutoTestMatch := "autoTest01"
AutoTestCmd   := " -u -r1 -g1 -k0 -l61 -w " AutoTestMatch " -f " AutoTestPath BUTLER
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
ProcessClose(Exename)  ; kill process if running: we don't know if its params are correct
sleep(100)             ; sometimes, process still exists, and next line fails (file 'list' still held by process)
FileDelete(AutoTestPath "\*")  ; delete all (old) files in the work directory
run StartBridgeWx
sleep(1200)
SendEvent "!+a"	; hotkey for generating mouse-positions/labels of edit-targets
sleep(1500)   	; give it time to produce wanted file
ProcessClose(Exename)
sleep(100)         ; be 'sure' process has stopped...
FileDelete(AutoTestPath AutoTestMatch ".*")
ExitApp(0)
