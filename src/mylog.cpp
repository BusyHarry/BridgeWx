// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/frame.h>

#include <wx/textctrl.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <map>

#include "mylog.h"

class MyLogFrame;

static bool             s_bLogger               {false};
static MyLogFrame*      s_pLogFrame             {0};
static wxWindowID       s_frameId               {0};
static wxTextCtrl*      s_pTextCtrl             {0};
static MyLog::Level     s_levelLastRequested    {MyLog::Level::LOG_MIN};
static MyLog::Level     s_levelCurrent          {MyLog::Level::LOG_Message};
static bool             s_bPositioned           {false};
static bool             s_bAppDebug             {false};
static bool             s_bScriptTesting        {false};
static const wxWindow*  s_pMainframe            {nullptr};  //ptr to mainframe to position logging window alongside

extern const wxString ES;       // an Empty String

struct frameInfo
{   // info for delayed destruction
    MyLogFrame* pLogFrame;
    wxWindowID  frameId;
    frameInfo()                                 : pLogFrame(nullptr), frameId(wxID_ANY){;}
    frameInfo(MyLogFrame* pFrame, wxWindowID id): pLogFrame(pFrame) , frameId(id)      {;}
};
static std::map < const MyLog*, frameInfo> s_mpLogFrames;   // backup of active logframes: on dynamic language switch, we need 'previous' frameptr

static wxSize const DEFAULT_SIZE = {300,-1};

class MyLogFrame : public wxFrame
{   // need this 'inbetween' frame to receive events
public:
    MyLogFrame() : wxFrame(   nullptr
                            , s_frameId
                            , _("MyLog window")
                            , {600,50}              //position
                            , DEFAULT_SIZE          //size
                            , wxDEFAULT_FRAME_STYLE //style)
                            , "MyModalLog"
                          )

    {}
    ~MyLogFrame() final = default;

    void OnClose (wxCloseEvent& event);
    void OnSave  (const wxCommandEvent& event);
    void OnClear (const wxCommandEvent& event);
    void OnHide  (const wxCommandEvent& event);
    pCallBack    pCallbackOnHide = nullptr;
};

MyLog::MyLog()
{
    s_frameId = wxID_ANY;
    Create();
}

MyLog::MyLog(wxWindowID a_frameId, bool a_bCreateNow)
{
    s_frameId = a_frameId;
    if ( a_bCreateNow )
        Create();
}   // MyLog()

MyLog::MyLog(bool a_bCreateNow, wxWindowID a_frameId)
{
    s_frameId = a_frameId;
    if ( a_bCreateNow )
        Create();
}   // MyLog()

MyLog::~MyLog()
{   // only dtor correct frame!
    auto it = s_mpLogFrames.find(this);
    if ( it != s_mpLogFrames.end() )
    {
        auto pLogFrame  = it->second.pLogFrame;
        auto frameId    = it->second.frameId;
        if ( pLogFrame->FindWindowById(frameId) )
            delete pLogFrame;
        s_mpLogFrames.erase(it);
    }
}   // ~MyLog()

