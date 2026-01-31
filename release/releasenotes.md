# Releasenotes
#V10.10.0  ...
 - reorganised the in/output methods in fileio in preparation to adding an sqlite database for storage
 - logic error while reading the config when an array value was out of range
 - sometimes you got an unexpected matchname like "-c1"
 - early creation of MyLog in App::OnInit(): we need it there for cmdline handling
 - resolve remarks of cppcheck V2.19.0
 - removed all trailing white space
 - added sqlite as data-storage
   - (for now) data is stored same as .db, so many data-strings and not individual items
   - select through menu 'tools/sWitch datatype'
   - referencefiles for testing added
   - version of sqlite3: sqlite-amalgamation-3.51.02.00 from 2026-01-09

#V10.9.0  Sunday November 23 2025
 - added encode/decode strings containing special characters when storing/retreiving the database
 - updated reference test outputs because of encode/decode strings in the database
 - replace deprecated fopen() with fopen_s()
 - undone above for logfile: fopen_s() does NOT support read-access by other apps!
 - cppcheck remarks removed
 - correction of wrong comment
 - corrections/extensions in conversion 'old' --> 'new' format:
    - don't show pairs in result that did not play
    - initialise pairnames for use in results/corrections
    - ignore zero results/corrections to prevent unneeded error popups
    - add missing schema/group descriptions
    - added read of 'very old' matchdata
    - to <old> : don't create empty files
 - Added main.h and moved some functions (communication with mainframe) from cfg to main 
 - added possibility to add scores through a web-page, using a server in the local network
    - this can replace the use of paper score-slips
    - needs a tablet/phone/laptop per table
 - added dark-mode support
   - use dark (complementary) colors where colors are explicitly set
     - through 'GetLightOrDark(const wxColor& lightColor)' -> in=light, out=light or dark, depending on dark-mode
   - changed SetUseNativeColLabels() to UseNativeColHeader() for wxGrid
     - SetUseNativeColLabels() has unchangeable white columnlabel background in dark-mode -> unreadable text
     - UseNativeColHeader() has less space for label -> enlarge many column sizes
     - UseNativeColHeader(): SetColLabelValue() has no effect within Begin/EndBatch() (just: 'A' 'B' 'C' ..) -> remove Begin/EndBatch()
   - wxListCtrl needs explicit SetTextColour(*wxWHITE) for columnheaders in dark-mode when using SetHeaderAttr()
     - dark-mode: no row highlight when hovering mouse (label ok!)
     - dark-mode: if using 'wxLC_HRULES', the lines disappear when hovering mouse over them


#V10.8.0  Monday Juli 21 2025
 - disabled v3.3.0 feature to show highlighted grid columnlabel on activating a cell
 - when producing builddate.h, 'prebuild.bat' now defaults to 'en_GB' locales for date and time
 - use ./build/msw/wx_setup.props to get the version-dependend names of some wx libraries
   - using '#include <msvc/wx/setup.h>' in main.cpp, all needed libs are auto-included, no need for above .props
 - added 'BridgeWx.props' to set needed variable(s): wxwin (base of wxWidgets version)
   - remark: if 'wxwin'is set in the environment, the BridgeWx.props file is not loaded
 - changed disabled feature: 
   - bad side effect: columnheaders got a much larger margin, so many texts did not fit anymore
   - new implementation has nice side effect of having sort-icons available in the column headers
 - show messagebox after page has been setup to prevent msgbox on empty page
 - implementation of combi-table results and optimization of table displays in result pages
 - added an overview of (possible) useful functions for those not interested in the program itself
 - optimization of session corrections (entry and result-calculation)
 - autotest: added symbolic names for columns of a grid
 - more logical column order in corrections for the total result
 - optimization of end corrections (entry and result-calculation)
 - updated/new validator for wxTextCtrl and grid-editor
 - implemented new validator


#V10.7.0  Monday June 9, 2025
 - easier translation (more)
 - wxCombobox sometimes truncates content (old problem again)
 - import of schema's: 'rounds' and 'sets' were swapped
 - set vs19 project settings to use x64 tools (x86 caused: C1060, compiler is out of heap space)
 - added a ./locales/getTxtNoLines.bat that produces translation files without linenumbers: much less differences!
 - force fontsize of dialog to size of its parent (last one I missed?)
 - added 'export schema' in the guides page
 - redesign/update of schema-data stuf
 - adaptations for wxWidgets V3.3.0


