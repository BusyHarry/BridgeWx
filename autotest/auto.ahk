#Requires AutoHotkey v2.0
#SingleInstance
TESTING_  := 0     ; set to 1 if you want to test something outside of the automatic test, testcode at the end
DEBUG_EXE := false ; default value (false): use the release.exe
;
; (any) optional first argument of script: DEBUG_EXE will be set to true
;
if (A_Args.Length > 0)
  DEBUG_EXE := true

;
; maybe: title not needed on many cmnds: it wil use last active window, should be faster
;
;generated mousepositions/variables/labels
#Include ".\work\AutoTest.pos"      ;include relative to script-location and can NOT be a variable!
;
;assume that InitTest.ahk/bat has been run: actual mousepositions/labels available
;
butlerArg     := bButler ? " -b1 " : ""
Exename       := "BridgeWx.exe"
DEBUG         := DEBUG_EXE ? "d" : ""
BridgeWxExe   := A_ScriptDir    "\..\vc_x64_mswu" DEBUG "\" ExeName
if 1 == InStr(a_ScriptDir, "f:\temp" , , 1, 1)
  BridgeWxExe := "D:\bridge\BridgeWx\vc_x64_mswu" DEBUG "\" ExeName  ; for local testing
AutoTestPath  := A_ScriptDir "\work\"
AutoTestMatch := "autoTest01"
AutoTestCmd   := " -u -d -r1 -g1 -k0 -l61 -w " AutoTestMatch " -f " AutoTestPath butlerArg
StartBridgeWx := BridgeWxExe AutoTestCmd
;MsgBox(StartBridgeWx) ; show command and its params
if (!FileExist(BridgeWxExe))
{
   MsgBox("Cannot find " BridgeWxExe)
   ExitApp(1)
}

BridgewxTitle     := WinTitle   ;"'Bridge' calculation program"
ListFile          := AutoTestPath "list"
VS_DEBUG          := false ;true   ; run under debugger: does not restart app

bShowMouseMove    := 0          ; show mouse moves
NrOfPairs         := 18         ; pairs in each session
NrOfGames         := 28         ; games in each session
timeCounter       := 0          ; timestamp in ms
TIME_INCREMENT    := 15         ; <timeCounter> will increment with this value
MOUSE_DELTA_TIME  := 510        ; minimum time between mouseclicks to prevent doubleclick (default windows: 500ms)
MOUSE_DELTA_POS   := 10         ; minimal offset in x/y mousposition to prevent doubleclick (default windows: 5)
BUSY_DELTA_WAIT   := 10         ; sleep time for testing busy
ROWCOUNT_MC       := 4          ; nr of rows in MC choices
KEYDELAY_DEFAULT  := 2          ; the default delay to use
NEXT_ITEM         := -1         ; Choice*: use this as <itemNr> if you want the next item to be selected, should be much faster
;
; for debugging: global vars
LastMousePosX     := 0
LastMousePosY     := 0
LastWindowName    := ""
SetCheckMouseX    := 0
SetCheckMouseY    := 0
maxCount1         := 0
maxCount2         := 0
; end debugging variables
;
SetControlDelay      -1
SetKeyDelay          KEYDELAY_DEFAULT,-1  ; -1= no delay, 0=smallest possible, second param = keydown time, adapt if you 'loose' characters
SetMouseDelay        0 ; 5  ;25
;SetControlDelay      20                  ; 20=default, -1=noDelay, 0=smallest
SetDefaultMouseSpeed 0 ;1                 ; 0=fastest, 100=slowest
CoordMode            "Mouse" , "Screen"   ;"Window"   ; use screen to ALWAYS address the app, not the editor/debugger: move app to (0,0)
SendMode             "Event"              ; mousemove now also slower!
SetTitleMatchMode    3                    ; exact match, needed for cases like : "search1" and "search12"
;DetectHiddenWindows  true
if winexist(BridgewxTitle)
  MouseOff()

+Esc::ExitApp     ; Exit script with 'shift Escape' key
+Home::Pause(-1)  ; toggle pause with 'shift pause'

if TESTING_
  goto TESTING              ; (temporary testing code)

normalrun:                  ; needed to prevent warning if TESTING
StartTime := A_TickCount
initApplication()           ; start, set dbasetype to .db, stop, delete old-files, restart
InitMatchS1()               ; settings for this match
InitSchema()                ; entry of used schema
InitNamesS1()               ; entry/assignments of names
Scores()                    ; entry of scores
ScoresEditS1()              ; remove/change scores
ScoresCorS1()               ; set corrections for session and total result
CalculateAndDisplayResult() ; get session/total results and display them
;
InitMatchS2()
InitSchema()
InitNamesS2()
Scores()
CalculateAndDisplayResult()
;
InitMatchS3()
InitSchema()
InitNamesS3()
Scores()
CalculateAndDisplayResult()