void MyLog::Create( ) const
{
    enum MY_MENU_IDS
    {
          ID_MENU_MYLOG_SAVE= wxID_HIGHEST + 1
        , ID_MENU_MYLOG_CLEAR
        , ID_MENU_MYLOG_HIDE
    };
    s_pLogFrame = new MyLogFrame();
    auto pMenu = new wxMenu;
    pMenu->Append(ID_MENU_MYLOG_SAVE , _("save &As..."), _("Save in logfile"           ));
    pMenu->Append(ID_MENU_MYLOG_CLEAR, _("&Clear")     , _("Clear logwindow"           ));
    pMenu->AppendSeparator();
    pMenu->Append(ID_MENU_MYLOG_HIDE,  _("&Close")     , _("Close (hide) the logwindow"));

    auto pMenuBar = new wxMenuBar;
    pMenuBar->Append(pMenu, _("&File"));
    s_pLogFrame->SetMenuBar(pMenuBar);

    s_pLogFrame->Bind(wxEVT_MENU         , &MyLogFrame::OnSave , s_pLogFrame, ID_MENU_MYLOG_SAVE );
    s_pLogFrame->Bind(wxEVT_MENU         , &MyLogFrame::OnClear, s_pLogFrame, ID_MENU_MYLOG_CLEAR);
    s_pLogFrame->Bind(wxEVT_MENU         , &MyLogFrame::OnHide,  s_pLogFrame, ID_MENU_MYLOG_HIDE );
    s_pLogFrame->Bind(wxEVT_CLOSE_WINDOW , &MyLogFrame::OnClose, s_pLogFrame  /* NB NO id here*/ );

    s_pLogFrame->SetStatusBar(new wxStatusBar(s_pLogFrame));

    s_pTextCtrl = new wxTextCtrl(s_pLogFrame, wxID_ANY, wxEmptyString,
        wxDefaultPosition, DEFAULT_SIZE, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);

    int size = s_pTextCtrl->GetFont().GetPointSize();
    wxFont  celFont (size, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    s_pTextCtrl->SetFont(celFont);              // fixed sized font, so tables look nice

    s_pLogFrame->Show(false);                   // start invisable

    s_frameId    = s_pLogFrame->GetId();        // in case it was wxID_ANY
    s_bLogger    = true;                        // we have a logger
    s_levelCurrent      = MyLog::Level::LOG_Message;            // default loglevel
    s_mpLogFrames[this] = frameInfo(s_pLogFrame, s_frameId);    // dtor info
    s_bPositioned= false;                       // first time position is next to app-window
}   // Create()

bool MyLog::IsShown()
{
    return s_pLogFrame->IsShown();
}   // IsShown()

void MyLog::SetCallbackOnHide(pCallBack ptr)
{
    s_pLogFrame->pCallbackOnHide = ptr;
}   // SetCallbackOnHide()

void MyLogFrame::OnHide (const wxCommandEvent& )
{
    if ( pCallbackOnHide )
        pCallbackOnHide();
    s_pLogFrame->Show(false);
}   // OnHide()

void MyLogFrame::OnSave  (const wxCommandEvent& )
{
    wxString filename = wxSaveFileSelector("log", "txt", "log.txt", s_pLogFrame);
    if ( !filename.IsEmpty() )
        s_pTextCtrl->SaveFile(filename);
    MyLogMessage(_("Log saved to file '%s'."), filename);
}   // OnSave()

void MyLogFrame::OnClear (const wxCommandEvent& )
{
    s_pTextCtrl->Clear();
}   // OnClear()

void MyLogFrame::OnClose(wxCloseEvent& a_event)
{
    if ( pCallbackOnHide )
        pCallbackOnHide();
    s_pLogFrame->Show(false);
    a_event.Veto();
}   // OnClose()

wxWindowID MyLog::GetId()
{
    return s_frameId;
}   // GetId()

void MyLog::SetLevel(MyLog::Level a_level)
{
    if ( a_level >= MyLog::Level::LOG_MIN && a_level <= MyLog::Level::LOG_Max )
        s_levelCurrent = a_level;
    else
        s_levelCurrent = MyLog::Level::LOG_Message;
}   // SetLevel()

void MyLog::FontScale(float a_scale)
{
    if ( s_bLogger && s_pLogFrame && s_pLogFrame->FindWindowById(s_frameId) )
    {
        auto font = s_pTextCtrl->GetFont();
        s_pTextCtrl->SetFont(font.Scale(a_scale));
    }
}   // FontScale()

void MyLog::Show(bool a_bShow)
{   // just check if window still exists....
    if ( s_bLogger && s_pLogFrame && s_pLogFrame->FindWindowById(s_frameId) )
    {
        if ( !s_bPositioned )
        {
            s_bPositioned = true;
            int x = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
            wxPoint pos = {0,50};

            if ( s_pMainframe )
            {
                pos.x = s_pMainframe->GetPosition().x + s_pMainframe->GetSize().GetX();
                if ( pos.x > x-100 )
                    pos.x = x-100;
            }
            else
                pos.x = (x*1600)/2000;
            s_pLogFrame->SetPosition(pos);
        }
        s_pLogFrame->Show(a_bShow);
    }
}   // Show()

static const wxString s_asTheType[]=
{
      " ERR: "
    , " WRN: "
    , " MSG: "
    , " INF: "
    , " DBG: "
};

static constexpr unsigned int NR_OF_TYPES = sizeof(s_asTheType)/sizeof(s_asTheType[0]);

bool MyLog::IsEnabled(MyLog::Level a_level)
{
    if ( !s_bLogger || s_levelCurrent < a_level || a_level < MyLog::Level::LOG_MIN )
        return false;
    if ( (unsigned int)a_level >= NR_OF_TYPES )
        return false;
#ifndef _DEBUG
    if ( a_level == MyLog::Level::LOG_Debug )
        return false;
#endif
    s_levelLastRequested = a_level;     // (was) needed for displaying the log-level
    return true;
}   // IsEnabled()

void MyLog::DoLog(MyLog::Level a_level, const wxString& a_msg)
{
    if ( !s_bLogger || s_levelCurrent < a_level )
        return;
    if ( (unsigned int)a_level >= NR_OF_TYPES )
        return;

#ifndef _DEBUG  // in releasemode only show errors. If debug set, also show debug msg
    if ( a_level != MyLog::Level::LOG_Error )
    {
        if ( a_level != MyLog::Level::LOG_Debug && !s_bAppDebug )
            return;
    }
#endif
    wxString result = s_bScriptTesting
    ? wxDateTime::UNow().Format("%H:%M:%S:%l") + s_asTheType[(int)a_level] + a_msg
    : wxDateTime:: Now().Format("%H:%M:%S")    + s_asTheType[(int)a_level] + a_msg;

    result.Replace("\n", "\\n", true);
    s_pTextCtrl->AppendText(result + '\n');
}   // DoLog()

void MyLog::SetScriptTesting()
{
    s_bScriptTesting = true;
}   // SetScriptTesting()

void MyLog::SetAppDebugging()
{
    s_bAppDebug = true;
}   // SetAppDebugging()

void MyLog::SetMainFrame(const wxWindow* pMainframe)
{
    s_pMainframe = pMainframe;
}   // SetMainFrame()
