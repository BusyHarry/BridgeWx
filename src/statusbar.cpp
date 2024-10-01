// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "wx/wx.h"
#include "statusbar.h"
#include "cfg.h"

// ----------------------------------------------------------------------------
// MyStatusBar
// ----------------------------------------------------------------------------

#ifdef __VISUALC__
    // 'this' : used in base member initializer list -- so what??
#pragma warning(disable: 4355)
#endif

MyStatusBar::MyStatusBar(wxWindow* a_parent, long a_style)
    : wxStatusBar(a_parent, wxID_ANY, a_style, "MyStatusBar")
    , m_timer(this)
{
     // compute the size needed for clock indicator pane
    wxSize sizeClock= GetTextExtent("22:50:59 PM");   // need the ' ' else field too small in dpi-aware exe...
    wxSize sizeInfo = GetTextExtent("ABCDEFGH:Z9, WINP");

    int widths[FIELD_MAX];
    widths[FIELD_TEXT]  = -1;           // growable, 'normal' status texts
    widths[FIELD_INFO]  = sizeInfo.x;   // competition name+session+printername
    widths[FIELD_CLOCK] = sizeClock.x;  // the clock

    SetFieldsCount(FIELD_MAX);
    SetStatusWidths(FIELD_MAX, widths);

    Bind(wxEVT_TIMER, &MyStatusBar::OnTimer, this, wxID_ANY);
    SetClock();
}   // MyStatusBar()

void MyStatusBar::SetInfo(const wxString& a_info)
{
    SetStatusText(a_info, FIELD_INFO);
}   // SetInfo()

void MyStatusBar::SetClock()
{
    if (cfg::GetClock() && !cfg::IsScriptTesting())
    {
        m_timer.Start(1000);    // 1 second
        UpdateClock();
    }
    else
    {
        m_timer.Stop();
        SetStatusText("              ", FIELD_CLOCK);
    }
}   // SetClock()

#ifdef __VISUALC__
#pragma warning(default: 4355)
#endif

MyStatusBar::~MyStatusBar()
{
    if (m_timer.IsRunning())
    {
        m_timer.Stop();
    }
}   // ~MyStatusBar()

void MyStatusBar::UpdateClock()
{
    SetStatusText(wxDateTime::Now().FormatTime(), FIELD_CLOCK);
}   // UpdateClock()

void MyStatusBar::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    UpdateClock();
}   // OnTimer()