OutputDebug("Testduration " (A_TickCount - StartTime)//1000 " seconds`n")
ExitApp(0)

; here follow all the function implementations
InitApplication()   ; set databasetype to .db and delete all temp-files
{
  global Printer_ChoicePrinter
  global Printer_ChoicePrinter_L
  if VS_DEBUG
    goto running
  ok:
  ProcessClose(Exename)  ; kill process if running: we don't know if its params are ok
  StartApplication()
  MenuSelect(MenuDbType)
  MenuSelect(MenuShutdown)
  FileDelete AutoTestPath AutoTestMatch ".*" ; delete temp files and 'list'
  if FileExist(ListFile)
     FileDelete ListFile

  running:
  StartApplication()	                        ; restart app, so tests can start
  MenuSelect(MenuPrinter)
    ChoiceSelect(&Printer_ChoicePrinter_L,1)  ; select 'list' as printer
    ;WaitNotBusy()
} ; InitApplication()

StartApplication()
{
  if !VS_DEBUG
  {
    Run           BridgewxExe AutoTestCmd
    Test4Timeout( WinWaitActive(BridgewxTitle,,2))
  }
  WinActivate     BridgewxTitle
  Test4Timeout(   Winwait(BridgewxTitle,,2))
  WinMove(0,0)
  ControlSetChecked(0, SystemComBox, BridgewxTitle) ; reset Busy
} ; StartApplication()

Test4Timeout(hwnd)
{
  if (hwnd == 0)
  {
     MsgBox("App title <" BridgewxTitle "> does not match.`nKeywords initialized for wrong language?")
     ExitApp(1)
  }
} ;  Test4Timeout()

BusyOff()
{
  ControlSetChecked(0, SystemComBox, BridgewxTitle)
  Loop 100
  {
    if !(ControlGetChecked(SystemComBox, BridgewxTitle) || ControlGetChecked(SystemComBox, BridgewxTitle))
    {
      OutputDebug("BusyOff(" A_Index ")`n")
      return
    }
    sleep(0)
  }
} ; BusyOff()

SetBusy()
{ ; old: REMARK: this will move the keyboardfocus to the SystemComBox,
  ; old so use it ONLY when repos the mouse after this!
  if ControlGetChecked(SystemComBox, BridgewxTitle) ;&& ControlGetChecked(SystemComBox, BridgewxTitle)
  {
    OutputDebug("SetBusy(), already set!`n")
    ;status := ControlGetChecked(SystemComBox, BridgewxTitle)
    return
  }
  HWND := ControlGetFocus(BridgewxTitle)
  ControlSetChecked(1, SystemComBox, BridgewxTitle)
  count := 0
  Loop
  { ; ControlSetChecked() is not reliable, so check result
    Sleep(0)
    if ControlGetChecked(SystemComBox, BridgewxTitle) ;&& ControlGetChecked(SystemComBox, BridgewxTitle)
      break
    Sleep(10)
    if (++count == 10)
      Pause()
  }
  if (count > 2)
    aa:=1
  if (HWND != 0)
    ControlFocus(HWND,BridgewxTitle)
} ; SetBusy()

WaitNotBusy(bSetBusy:=false)
{
  static busyCounter := 0
  ++busyCounter
  count := 0
  loop 200
  {
    IsChecked := ControlGetChecked(SystemComBox, BridgewxTitle) || ControlGetChecked(SystemComBox, BridgewxTitle)
    if (IsChecked == 0)
      break
    ++count
    Sleep(BUSY_DELTA_WAIT)
  }
  if count = 200
    OutputDebug("Busy timeout: " busyCounter "`n")
  if (bSetBusy)
    SetBusy()
  return count
} ;  WaitNotBusy()

SetBusyMC()
{ ; will NOT change mousepos
  ControlSetChecked(1, SystemComBoxMC, BridgewxTitle)
  count := 0
  Loop
  {
    Sleep(0)
    if ControlGetChecked(SystemComBoxMC, BridgewxTitle) ;&& ControlGetChecked(SystemComBoxMC, BridgewxTitle)
      break
    if (++count == 10)
      Pause()
    Sleep(10)
  }
} ; SetBusyMC()

WaitPopupMC()
{
  count := 0
  loop 200
  {
    IsChecked := ControlGetChecked(SystemComBoxMC, BridgewxTitle) ;|| ControlGetChecked(SystemComBoxMC, BridgewxTitle)
    if (IsChecked == 0)
      break
    ++count
    Sleep(BUSY_DELTA_WAIT)
  }
  if count = 200
    aa := 1
  return count
} ;  WaitPopupMC()

MenuSelect(a_menu)
{
  bBusy:=true
  if a_menu = MenuDbType || a_menu = MenuOldType
    bBusy:=false  ; this is a radio, if allready active no menutrigger
  if (bBusy)
    SetBusy()
  SendEvent "{ALT}"
  SendEvent(a_menu)
  if a_menu = MenuShutdown
  { ;no ready mark, so just wait a bit
    Sleep(200)
    return
  }
  WaitNotBusy()
  Sleep(50) ; sometimes, new page is not setup yet!
} ; MenuSelect()

AddName(count?)
{
  if not IsSet(count)
    count := 1
  loop count
  {
    SetBusy()
   ; Sleep(50)
    ;ControlFocus(NameEditor_AddName_L,BridgewxTitle)
    ControlClick(NameEditor_AddName_L,BridgewxTitle)
    ;MouseClick(,NameEditor_AddName[1],NameEditor_AddName[2])
    ;Sleep(50)
    WaitNotBusy()
  }
} ; AddName()

MouseMoveSure(pos, isEqual:=false)  ; if called  from Mouse(), then isEqual (position) could be set
{ ; move mouse to <pos> AND wait till it is really there
  static oldX:=0, oldY:=0, oldHwnd:=0
  if isEqual
  {
    oldHwnd:=0
    return
  }
  MouseGetPos(,,,&hwnd,2)          ; get current hWnd
  if  oldX = pos[1] && oldY = pos[2] && oldHwnd = hwnd
    return ; already there!

;  The following is an alternate way to move the mouse cursor that may work better in certain multi-monitor configurations:
;DllCall("SetCursorPos", "int", pos[1], "int", pos[2])  ; The first number is the X-coordinate and the second is the Y (relative to the screen).
;SetMousePos()
  if bShowMouseMove
    MouseMove(pos[1], pos[2])       ; use active mousespeed (SetDefaultMouseSpeed)
  else
    MouseMove(pos[1], pos[2], 0)    ; as fast as possible
  loop 10
  {
    MouseGetPos(&newx,&newY,,&hwnd,2)
    if (newX = pos[1] && newY = pos[2] && oldHwnd != hwnd) ;   oldHwnd != hwnd : hm, sometimes handles are equal, but NOT there positions!
      break
    Sleep(10)
    if A_Index = 10
    {
      MsgBox("positioning mouse(" pos[1] "," pos[2] ") takes more then 100 ms")
    }
    ; WinActivate(BridgewxTitle)     ; if we set a breakpoint on the previous line, we need this on continue script!
    ; MouseMove(pos[1], pos[2], 0)   ;   and this! to get focus and mouse to where it should be.
  }
  oldX:=newX, oldY:=newY, oldHwnd:=hwnd
  ; test Sleep(50) ; ....
} ; MouseMoveSure()

Mouse(pos)
{ ;  do NOT WaitNotBusy(), only caller knows if the busy will be set...
  static oldX := 0, oldY := 0
  x := pos[1], y := pos[2], isEqual := (oldX = x && oldY = y)
  if (isEqual)
    x += MOUSE_DELTA_POS ; crude: prevent double click...

  ;MouseMoveSure(pos, isEqual)
  oldX := x, oldY := y
  MouseClick(, x, y)  ; default 'left' click
  Sleep(50)   ; on return of mouseclick, there is NO garantee that mouse is there already!
} ; Mouse()

MouseOff()
{
;  MouseClick("left",-100,-100)  ; will put mouse at (0,0) ????
  WinGetPos( &x, &y, &w, &h, BridgewxTitle)
  Mousemove(x+1, y+h+1)
  WinActivate   BridgewxTitle   ; but activate our window  again!
  WinWaitActive BridgewxTitle
} ; MouseOff

SetText(pos, text)
{
  Mouse(pos)
  SendEvent "^a" text
  ;WaitNotBusy()
} ; SetText()

SetText_L(label, text)
{
  ControlFocus(label,BridgewxTitle)
  ;SendEvent "^a" text
  ControlSetText(text, label, BridgewxTitle)  ; text invisible in log..
  return
  xx:
  ;ttt := "{CONTROL DOWN}a{CONTROL UP}" text
  ;ControlSend(ttt,label,BridgewxTitle)
  ControlSend("{CONTROL DOWN}a{CONTROL UP}" text ,label,BridgewxTitle)
  ;ControlSend( "^a",label,BridgewxTitle)
 ; sleep(50)
 ; ControlSetText( text,label,BridgewxTitle)
;  ControlClick(label,BridgewxTitle)
  ;sleep(50)
} ; SetText_L()

SetText_LEW(label, text)
{ ; SetBusy(), replace the contents of 'label' with 'text', send {ENTER} and wait for NotBusy()
  SetBusy()
  ControlSend( "{CONTROL DOWN}a{CONTROL UP}" text "{ENTER}",  label, BridgewxTitle)
  WaitNotBusy()
} ;  SetText_LEW()

AddText(text)
{
  SendEvent text
  ;WaitNotBusy()
} ; AddText()

AddText_EW(text)
{ ; add text and {ENTER} and wait till action done
  SetBusy()
  SendEvent text "{ENTER}"
  WaitNotBusy()
} ; AddText_EW()

PrintPage()
{
  MenuSelect(MenuPrintPage)
} ; PrintPage()

ButtonClick(pos)
{
  Mouse(pos)
} ; ButtonClick()

ButtonClick_L(label)
{
  SetBusy()
  ControlClick(label,BridgewxTitle) ;,,,,"NA")
  WaitNotBusy()
} ; ButtonClick_L()

