// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/frame.h>

#include <wx/textctrl.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <map>

#include "mylog.h"

#define MYSELF 1        /* if its me*/
#if MYSELF == 1
    #include "cfg.h"
#endif

class MyLogFrame;

static bool             sbLogger        {false};
static MyLogFrame*      spLogFrame      {0};
static wxWindowID       sFrameId        {0};
static wxTextCtrl*      spTextCtrl      {0};
static MyLogLevel       sLastRequested  {MyLogLevel::MyLOG_MIN};
static MyLogLevel       sLevel          {MyLogLevel::MyLOG_Message};
static bool             sbPositioned    {false};

struct frameInfo
{   // info for delayed destruction 
    MyLogFrame* pLogFrame = nullptr;
    wxWindowID  frameId   = wxID_ANY;
    frameInfo() {}
    frameInfo(MyLogFrame* pFrame, wxWindowID id)
    {
        pLogFrame   = pFrame;
        frameId     = id;
    }
};
static std::map < MyLog*, frameInfo> sm_pLogFrames; // backup of active logframes: on dynamic language switch, we need 'previous' frameptr

#define DEFAULT_SIZE {300,-1}
class MyLogFrame : public wxFrame
{   // need this 'inbetween' frame to receive events
public:
    MyLogFrame() : wxFrame(   0
                            , sFrameId
                            , _("MyLog window")
                            , {600,50}              //position
                            , DEFAULT_SIZE          //size
                            , wxDEFAULT_FRAME_STYLE //style)
                            , "MyModalLog"
                          )
    {
        pCallbackOnHide = nullptr;
    }
    ~MyLogFrame() {}

    void OnClose (wxCloseEvent& event);
    void OnSave  (wxCommandEvent& event);
    void OnClear (wxCommandEvent& event);
    void OnHide  (wxCommandEvent& event);
    void (*pCallbackOnHide)();
};

MyLog::MyLog()
{
    sFrameId = wxID_ANY;
    Create();
}

MyLog::MyLog(wxWindowID a_frameId, bool a_bCreateNow)
{
    sFrameId = a_frameId;
    if (a_bCreateNow)
        Create();
}   // MyLog()

MyLog::MyLog(bool a_bCreateNow, wxWindowID a_frameId)
{
    sFrameId = a_frameId;
    if (a_bCreateNow)
        Create();
}   // MyLog()

MyLog::~MyLog()
{   // only dtor correct frame!
    auto it = sm_pLogFrames.find(this);
    if (it != sm_pLogFrames.end())
    {
        auto pLogFrame  = it->second.pLogFrame;
        auto frameId    = it->second.frameId;
        if (pLogFrame->FindWindowById(frameId))
            delete pLogFrame;
        sm_pLogFrames.erase(it);
    }
}   // ~MyLog()

void MyLog::Create( )
{
    spLogFrame = new MyLogFrame();
    wxMenu *pMenu = new wxMenu;
    pMenu->Append(ID_MENU_MYLOG_SAVE , _("save &As..."), _("Save in logfile"           ));
    pMenu->Append(ID_MENU_MYLOG_CLEAR, _("&Clear")     , _("Clear logwindow"           ));
    pMenu->AppendSeparator();
    pMenu->Append(ID_MENU_MYLOG_HIDE,  _("&Close")     , _("Close (hide) the logwindow"));

    wxMenuBar *pMenuBar = new wxMenuBar;
    pMenuBar->Append(pMenu, _("&File"));
    spLogFrame->SetMenuBar(pMenuBar);

    spLogFrame->Bind(wxEVT_MENU         , &MyLogFrame::OnSave , spLogFrame, ID_MENU_MYLOG_SAVE );
    spLogFrame->Bind(wxEVT_MENU         , &MyLogFrame::OnClear, spLogFrame, ID_MENU_MYLOG_CLEAR);
    spLogFrame->Bind(wxEVT_MENU         , &MyLogFrame::OnHide,  spLogFrame, ID_MENU_MYLOG_HIDE );
    spLogFrame->Bind(wxEVT_CLOSE_WINDOW , &MyLogFrame::OnClose, spLogFrame  /* NB NO id here*/ );

    spLogFrame->SetStatusBar(new wxStatusBar(spLogFrame));

    spTextCtrl = new wxTextCtrl(spLogFrame, wxID_ANY, ES,
        wxDefaultPosition, DEFAULT_SIZE, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);

    spLogFrame->Show(false);                    // start invisable

    sFrameId    = spLogFrame->GetId();          // in case it was wxID_ANY
    sbLogger    = true;                         // we have a logger
    sLevel      = MyLogLevel::MyLOG_Message;    // default loglevel
    sm_pLogFrames[this] = frameInfo(spLogFrame, sFrameId);  // dtor info
    sbPositioned= false;                        // first time position is next to app-window
}   // Create()

