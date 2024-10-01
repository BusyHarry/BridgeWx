// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _STATUSBAR_H_
#define _STATUSBAR_H_
#pragma once
#include "wx/wx.h"

class MyStatusBar : public wxStatusBar
{
public:
    explicit MyStatusBar(wxWindow* parent, long style = wxSTB_DEFAULT_STYLE);
    virtual ~MyStatusBar();

    void UpdateClock();
    void SetInfo(const wxString& a_info);
    void SetClock();

protected:
    // event handlers
    void OnTimer(wxTimerEvent& WXUNUSED(event));

private:
    enum
    {
        FIELD_TEXT,
        FIELD_INFO,
        FIELD_CLOCK,
        FIELD_MAX
    };

    wxTimer     m_timer;
};

#endif