ChoiceSelect(&choice,itemNr)
{ ;select 'itemNr' from choiceselector 'choice'. 'itemNr' is 1 based
  OutputDebug("ChoiceSelect(" choice "," itemNr ") ready in ")
  if !Winactive(BridgewxTitle)
    WinActivate(BridgewxTitle)
  SetBusy()
  if (Type(choice) = "Array")
  { ; convert to windowname on first usage
    ;Mouse(choice)
    MouseMove(choice[1], choice[2], 0)
    Sleep(100)
    MouseGetPos(,,,&choice,0)
    OutputDebug("Array->ClassNN`n")
    ;MouseOff()
  }
  ControlClick(choice,BridgewxTitle)
  Sleep(125)   ; it takes some time for the popup to appear, on slower pc, increase it
  if (itemNr = NEXT_ITEM)
    SendEvent "{DOWN}{ENTER}"
  else
    SendEvent "{HOME}{DOWN " itemNr-1 "}{ENTER}"
  count := WaitNotBusy()
  ;  OutputDebug("choice " itemNr  " ready in : " count*BUSY_DELTA_WAIT "'n")
  OutputDebug(count * BUSY_DELTA_WAIT "ms`n")
  MouseOff()
} ;ChoiceSelect()

ChoiceSelectMC(choice,itemNr)
{ ;select 'itemNr' from choiceselector 'choice'. 'itemNr' is 1 based
  OutputDebug("ChoiceSelectMC(" choice "," itemNr ")`n")
  SetBusyMC()
  SetBusy()
  Mouse(choice)
;  WinWaitNotActive BridgewxTitle ; wait till focus away from mainwindow: popup is active window...
  count1 := WaitPopupMC()  ; wait till popup active
;  OutputDebug("`nchoiceMCPopup ready in : " count1*BUSY_DELTA_WAIT)
;  SendEvent "^{HOME}{DOWN " itemNr-1 "}{ENTER}"
  if (itemNr = NEXT_ITEM)
    SendInput "{DOWN}{ENTER}"
  else
    SendInput "^{HOME}{DOWN " itemNr-1 "}{ENTER}"
  count2 := WaitNotBusy()
;  OutputDebug("`nchoiceMC " itemNr  " ready in : " count2*BUSY_DELTA_WAIT)
  Sleep(50)
  WinWaitActive BridgewxTitle ; wait till focus back to mainwindow, else popup is active window...
;  MouseClick(,1,1)  ; force focus to app window???
;  Mouse(CalcScore_TextSearchEntry) ; move focus away from choice
  Sleep(50)
  OutputDebug("ChoiceSelectMC(" choice "," itemNr "): ready in " count2 * BUSY_DELTA_WAIT "ms`n")

} ;ChoiceSelectMC()