#V10.6.0  Tuesday 20 May 2025
 - updated wxWidgets to V3.2.8 (no changes compared to V3.2.7)
 - don't need anymore to change 'include/wx/grid.h' with public method 'SetRow()'
 - show used version of wxWidgets in About()
 - added 'pass' as valid contract
 - when reading scores from the database, clear the contracts for each game, else they inherit these from a previous game!
 - removed an empty, unneeded correction column in several result listings
 - made translations easier by doing the math for tables internally, so only plain text-translations needed

#V10.5.0  Tuesday 1 April 2025
 - replaced most wxMessageBox() with MyMessageBox()
 - outof bounds array access: you have scores, you reduce the number of players in a group, recalculate results
   - now the bad scores are removed and the internal pairnumbers are adjusted
 - added importing new playing schemas/movements
 - changed 'calculation program' to 'scoring program'
 - changed internal playing schemas/movements into an easier to read format
 - stupid typo in IsInRange()
 - decoupled old/global gameinfo
 - small texts update
 - consequent use of wxString::IsEmpty() i.s.o wxString::empty()
 - changed some wxStatictext to wxGenericStaticText to prevent clipping problems
 - solved problem when .db was on a non-existing/non-writable location
 - now current programversion is always updated in the datafiles
 - entry of scores can now also be done by contract i.s.o pure score-data
 - updated wxWidgets to V3.2.7 (no changes compared to V3.2.6)

#V10.4.0 Tuesday 4 March 2025
 - updated AutoHotkey64.exe from version 2.0.18 to 2.0.19
 - changed some bad-english words to 'better' ones (arbitrairy score -> adjusted score)
 - refactored the MyLog class
   - only dependend of wx-headerfiles
   - all functions are now static methods, so no polluting of global namespace
   - added scaling for the log-messages
 - when scaling is used, also MyMessageBox(), MyLog(), BusyBox() and wxSystemInformationFrame() are scaled
   - apparently wxDialog and wxFrame do NOT inherit the fontsize of there parent
 - added butler score calculation
 - scaling added/updated in:
   - language selection (wxGetSingleChoiceIndex --> MyGetSingleChoiceIndex)
   - row/column lables of wxGrid
   - MyMessageBox
   - BusyBox
   - size of mainframe

#V10.3.0  Monday Januari 20 2025:
 - adaptation from c++17 to c++20
   - logical '|' not allowed between different enums (intensionally used in wxSizers)
   - result values of the '?' operator should have equal type
   - variable name 'default' not allowed anymore, changed to 'defaultValue'
 - the popup for unlikely scores could show the negated score
 - an unlikely score is now shown in red
 - a rejected unlikely score does not result anymore in an extra 'wrong score' popup
 - added the vc-redistribution .dll's to the release zip

#V10.2.0  November 8, 2024:
- all texts in sources now in english
- translation for english removed
- translation for dutch added
- changes in batch-files to cover the language-change
- autotest:
  - scripts now also have texts in english
  - reference files now for english and dutch

#V10.1.0  October 25, 2024: bugfixes/extensions/cleanup

#New Features
- on language change, switch to 'old' page
- print guides/scoreslips also to file 'list'
- 'list' is now coded in utf-8 with BOM (was DOS-cp437)
- adaptation to dpi-changes
- scoreslips now can have upto 6 games/slip

#Bug fixes
- better handling of unexpected data when reading schemadata
- don't display schemadata and prevent unneeded assert's when there is no active schema
- don't show empty rows for ChoiceMc()
- don't send selection event for ChoiceMc if clicked on scrollbar-area 


#V10.0.0  September 30, 2024: first release of BridgeWx

A simple (windows-only) 'bridge' calculation program

Choose language: menu->extra/taal or menu->tools/language

If you get errors 'missing dll' at startup (VCRUNTIME140.dll, MSVCP140.dll, VCRUNTIME140_1.dll): then download/install the redistributable package from: https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170

 - https://aka.ms/vs/17/release/vc_redist.x64.exe   : for 64 bit BridgeWx
 - https://aka.ms/vs/17/release/vc_redist.x86.exe   : for 32 bit BridgeWx

See read.md for more info
