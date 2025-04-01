# Releasenotes
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