ChoiceSelectMC_L(choice,itemNr)
{ ;select 'itemNr' from choiceselector 'choice'. 'itemNr' is 1 based
  Global ROWCOUNT_MC
  OutputDebug("ChoiceSelectMC_L(" choice "," itemNr "): ready in ")
  SetBusyMC()
  SetBusy()
  ControlClick(choice,BridgewxTitle)
  count1 := WaitPopupMC()  ; wait till popup active
  ;OutputDebug("choiceMCPopup ready in : " count1*BUSY_DELTA_WAIT "`n")
  if (itemNr = NEXT_ITEM)
    SendEvent("{DOWN}{ENTER}")
  else
  {
    row := Mod(itemNr-1, ROWCOUNT_MC)
    col := (itemNr-1) // ROWCOUNT_MC
    SetKeyDelay(-1)
    SendEvent "^{HOME}{RIGHT " col "}{DOWN " row "}{ENTER}"
    SetKeyDelay(KEYDELAY_DEFAULT)
  }
  count2 := WaitNotBusy()
;  OutputDebug("choiceMC " itemNr  " ready in : " count2*BUSY_DELTA_WAIT "`n")
;  Sleep(50)
  WinWaitActive BridgewxTitle ; wait till focus back to mainwindow, else popup is active window...
;  MouseClick(,1,1)  ; force focus to app window???
;  Mouse(CalcScore_TextSearchEntry) ; move focus away from choice
;  Sleep(50)
  OutputDebug(count1*BUSY_DELTA_WAIT ", " count2 * BUSY_DELTA_WAIT "ms`n")

} ;ChoiceSelectMC_L()


GetWindowNameAt(pos)
{ ; get the name of the control at position 'pos'
  MouseGetPos(,,,&hwnd0,2)
  MouseMoveSure(pos)
  Sleep(20)   ; mousemove() does NOT always have the mouse at the wanted location, so just wait a moment
  MouseGetPos(&posX1,&posY2,,&winName, 0)
  MouseGetPos(&posX ,&posY ,,&hwnd , 2)
  if hwnd0 = hwnd
  {
    oldName := ControlGetClassNN(hwnd0)
    newName := ControlGetClassNN(hwnd)
    dummy   :=1  ; set breakpoint here to check
  }
  return ControlGetClassNN(hwnd)
} ; GetWindowNameAt()

