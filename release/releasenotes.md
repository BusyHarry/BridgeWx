# Releasenotes

#V10.1.0  October 25, 2024: bugfixes/extensions/cleanup

- on language change, switch to 'old' page
- better handling of unexpected data when reading schemadata
- don't display schemadata and prevent unneeded assert's when there is no active schema
- don't show empty rows for ChoiceMc()
- don't send selection event for ChoiceMc if clicked on scrollbar-area 
- print guides/scoreslips also to file 'list'
- 'list' is now coded in utf-8 with BOM (was DOS-cp437)
- adaptation to dpi-changes


#V10.0.0  September 30, 2024: first release of BridgeWx

A simple (windows-only) 'bridge' calculation program

Choose language: menu->extra/taal or menu->tools/language

If you get errors 'missing dll' at startup (VCRUNTIME140.dll, MSVCP140.dll, VCRUNTIME140_1.dll): then download/install redistributable package from: https://aka.ms/vs/17/release/vc_redist.x64.exe or https://aka.ms/vs/17/release/vc_redist.x86.exe

See read.md for more info
