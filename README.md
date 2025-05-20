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


# Running WxBridge

If you run WxBridge for the first time from the binairy on a clean win10 installation you may get errors when starting:
  - missing VCRUNTIME140.dll
  - missing MSVCP140.dll
  - missing VCRUNTIME140_1.dll

These .dll's are needed when running BridgeWx (compiled with VS2019/VS2022). 
Download/install the redistributable package from: https://aka.ms/vs/17/release/vc_redist.x64.exe
(for x86: https://aka.ms/vs/17/release/vc_redist.x86.exe )

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
I have used Poedit to retrieve these text to \<BridgeWx.pot\>. From there I have made an English translation \<en.po\> and compiled it to \<en.mo\> (all in the \<BridgeWxBase\>/locales' folder)
You may use the './locales/gettxt.bat' to
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

# Building:

 - I have only build/tested everything in MicrosoftWindows 10 with VisualStudio2019!
 - the gui is based on wxWidgets V3.2.8 (V3.2.7, V3.2.6, V3.2.5 is also ok)
 - I only used the x64 version (also for wxWidgets)
 - copy the tools from the release BridgeWxBin_v\*.zip[BridgeWx/tools/*.exe] to the '\<BridgeWxBase\>/tools' folder 
 - add a global env-variable '\<wxwin\>' or
 - from a cmd-prompt: set environment variable '\<wxwin\>' to wxWidgets basefolder:
 	- set wxwin=d:\wxWidgets_3.2.8  ( == \<wxBase\> )

 - start BridgeWx.sln (from the base folder where the repo is located)

 - initially to have the English language available for the compiled executable:
     - create the folder(s) for the Dutch/English language (automaticaly done when building in VS):
       - '\<BridgeWxBase\>vc_x64_mswu[d]/locales/en' and
       - '\<BridgeWxBase\>vc_x64_mswu[d]/locales/nl'
   and copy the *mo files from the BridgeWxBin_v\*.zip to there corresponding locations

# Licence

MIT License
You may use (part of) this code for your own. However, I would appreciate a link to the original developer.