GetActiveWindowName()
{ ; window with keyboardfocus
  Sleep(20)  ; just be sure all is settled
  MouseGetPos(,,,&name,0)  ; 0(default) -> window name, 2 -> hwnd
  return name
} ; GetActiveWindowName()

SetCheckBoxValue(id,value)
{ ; set checkbox 'id' to value 'value'. Must be 0, 1 or -1 (toggle)
  WaitNotBusy()
  MouseMoveSure(id)
  loop 10
  {
    MouseGetPos(&setCheckMouseX, &setCheckMouseY, , &windowName, 0)
    if IsSet(windowName)
      break
    Sleep(10)  ; just be sure all is settled
  }
  LastWindowName := windowName
  ControlSetChecked(value,windowName,BridgewxTitle)
  Sleep(50)
} ; SetCheckBoxValue()

GetCheckBoxValue(pos)
{ ; get value of checkbox at 'pos'
  MouseMoveSure(pos)
  Sleep(20)  ; just be sure all is settled
  MouseGetPos(, , , &windowName, 0)
  return ControlGetChecked(windowName,BridgewxTitle)
} ; GetCheckBoxValue()

SetCheckBoxValue_LW(label,value)
{ ;setting a checkbox/radio button with busy/wait
  if value = ControlGetChecked(label, BridgewxTitle)
    return  ;radio's don't react, if already active...
  SetBusy()
  SetCheckBoxValue_L(label,value)
  WaitNotBusy()
} ;  SetCheckBoxValue_LW()

SetCheckBoxValue_L(label,value)
{
  old := ControlGetChecked(label, BridgewxTitle)
  if old = value
    return
  count := 0
  Loop
  { ; function is unreliable: wait till 2 readbacks give same value
    ControlSetChecked(value, label, BridgewxTitle)
    Sleep(0)
    value1 := ControlGetChecked(label, BridgewxTitle)
    value2 := ControlGetChecked(label, BridgewxTitle)
    if value = value1 && value = value2
      break
    if (++count == 10)
      Pause()
  }
} ; SetCheckBoxValue_L()

GetCheckBoxValue_L(label)
{ ; get value of checkbox oflabel 'label'
  return ControlGetChecked(label,BridgewxTitle)
} ; GetCheckBoxValue_L()

InitMatchS1()
{ ; init match for session 1
  MenuSelect(MenuNewMatch)
    SetText_L(Match_Session_L,"1")  ; session 1
    ;ControlSetText("1", Match_Session_L)  ; session 1
    PrintPage()
  MenuSelect(MenuSetupMatch)
    SetText_L(SetupGame_Description_L, "<auto-test, " sSession " 1>")
    ;ControlSetText("<auto-test, " sSession " 1>", SetupGame_Description_L)
    SetCheckBoxValue_L(SetupGame_Neuberg_L,1)
    PrintPage()
} ; InitMatchS1()

InitMatchS2()
{ ; init match for session 2
  MenuSelect(MenuNewMatch)
    SetText_L(Match_Session_L,"2")  ; session 2
  MenuSelect(MenuSetupMatch)
    SetText_L(SetupGame_Description_L, "<auto-test, " sSession " 2>")
    SetCheckBoxValue_L(SetupGame_Neuberg_L,0) ; second session: NO neuberg
} ; InitMatchS2()

InitMatchS3()
{ ; init match for session 3
  MenuSelect(MenuNewMatch)
    SetText_L(Match_Session_L,"3")  ; session 3
  MenuSelect(MenuSetupMatch)
    SetText_L(SetupGame_Description_L, "<auto-test, " sSession " 3>")
    SetCheckBoxValue_L(SetupGame_Neuberg_L,0) ; second session: NO neuberg
} ; InitMatchS3()

InitSchema()
{ ; init schema  for round 1
  global SetupSchema_ChoiceGroup
  global SetupSchema_ChoiceGroup_L
  MenuSelect(MenuSetupSchema)
    SetText_LEW(SetupSchema_Rounds_L         , "7")
    SetText_LEW(SetupSchema_FirstGame_L      , "1")
    SetText_LEW(SetupSchema_SetSize_L        , "4")
    SetText_LEW(SetupSchema_Groups_L         , "2")
    ; init group1
    ChoiceSelect(&SetupSchema_ChoiceGroup_L,1)
    SetText_LEW(SetupSchema_GroupChars_L     , "a")
    SetText_LEW(SetupSchema_Pairs_L          , "8")
    ChoiceSelectMC_L(SetupSchema_ChoiceSchema_L,1)               ; 7multi08
    SetText_LEW(SetupSchema_Absent_L         , "0")
    ; init group 2
    ChoiceSelect(&SetupSchema_ChoiceGroup_L,2)
    SetText_LEW(SetupSchema_GroupChars_L     , "b")
    SetText_LEW(SetupSchema_Pairs_L          , "10")
    ChoiceSelectMC_L(SetupSchema_ChoiceSchema_L,1)                ; 7multi10
    SetText_LEW(SetupSchema_Absent_L         , "8")

    ButtonClick_L(SetupSchema_Ok_L)
    PrintPage()  ; overview of all groups
} ; InitSchema()

