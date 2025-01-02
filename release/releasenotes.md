# Releasenotes

#V10.3.0  ....... :
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

If you get errors 'missing dll' at startup (VCRUNTIME140.dll, MSVCP140.dll, VCRUNTIME140_1.dll): then download/install redistributable package from: https://aka.ms/vs/17/release/vc_redist.x64.exe or https://aka.ms/vs/17/release/vc_redist.x86.exe

See read.md for more info
