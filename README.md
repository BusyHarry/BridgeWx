# BridgeWx

A simple bridge calculation/scoring program.
Its a gui-remake of an old dos-commandline program I started in the early 1990s when my bridgepartner was a districtscompetition leader of the Dutch Bridge Association. Its based on the WxWidgets framework.

Score entry can be done via:
- score-slips
- per game

You can have multiple sessions (competition) and multiple groups per session.
There is a global name entry for participants and you can assign these names to a specific player in a session.

Corrections can be entered for a session and for the competition.

Default, the calculation is done as % scoring.
Since V10.4.0 scoring can also be done as Butler scoring.

Results are shown in different formats for the session, competition, per player, per game.
If you enter club-info for the players, you also can have club-rank as result.

You can create guides for the participants, and scoreslips for entering play-results.

You can also run an 'autotest' on the program: feed it with lots of data, do calculations, print out stuff (to a disk-file) and compare it with a 'known good' output.
For this test I used 'AutoHotkey2' and for this, I have buildin some synchronisation methods, so data-entry can be as fast as possible, without using too many 'sleeps()'
When setting keydelay/duration to minimum, some chars are lost/arrive at wrong window: you will get some (still unhandled) asserts.
To run the autotest:
 - AutoHotkey64.exe MUST exist, reachable through the 'path' environment-variable, or be present in the tools folder
 - run once: initTest.bat (it will create a file with mouse-positions/labels used in the actual test)
 - run the test: runTest.bat
 - afterwoods, you may run 'compareDb.bat' and 'compareList.bat' to check the results against a 'correct' reference (for English and Dutch present in the autotest folder)

I used SciTE4AutoHotkey ( https://www.autohotkey.com/scite4ahk/ ) to edit/debug the testscript.
Another edit/debug tool: Visual Studio Code with extension AutoHotkey Plus Plus (AHK++) ( https://marketplace.visualstudio.com/items?itemName=mark-wiemer.vscode-autohotkey-plus-plus )


# Running BridgeWx

If you run BridgeWx for the first time from the binairy on a clean win10 installation you may get errors when starting:
  - missing VCRUNTIME140.dll
  - missing MSVCP140.dll
  - missing VCRUNTIME140_1.dll

These .dll's are needed when running BridgeWx (compiled with VS2019/VS2022). 
Download/install the redistributable package from: https://aka.ms/vs/17/release/vc_redist.x64.exe
(for x86: https://aka.ms/vs/17/release/vc_redist.x86.exe )
 - As from V10.3.0 on, these dll's are included in the BridgeWxBin_Vxxx.zip 

## Language

From the "Extra\Taal" menu in Dutch, or "Tools\Language" in English, you can switch to all available languages.
On first use, I try to select the current systemlanguage.

# Translations

- As of version V10.2.0, internal texts are in English (was Dutch)

Used names:
- \<wxBase\>		: the folder where you installed/unzipped the wxWidgets source
- \<BridgeWxBase\>	: the folder where the BridgeWx files are
- \<exeDir\>		: the folder where the BridgeWx.exe resides (debug or release)
- \<language\>		: the wanted display language

There are provisions for translation: all translatable text have the _(..) macro.
I have used Poedit to retrieve these text to \<BridgeWx.pot\>. From there I have made a Dutch translation \<nl.po\> and compiled it to \<nl.mo\> (all in the \<BridgeWxBase\>/locales' folder)
You may use the './locales/gettxt.bat' or './locales/getTxtNoLines.bat' to:
  - update \<BridgeWx.pot\> from the sources
  - update/create a \<language\>.po from the .pot file
  - change/add translations
  - compile to .mo
  - you will get a warning pop-up if Poedit can not be found in '\<drive\>/Program Files (x86)/Poedit' where \<drive\> = C:,D:,E:,F:

To use this (or your own) translation you must do the following:

To get the text from wxWidgets itself in your language:
 - copy '\<wxBase\>/locale/\<language\>.mo' to '\<exeDir\>/locales/\<language\>/wxstd.mo'

To get your BridgeWx translation:
 - copy '\<BridgeWxBase\>/locales/\<language\>.mo' to '\<exeDir\>/locales/\<language\>/BridgeWx.mo'

Remark: English is buildin, so for this language you don't need to do anything


# Building:

 - I have only build/tested everything in MicrosoftWindows 10 with VisualStudio2019!
 - the gui is based on wxWidgets V3.3.1 (V3.3.0, V3.2.5 --> V3.2.8 also ok)
 - I only used the x64 version (also for wxWidgets)
 - copy the tools from the release BridgeWxBin_v\*.zip[BridgeWx/tools/*.exe] to the '\<BridgeWxBase\>/tools' folder 
 - add  global env-variables '\<wxwin\>' and '\<wx_version\>' or
 - from a cmd-prompt: set environment variable '\<wxwin\>' to wxWidgets basefolder and '\<wx_version\>' to the MajorMinor wx-version:
    - set wxwin=d:\wxWidgets_3.3.0  ( == \<wxBase\> )
    - set wx_version=33             ( for V3.3.* (default) and '32' for V3.2.*)
    - as from V10.8.0 the setting of 'wx_version' is not needed anymore

 - start BridgeWx.sln (from the base folder where the repo is located)
    - from version V10.8.0 onwards, you can also use the BridgeWx.props file i.s.o the environment variable(s)
       - adjust the value(s) in it so they match your situation
       - the environment has priority above the .props file

 - initially to have the Dutch language available for the locally compiled executable:
     - create the folder(s) for the Dutch language (automaticaly done when building in VS):
       '\<BridgeWxBase\>vc_x64_mswu[d]/locales/nl'
   and copy the *mo files from the BridgeWxBin_v\*.zip to there corresponding locations

# Network use
 - from version V10.9.0 a 'slipserver' has been added: you can add result-scores on all tables during play in a web-page
 - for more info see '/slipServer/read.me'

# Dark mode support
 - from version V10.9.0 dark-mode is supported
 - it can be forced by setting an environment variable: set wx_msw_dark_mode=2

# SQLite
 - from version V10.10.0 you can also store the game-data in an SQLite database
 - referencefiles for autotesting are added

# Licence

MIT License
You may use (part of) this code for your own. However, I would appreciate a link to the original developer.