InitNamesS1()
{
  MenuSelect(MenuNamesInit)
    AddName(NrOfPairs+1)
    GridPosition(NameEditor_Grid_L,5,1)
    AddText_EW(sChanged " " sPair " 5") ;SendEvent sChanged " " sPair " 5{ENTER}" ; change name of pair 5
    PrintPage()
  MenuSelect(MenuNamesAssign)
    ButtonClick_L(AssignNames_OnOriginal_L)
    ButtonClick_L(AssignNames_Ok_L)
    PrintPage()
} ; InitNamesS1()

InitNamesS2()
{
  MenuSelect(MenuNamesAssign)
    ButtonClick_L(AssignNames_OnRankPrev_L)
    ; pairnames for current session
    if (bButler)
    {
      GridPosition(AssignNames_Grid_L, 2, 2)
        AddText_EW("a7")        ; p2: ??? -> A7, p17 A7 -> ???
    }
    else
    {
      GridPosition(AssignNames_Grid_L, 5, 2)
        AddText_EW("b10")           ;SendEvent("b10{ENTER}")           ; p5  B2  -> B10, p16: B2 -> ???
        AddText_EW("{ENTER 10}b2")  ;SendEvent("{ENTER 10}b2{ENTER}")  ; p16 B10 -> B2
        AddText_EW("a8")            ;SendEvent("a8{ENTER}")            ; p17 B3  -> A8 , p18: A8 -> ???
        AddText_EW("b3")            ;SendEvent("b3{ENTER}")            ; p18 A8  -> B3
    }

    ButtonClick_L(AssignNames_Ok_L)
    PrintPage()
} ; InitNamesS2()

InitNamesS3()
{
  MenuSelect(MenuNamesAssign)
    ButtonClick_L(AssignNames_OnRankTotal_L)
    GridPosition(AssignNames_Grid_L, 1, 2)    ; pairnames for current session
    ButtonClick_L(AssignNames_Ok_L)
    PrintPage()
} ; InitNamesS3()

SS(score)
{ ; SendScore() : set busybit, send score and wait till score handled
    SetBusy()
    ControlFocus(ScoreEntry_Grid_L,BridgewxTitle)
    Sleep(0)
    SendEvent(ltrim(score) "{ENTER}")
    ;ControlSetText(ltrim(score), ScoreEntry_Grid_L, BridgewxTitle)
    ;SendEvent("{ENTER}")
    WaitNotBusy()
} ; SendScore()