bool MyLog::IsShown()
{
    return spLogFrame->IsShown();
}   // IsShown()

void MyLog::SetCallbackOnHide(void(*ptr)())
{
    spLogFrame->pCallbackOnHide = ptr;
}   // SetCallbackOnHide()

void MyLogFrame::OnHide (wxCommandEvent& )
{
    if (pCallbackOnHide) pCallbackOnHide();
    spLogFrame->Show(false);
}   // OnHide()

void MyLogFrame::OnSave  (wxCommandEvent& )
{
    wxString filename = wxSaveFileSelector("log", "txt", "log.txt", spLogFrame);
    if (!filename.IsEmpty())
        spTextCtrl->SaveFile(filename);
    LogMessage(_("Log saved to file '%s'."), filename);
}   // OnSave()

void MyLogFrame::OnClear (wxCommandEvent& )
{
    spTextCtrl->Clear();
}   // OnClear()

void MyLogFrame::OnClose(wxCloseEvent& a_event)
{
    if (pCallbackOnHide) pCallbackOnHide();
    spLogFrame->Show(false);
    a_event.Veto();
}   // OnClose()

wxWindowID MyLogGetId()
{
    return sFrameId;
}   // MyLogGetId()

void MyLogSetLevel(MyLogLevel a_level)
{
    if (a_level >= MyLogLevel::MyLOG_MIN && a_level <= MyLogLevel::MyLOG_Max)
        sLevel = a_level;
    else
        sLevel = MyLogLevel::MyLOG_Message;
}   // MyLogSetLevel()

void MyLogShow(bool a_bShow)
{   // just check if window still exists....
    if (sbLogger && spLogFrame && spLogFrame->FindWindowById(sFrameId))
    {
        if (!sbPositioned)
        {
            sbPositioned = true;
            int x = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
            wxPoint pos = {0,50};
            if (GetMainframe())
            {
                pos.x = GetMainframe()->GetPosition().x + GetMainframe()->GetSize().GetX();
                if (pos.x > x-100) pos.x = x-100;
            }
            else
                pos.x = (x*1600)/2000;
            spLogFrame->SetPosition(pos);
        }
        spLogFrame->Show(a_bShow);
    }
}   // MyLogShow()

static wxString theType[]=
{
      " ERR: "
    , " WRN: "
    , " MSG: "
    , " INF: "
    , " DBG: "
};

static const unsigned int nrOfTypes = sizeof(theType)/sizeof(theType[0]);

bool MyLogIsEnabled(MyLogLevel a_level)
{
    if (!sbLogger || sLevel < a_level || a_level < MyLogLevel::MyLOG_MIN ) return false;
    if ( (unsigned int)a_level >= nrOfTypes ) return false;
#ifndef _DEBUG
    if (a_level == MyLogLevel::MyLOG_Debug) return false;
#endif
    sLastRequested = a_level;   // (was) needed for displaying the log-level
    return true;
}   // MyLogIsEnabled()

void MyLogGeneral(MyLogLevel a_level, const wxString& a_msg)
{
    if (!sbLogger || sLevel < a_level ) return;
    if ( (unsigned int)a_level >= nrOfTypes ) return;

#ifndef _DEBUG  // in releasemode only show errors. If debug set, also show debug msg
    if (a_level != MyLogLevel::MyLOG_Error)
    {
        if (a_level != MyLogLevel::MyLOG_Debug) return;
        #if MYSELF == 1
            if (!cfg::IsDebug()) return;
        #endif
    }
#endif
    static bool bTesting = cfg::IsScriptTesting();
    wxString result = bTesting
    ? wxDateTime::UNow().Format("%H:%M:%S:%l") + theType[(int)a_level] + a_msg
    : wxDateTime:: Now().Format("%H:%M:%S")    + theType[(int)a_level] + a_msg;

    result.Replace("\n", "\\n", true);
    spTextCtrl->AppendText(result + '\n');
}   // MyLogGeneral()
