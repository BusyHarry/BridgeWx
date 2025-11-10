// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _MAIN_H_
#define _MAIN_H_
#pragma once

class wxString;
class wxWindow;

void        SetStatusbarText(const wxString& msg);
void        SetStatusbarInfo();
wxWindow*   GetMainframe();

/*
communicate with mainframe on the base of a menu-event
@param id the value for the command handler
@param pClientData optional pointer to extra data for this command
*/
void SendEvent2Mainframe(int id, void* const pClientData = nullptr);
void SendEvent2Mainframe(wxWindow* pWindows, int id, void* const pClientData = nullptr);

#endif