Scores()
{
  global ScoreEntry_ChoiceRound
  global ScoreEntry_ChoiceRound_L
  MenuSelect(MenuScoreEntry)
  SetCheckBoxValue_LW(ScoreEntry_InputOrder0_L,1)  ; set order
  SetCheckBoxValue_LW(ScoreEntry_SlipOrder1_L ,1)  ;  + NS order
  ChoiceSelect(     &ScoreEntry_ChoiceRound_L,1)   ;  + round 1
  ;
  SetKeyDelay(-1)
  SS("   0"), SS("  70"), SS("  70"), SS("  80")  ; 1 a1-2
  SS("  90"), SS(" 100"), SS(" 110"), SS(" 120")  ; 5 a3-4
  SS(" 130"), SS(" 140"), SS(" 150"), SS(" 160")  ; 9 a5-6
  SS(" 170"), SS(" 180"), SS(" 190"), SS(" 200")  ;13 a7-8
  SS(" 210"), SS(" 230"), SS(" 240"), SS(" 260")  ; 1 b1-2
  SS(" 270"), SS(" 340"), SS(" 300"), SS(" 340")  ; 5 b3-4
  SS(" -50"), SS(" -70"), SS(" -80"), SS(" -90")  ; 9 b5-6
  SS("-100"), SS("-110"), SS("-120"), SS("-130")  ;17 b9-10
  SetKeyDelay(KEYDELAY_DEFAULT)
  PrintPage()
  ;
  ChoiceSelect(&ScoreEntry_ChoiceRound_L,2)  ;  round 2
  SetKeyDelay(-1)
  SS(" -140"), SS(" -150"), SS(" -160"), SS(" -170")     ;17 a1-8
  SS(" -180"), SS(" -190"), SS(" -200"), SS(" -210")     ; 9 a2-7
  SS(" -230"), SS(" -240"), SS(" -250"), SS(" -260")     ;25 a5-4
  SS(" -270"), SS(" -280"), SS(" -300"), SS(" -340")     ;13 a6-3
  SS(" -600"), SS(" -800"), SS("-1000"), SS("-1100")     ; 5 b2-9
  SS("-1200"), SS("  300"), SS("  400"), SS("  500")     ;17 b5-4
  SS("  600"), SS("  800"), SS("  120"), SS("  150")     ;13 b6-3
  SS("  110"), SS(" -120"), SS(" -100"), SS("  100")     ;21 b10-7
  SetKeyDelay(KEYDELAY_DEFAULT)
  PrintPage()
  ;
  ChoiceSelect(&ScoreEntry_ChoiceRound_L,3)  ;  round 3
  SetKeyDelay(-1)
  SS("  150"), SS("  150"), SS("  170"), SS(" -170")               ;17 a3-2
  SS("  200"), SS("  300"), SS(" -500"), SS(" -500")               ;13 a4-1
  SS("  160"), SS(" -160"), SS(" -160"), SS(" r150"), SS(" r110")  ;21 a5-8
  SS("  300"), SS(" -600"), SS(" -150"), SS("  %70"), SS("  %70")  ; 5 a6-7
  SS("  -50"), SS("    0"), SS("   90"), SS("  %60"), SS(" r630")  ; 9 b2-7
  SS("  800"), SS("  600"), SS("  500"), SS(" r300"), SS("  %50")  ;13 b5-9
  SS(" -150"), SS(" -800"), SS(" -600"), SS(" -120")               ;17 b6-1
  SS("  150"), SS("  140"), SS("  140"), SS("   50")               ;25 b10-4
  SetKeyDelay(KEYDELAY_DEFAULT)
  PrintPage()
  ;
  ChoiceSelect(&ScoreEntry_ChoiceRound_L,4)  ;  round 4
  SetKeyDelay(-1)
  SS("   50"), SS("  100"), SS(" -200"), SS(" -300")               ;21 a2-4
  SS("  100"), SS("  200"), SS("  300"), SS("  %40"), SS("  %40")  ; 9 a3-1
  SS("   70"), SS("   80"), SS("   90"), SS("  %80"), SS(" r620")  ; 1 a6-8
  SS(" -420"), SS(" -420"), SS("  420"), SS("  620")               ;17 a7-5
  SS("  800"), SS("  200"), SS("  300"), SS("  300")               ;13 b4-2
  SS("  150"), SS("  150"), SS("  150"), SS("  150")               ;25 b5-1
  SS("  100"), SS("  110"), SS("  120"), SS("  130")               ;17 b7-3
  SS("  130"), SS("  120"), SS("  110"), SS("  100")               ;21 b9-6
  SetKeyDelay(KEYDELAY_DEFAULT)
  PrintPage()
  ;
  ChoiceSelect(&ScoreEntry_ChoiceRound_L,5)  ;  round 5
  SetKeyDelay(-1)
  SS("  100"), SS("  100"), SS("  100"), SS("  100")   ;21 a1-6
  SS("  110"), SS("  110"), SS("  110"), SS("  110")   ;13 a2-5
  SS("  120"), SS("  120"), SS("  120"), SS("  120")   ;25 a3-8
  SS("  130"), SS("  130"), SS("  130"), SS("  130")   ; 1 a4-7
  SS("  140"), SS("  140"), SS("  140"), SS("  140")   ; 1 b3-9
  SS("  150"), SS("  150"), SS("  150"), SS("  150")   ;21 b4-1
  SS("  160"), SS("  160"), SS("  160"), SS("  160")   ; 5 b5-10
  SS("  170"), SS("  170"), SS("  170"), SS("  170")   ;25 b7-6
  SetKeyDelay(KEYDELAY_DEFAULT)
  PrintPage()
  ;
  ChoiceSelect(&ScoreEntry_ChoiceRound_L,6)  ;  round 6
  SetKeyDelay(-1)
  SS(" -100"), SS(" -100"), SS(" -100"), SS(" -100")   ; 5 a2-8
  SS(" -110"), SS(" -110"), SS(" -110"), SS(" -110")   ;17 a4-6
  SS(" -120"), SS(" -120"), SS(" -120"), SS(" -120")   ; 1 a5-3
  SS(" -130"), SS(" -130"), SS(" -130"), SS(" -130")   ;25 a7-1
  SS(" -140"), SS(" -140"), SS(" -140"), SS(" -140")   ; 5 b1-7
  SS(" -150"), SS(" -150"), SS(" -150"), SS(" -150")   ; 9 b3-10
  SS(" -160"), SS(" -160"), SS(" -160"), SS(" -160")   ; 1 b4-6
  SS(" -170"), SS(" -170"), SS(" -170"), SS(" -170")   ;21 b5-2
  SetKeyDelay(KEYDELAY_DEFAULT)
  PrintPage()
  ;
  ChoiceSelect(&ScoreEntry_ChoiceRound_L,7)  ;  round 7
  SetKeyDelay(-1)
  SS(" -100"), SS("  100"), SS("  150"), SS("  200")   ; 5 a1-5
  SS("   70"), SS("   90"), SS("  100"), SS("  110")   ; 9 a4-8
  SS("  110"), SS("  120"), SS("  130"), SS("  140")   ;25 a6-2
  SS("  120"), SS("  130"), SS("  140"), SS("  150")   ;21 a7-3
  SS("  130"), SS("  140"), SS("  150"), SS("  160")   ;13 b1-10
  SS("  140"), SS("  150"), SS("  160"), SS("  170")   ;25 b3-2
  SS("    0"), SS("    0"), SS("    0"), SS("    0")   ; 1 b5-7
  SS("    0"), SS("    0"), SS("    0"), SS("    0")   ; 9 b9-4
  SetKeyDelay(KEYDELAY_DEFAULT)
  PrintPage()
} ; Scores()

ScoresEditS1()
{
  SetCheckBoxValue_L(ScoreEntry_InputOrder1_L,1)  ; set order on gameNr
    WaitNotBusy()
  ; change game 17: b7-b3: 100->150
  ChoiceSelectMC_L(ScoreEntry_ChoiceGame_L,17)  ;
    ;SetBusy()
    SetText_LEW(ScoreEntry_TextSearchEntry_L, "b7") ;{ENTER}")
     ; WaitNotBusy()
      AddText("{RIGHT 2}150{ENTER}")
      WaitNotBusy()
      PrintPage()
  ; remove game 2, a4-a7
  ChoiceSelectMC_L(ScoreEntry_ChoiceGame_L,2)
    ;SetBusy()
    SetText_LEW(ScoreEntry_TextSearchEntry_L, "a7") ;{ENTER}")
      ;WaitNotBusy()
      AddText("{RIGHT} {BS}{ENTER}")
      WaitNotBusy()
      PrintPage()
; add it back again
    ;SetBusy()
    SetText_LEW(ScoreEntry_TextSearchEntry_L, "a4") ;{ENTER}"),
      ;WaitNotBusy()
      AddText("{RIGHT 2} 130{ENTER}")
      WaitNotBusy()
      PrintPage()
} ; ScoresEditS1()

ScoresCorS1()
{
  MenuSelect(MenuCorSession)
  GridPosition(CorSession_Grid_L)      ; select the grid, goto first row/column
  if (bButler)
  {
    AddText_EW(  "{RIGHT 3} 2{TAB} 4"                 ) ; pair A1, combi: imps=2, games=4, bad, will normally popup
    AddText_EW(  "^{HOME}{DOWN 8}{RIGHT 3} -5{TAB} 4" ) ; pair B1, combi: imps-5, games 4
    AddText_EW(  "^{HOME}{DOWN 2}{RIGHT 2} 10"        ) ; pair A3, +10imps
    AddText_EW(  "^{HOME}{DOWN 4}{RIGHT 2} -3"        ) ; pair A5, -3imps
  }
  else
  {
    AddText_EW(  "{RIGHT 4}48{TAB}16")                  ; pair A1, combi: max=4*12, extra=16, bad will normally popup
    AddText_EW(  "^{HOME}{DOWN 8}{RIGHT 4}48{TAB}32")   ; pair B1, combi: max=4*12, extra=32
    AddText_EW(  "^{HOME}{DOWN 2}{RIGHT 3}10" )         ; pair A3, +10mp
    AddText_EW(  "^{HOME}{DOWN 4}{RIGHT 2}3"  )         ; pair A5, +3%
  }
  PrintPage()
  ;
  MenuSelect(MenuCorEnd)
  GridPosition(CorEnd_Grid_L)            ; select the grid, goto first row/column
  if (bButler)
    AddText_EW(   "{DOWN}{RIGHT 2}4") ; pair 2 gets 4 imps for total result
  else
    AddText_EW(   "{DOWN}{RIGHT 2}50") ; pair 2 gets 50% for total result
  PrintPage()
} ; ScoresCorS1()

CalculateAndDisplayResult()
{
  global CalcScore_ChoiceResult
  global CalcScore_ChoiceResult_L
  MenuSelect(MenuResult)        ; will show session result
  index := 1
  loop 5
  {  ; show/print all results
    ChoiceSelect(&CalcScore_ChoiceResult_L,index)
    PrintPage()
    index := NEXT_ITEM
  }
  index := 1
  Loop NrOfPairs   ; individual result for all pairs
  {
    ChoiceSelectMC_L(CalcScore_ChoicePair_L,index)
    PrintPage()
    index := NEXT_ITEM
  }
  index := 1
  Loop NrOfGames   ; results per game
  {
    ChoiceSelectMC_L(CalcScore_ChoiceGame_L,index)
    PrintPage()
    index := NEXT_ITEM
  }
;  sleep(500)
} ; CalculateAndDisplayResult()

GridPosition(label, row:=1, col:=1)
{ ; goto grid 'label' and position cursor at row 'row' and column 'col', both 1 based: (1,1) = first row, first column )
  ControlClick(label, BridgewxTitle)
  SendEvent("^{HOME}{RIGHT " col-1 "}{DOWN " row-1 "}")
} ; GridPosition()

;======================================= end of normal code =============
TESTING:
;
; start testing area
WinActivate   BridgewxTitle
WinMove(0,0,,,BridgewxTitle)
HWND1 := ControlGetFocus(BridgewxTitle)
; code to test follows here:

ExitApp(0)
