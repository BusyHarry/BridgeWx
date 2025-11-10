// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// Start of wxWidgets Bridge calc Program
#ifdef _MSC_VER
#include <msvc/wx/setup.h>      // all kinds of defines and force include of needed wx-libs
#endif
#include <wx/image.h>
#include <wx/uilocale.h>
#include <wx/stdpaths.h>

// my own includes
#include "cfg.h"
#include "statusbar.h"
#include "nameEditor.h"
#include "setupGame.h"
#include "setupSchema.h"
#include "setupPrinter.h"
#include "setupNewMatch.h"
#include "assignNames.h"
#include "scoreEntry.h"
#include "correctionsSession.h"
#include "correctionsEnd.h"
#include "printer.h"
#include "debug.h"
#include "calcScore.h"
#include "showStartImage.h"
#include "database.h"
#include "slipServer.h"
#include "fileIo.h"
#include "version.h"
#include "main.h"

static wxWindow* spMainframe = nullptr;      // to reach mainframe from clients
wxWindow* GetMainframe(){return spMainframe;}

wxCheckBox* g_pCheckboxBusyMC=nullptr;  // autohotkey uses this to find out if it has to wait before sending movements to popup in ChoiceMC
wxCheckBox* g_pCheckboxBusy=nullptr;    // autohotkey uses this to find out if it has to wait before sending more mouse/text
static const wxString ssCheckBoxBusy   = "CommunicationCheckBox";
static const wxString ssCheckBoxBusyMC = "CommunicationCheckBoxMC";
static       wxString ssWinTitle;       // set in app::init() to get it translated

static bool sbIdleHandleBusy= true;     // if set, next idle will uncheck Busy
static bool sbBusySet       = false;    // we already have started the BusyCycle

#define DEBUG_BUSY if (1)

static void SetReady(wxWindow* pWin, int count)
{   //set busy off after <count> CallAfter()
    if (count > 0 )
        pWin->CallAfter([pWin, count]{SetReady(pWin, count-1);});
    else
    {
        DEBUG_BUSY MyLogDebug("Busy(0)");
        sbBusySet           = false;    // not busy anymore, can be activated again
        sbIdleHandleBusy    = true;     // let idle do the uncheck, so grid is handled more properly in autotest
        //g_pCheckboxBusy->SetValue(0); // uncheck -> not busy == ready
    }
}   // SetReady()

void AutoTestBusy(wxWindow* a_pWin, const wxString& a_msg)
{
    if (!cfg::IsScriptTesting()) return;
    if (sbBusySet) return;            // we set it already ourself, no need to do it again
    if (sbIdleHandleBusy)
        MyLogError(_("Help, new busy request while previous not yet handled by Idle???"));
    g_pCheckboxBusy->SetValue(1);   // check
    DEBUG_BUSY MyLogDebug("Busy(1:'%s')", a_msg);
    if (a_pWin != nullptr)
    {   // atleast 2* CallAfter() to ensure(?) that all is really finished
        sbBusySet = true;
        SetReady(a_pWin, 4);
    }
}   // AutoTestBusy()

void AutoTestBusyMC(bool a_bSetCheck)
{
    if (!cfg::IsScriptTesting()) return;
    g_pCheckboxBusyMC->SetValue(a_bSetCheck);
}   // AutoTestBusyMC()

static int idleCount = 0;
void OnIdle()
{
    ++idleCount;
    if (sbIdleHandleBusy)
    {   // first idle after cmdready will reset busy checkbox
        sbIdleHandleBusy= false;
        int checked     = g_pCheckboxBusy->IsChecked();  // should be set...
        g_pCheckboxBusy->SetValue(0);   // uncheck -> not busy == ready
        DEBUG_BUSY MyLogDebug("Idle(%i), busy: %i --> 0",idleCount, checked);
    }
}   // OnIdle()

class MyFrame;

class MyApp : public wxApp
{
public:
    MyApp() { m_pMainFrame = 0; m_pLocale = nullptr; }
    ~MyApp() { wxDELETE(m_pLocale); }
    bool OnInit() override;
    virtual int OnExit();
    void InitLanguage();        // setup the language
    void ReInitLanguage();      // re-setup the language AND the mainframe/topwindow
private:
    MyFrame*  m_pMainFrame;
    wxLocale* m_pLocale;

};  // class MyApp

int MyApp::OnExit()
{
    return 0;
}
wxIMPLEMENT_APP(MyApp);

class EventCatcher : public wxEventFilter
{   // used while auto-testing to see/findout why events are not there where expected!
public:
    EventCatcher()
    {
        wxEvtHandler::AddFilter(this);
        m_bBusy             = false;
        m_timeStamp         = 0;
        m_previousTimeStamp = 0;
        m_previousChar      = 0;
        m_keys2String[WXK_BACK        ] = "{BS}";
        m_keys2String[WXK_RETURN      ] = "{ENTER}";
        m_keys2String[WXK_TAB         ] = "{TAB}";
        m_keys2String[WXK_ESCAPE      ] = "{ESC}";
        m_keys2String[WXK_DELETE      ] = "{DEL}";
        m_keys2String[WXK_SHIFT       ] = "{SHIFT}";
        m_keys2String[WXK_ALT         ] = "{ALT}";
        m_keys2String[WXK_CONTROL     ] = "{CTRL}";
        m_keys2String[WXK_END         ] = "{END}";
        m_keys2String[WXK_HOME        ] = "{HOME}";
        m_keys2String[WXK_LEFT        ] = "{LEFT}";
        m_keys2String[WXK_UP          ] = "{UP}";
        m_keys2String[WXK_RIGHT       ] = "{RIGHT}";
        m_keys2String[WXK_DOWN        ] = "{DOWN}";
        m_keys2String[WXK_NUMPAD_HOME ] = "{NUM_HOME}";
        m_keys2String[WXK_NUMPAD_END  ] = "{NUM_END}";
        m_keys2String[WXK_NUMPAD_UP   ] = "{NUM_UP}";
        m_keys2String[WXK_NUMPAD_DOWN ] = "{NUM_DOWN}";
        m_keys2String[WXK_NUMPAD_LEFT ] = "{NUM_LEFT}";
        m_keys2String[WXK_NUMPAD_RIGHT] = "{NUM_RIGHT}";
        m_keys2String[WXK_NUMPAD_ENTER] = "{NUM_ENTER}";
    }

    virtual ~EventCatcher()
    {
        wxEvtHandler::RemoveFilter(this);
    }   // ~EventCatcher()

    wxString Key2String(int key)
    {
        auto it = m_keys2String.find(key);
        if (it != m_keys2String.end())
            return it->second;
        return FMT("<%04X>", key);
    }   // Key2String()

    virtual int FilterEvent(wxEvent& event)
    {
        #define KEYDOWN     10070   /*wxEVT_KEY_DOWN */
        #define MOUSE_LD    10052   /*wxEVT_LEFT_DCLICK*/
        #define MOUSE_L     10043   /*wxEVT_LEFT_DOWN*/
        #define CHAR        10066   /*wxEVT_CHAR */
        #define CHARHOOK    10068   /*wxEVT_CHARHOOK */
        #define KEYUP       10071   /*wxEVT_KEY_UP */
        #define COMMAND     10148   /*wxCommandEvent*/
        #define MOTION      10049   /*wxEVT_MOTION*/
        #define CURSOR      10073   /*wxSetCursorEvent: clicking radiobutton*/
        #define MOUSEWHEEL  10050   /*wxMouseEvent :wxEVT_ENTER_WINDOW: clicking radiobutton*/
        /*wxEVT_NAVIGATION_KEY wxEVT_RIGHT_DOWN  wxEVT_MIDDLE_DOWN wxIdleEvent*/

        if (m_bBusy)
        {   // recursive call or call when I am busy??????
            //Don't output anything now: you WILL get lots of debug-assertions
            // wxCommandEvent
            return Event_Skip;
        }

        m_bBusy  = true;
        int type = event.GetEventType();
        // Can't use switch(): eventid's are no defines, but dynamically assigned
        if (type == wxEVT_KEY_DOWN || type == wxEVT_KEY_UP)
        {
            bool    bUp = true;
            wxChar  chr = (static_cast<wxKeyEvent*>(&event))->GetKeyCode();
            if (type == wxEVT_KEY_DOWN)
            {
                bUp = false;
                long timeStamp = event.GetTimestamp();
                if ( timeStamp == m_timeStamp)
                {
                    if (0) if (chr == m_previousChar)
                    {
                        m_bBusy = false;
                        return Event_Skip;      // same event, other source?
                    }
                }
                m_previousTimeStamp = m_timeStamp;
                m_timeStamp = timeStamp;
            }

            m_previousChar = chr;
            if ((chr >= ' ') && (chr < 127))
                m_keys = chr;
            else
                m_keys = Key2String(chr);
            //wxChar updown = bUp ? 0x02C4 : 0x02C5; //  '^'  'v'
            wxChar updown = bUp ? 0x2191 : 0x2193; //  arrow-up/down
            m_keys += updown;
            MyLogDebug("%s", m_keys);
        } else
        if (type == wxEVT_LEFT_DOWN || type == wxEVT_LEFT_DCLICK)
        {
            bool            bChecked1   = g_pCheckboxBusy  ->IsChecked();
            bool            bChecked2   = g_pCheckboxBusyMC->IsChecked();
            wxMouseEvent*   evt         = dynamic_cast<wxMouseEvent*>(&event);
            auto            pos         = evt->GetPosition();
            auto            pWin        = dynamic_cast<wxWindow*>(evt->GetEventObject());
            auto            screenPos   = pWin->ClientToScreen(pos);
            MyLogDebug("%cMouse(%i,%i), busy: %i,%i", type == wxEVT_LEFT_DOWN ? 'L' : 'D', screenPos.x, screenPos.y, bChecked1, bChecked2);
        } else
        if (    type == wxEVT_LEFT_UP
            ||  type == wxEVT_RIGHT_DOWN
            ||  type == wxEVT_RIGHT_UP
            ||  type == wxEVT_RIGHT_DCLICK
            ||  type == wxEVT_MIDDLE_DOWN
            ||  type == wxEVT_MIDDLE_UP
           )
        {   // mouse
        } else
        if (dynamic_cast<wxCommandEvent*>(&event))   // gives 'reentrancy' problems
        {
        } else
        if (    type == wxEVT_IDLE
            ||  type == wxEVT_MOTION
            ||  type == wxEVT_SET_CURSOR
            ||  type == wxEVT_MOUSE_CAPTURE_CHANGED
            ||  type == wxEVT_MOUSE_CAPTURE_LOST
            ||  type == wxEVT_LEAVE_WINDOW
            ||  type == wxEVT_ENTER_WINDOW
            ||  type == wxEVT_PAINT
            ||  type == wxEVT_ERASE_BACKGROUND
            ||  type == wxEVT_NC_PAINT
            ||  type == wxEVT_MOUSEWHEEL
            ||  type == wxEVT_SIZE
            ||  type == wxEVT_SIZING
            ||  type == wxEVT_MOVE
            ||  type == wxEVT_SET_FOCUS
            ||  type == wxEVT_KILL_FOCUS
            ||  type == wxEVT_ACTIVATE
            ||  type == wxEVT_THREAD
            ||  type == wxEVT_ASYNC_METHOD_CALL
            ||  type == wxEVT_MENU_OPEN
            ||  type == wxEVT_MENU_CLOSE
            ||  type == wxEVT_MENU_HIGHLIGHT
            ||  type == wxEVT_SHOW
            ||  type == wxEVT_CHAR
            ||  type == wxEVT_CHAR_HOOK
            ||  type == wxEVT_AFTER_CHAR
            ||  type == wxEVT_ACTIVATE_APP
            ||  type == wxEVT_MOVE_START
            ||  type == wxEVT_TIMER
            ||  type == wxEVT_MOVE_END 
            ||  type == wxEVT_ICONIZE
            )
        {
            ;   // nothing
        } else
        {   // unknown/unseen yet...
            if (1)
            {
                bool bChecked1 = g_pCheckboxBusy  ->IsChecked();
                bool bChecked2 = g_pCheckboxBusyMC->IsChecked();
                MyLogDebug("Event(%i), busy: %i,%i", type, bChecked1, bChecked2);
            }
        }

        // Continue processing the event normally as well.
        m_bBusy = false;
        return Event_Skip;
    }   // FilterEvent()

private:
    wxString m_keys;
    bool     m_bBusy;   // check for re-entrancy
    long     m_timeStamp;
    long     m_previousTimeStamp;
    wxChar   m_previousChar;
    std::map <int,wxString>  m_keys2String;
};

static wxCommandEvent s_dummy; 
class MyFrame : public wxFrame
{
public:
    explicit MyFrame(MyApp& theApp);
    ~MyFrame();
    void UpdateStatusbarInfo(wxCommandEvent& event=s_dummy);
    void UpdateStatusbarText(wxCommandEvent& a_event);
    UINT GetCurrentMenuId() {return m_oldId;}
    void SetClock(wxCommandEvent& event=s_dummy);
    MyStatusBar* StatusBar() {return m_pStatusbar;}
    wxMenuBar* m_pMenuBar;

private:
    void OnSystemInfo   (wxCommandEvent& event);
    void OnExit         (wxCommandEvent& event);
    void OnAbout        (wxCommandEvent& event);
    void OnPrintPage    (wxCommandEvent&);
    void OnMenuChoice   (wxCommandEvent& event);
    void OnLogging      (wxCommandEvent& event);
    void OnPrintFile    (wxCommandEvent& event);
    void OnImportSchema (wxCommandEvent& event);
    void OnLanguage     (wxCommandEvent& event);
    void AutotestCreatePositions();
    void LoadExistingSchemaFiles();

    Cleanup         m_theCleaner;   // clean wx-stuff before app exits to prevent crashes
    MyStatusBar*    m_pStatusbar;
    Baseframe*      m_pActivePage;
    MyLog           m_myLogger; // use wxLogWindow(): does the same and can stop msg passtrough!
    wxBoxSizer*     m_vSizer;
    
    EventCatcher*   m_pMyEventCatcher;
    MyApp&          m_theApp;
    std::map<UINT, Baseframe*> m_pages; // all created pages
    UINT            m_oldId;

};  // class MyFrame
 
MyFrame::~MyFrame()
{
    if (m_pActivePage) m_pActivePage->BackupData();
    delete m_pMyEventCatcher;
//    delete g_pCheckboxBusy;       // destroyed by MyFrame?
//    delete g_pCheckboxBusyMC;     // destroyed by MyFrame?
}   // ~MyFrame()

static void UncheckLogMenu()
{
    (reinterpret_cast<MyFrame*>(spMainframe))->m_pMenuBar->Check(ID_MENU_LOG, false);
}   // UncheckLogMenu()

#include <wx/dir.h>
static wxString sLocales = "locales";   // folder name in .exe folder where translations are stored

static void MakeDir(const wxString& a_dir)
{
    if (!wxDirExists(a_dir))
    {
        wxFileName::Mkdir(a_dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
}   // end MakeDir()

static wxString GetCatalogPath()
{   // get the catalog path and assure sub-folders 'nl' and 'en' exist
    wxString path = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + PS + sLocales;
    MakeDir(path);
    MakeDir(path + PS + "nl");
    MakeDir(path + PS + "en");
    return path;
}   // GetCatalogPath()

static void GetInstalledLanguages(wxArrayString & a_descriptions, wxArrayInt & a_identifiers)
{
    a_descriptions.Clear();
    a_identifiers.Clear();
    a_descriptions.Add(_("Use buildin language: 'English'"));
    a_identifiers.Add(wxLANGUAGE_ENGLISH);
    wxDir       dir(GetCatalogPath());
    wxString    filename;
    for (bool bFound = dir.GetFirst(&filename, "*.*", wxDIR_DIRS); bFound; bFound = dir.GetNext(&filename))
    {
        MyLogMessage(_("Search for languages, folder found: '%s'"), dir.GetNameWithSep() + filename);
        auto langinfo = wxLocale::FindLanguageInfo(filename);
        if (langinfo != nullptr)
        {
            if (wxFileExists(dir.GetName() + PS + filename + PS + __PRG_NAME__ + ".mo"))
            {
                wxString description = langinfo->Description;
                wxString native      = langinfo->DescriptionNative;
                if (description != native)
                   description += " '" + native + "'";
                a_descriptions.Add(description);
                a_identifiers.Add(langinfo->Language);
            }
        }
    }
}   // GetInstalledLanguages()

void MyApp::InitLanguage()
{
    auto language = cfg::GetLanguage();
    wxDELETE(m_pLocale);
    m_pLocale = new wxLocale;   // .init() can only be done ONCE!
    m_pLocale->AddCatalogLookupPathPrefix(GetCatalogPath());
    m_pLocale->Init(language);
    m_pLocale->AddCatalog(__PRG_NAME__, wxLANGUAGE_DUTCH_NETHERLANDS);
//    wxSetlocale(LC_NUMERIC, "C"); //"en_US.UTF-8");// "C" /*"en"*/);  // need '.' as decimal separator in float values in config files
}   // InitLanguage()

void MyApp::ReInitLanguage()
{
    cfg::FLushConfigs();
    InitLanguage();
#if 1
    //
    // experimental
    //
    // following should work!, but there are (too) many static vars that are not re-initialized!
    // only texts that are now used for the first time will have the new language.
    // So to be sure, just restart the app!

    // recreate the mainframe with the new language
    //wxWindow* pTopwindow = m_pMainFrame;   // GetTopWindow(); // SHOULD give m_pMainframe
    SetTopWindow(nullptr);
    m_pMainFrame->Close();
    m_pMainFrame->Destroy();  // system will delete it when idle....
    auto previousMenuId = m_pMainFrame->GetCurrentMenuId();
    m_pMainFrame = new MyFrame(*this);
    if (previousMenuId != m_pMainFrame->GetCurrentMenuId())
    {
        wxCommandEvent event(wxEVT_MENU, previousMenuId);
        m_pMainFrame->GetEventHandler()->AddPendingEvent(event);
    }

    m_pMainFrame->Show(true);
    SetTopWindow(m_pMainFrame);
#endif

    const wxLanguageInfo* const langInfo = wxUILocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
    if (langInfo)
        MyLogMessage(_("Systemlanguage: id=%d, %s"), langInfo->Language, langInfo->Description);
    else
        MyLogMessage(_("Systemlanguage: the default system language"));

    score::InitTexts4Translation(true);   // force re-init of static texts for translation
}   // ReInitLanguage()

bool MyApp::OnInit()
{
    if (!wxUILocale::UseDefault())
    {;}
    //Need the stupid wxLogWindow, else you get many error-popups!
    auto logWindow = new wxLogWindow(0, "Logwindow", false, false /*pass trough*/);
    logWindow->SetVerbose(TRUE);
    wxLog::SetActiveTarget(logWindow);
    logWindow->SetLogLevel(255);// wxLOG_FatalError);
    //logWindow->Show();

    SetVendorName("HarrieL");
    SetAppName(__PRG_NAME__);     // not needed, it's the default value
    SetConsoleOutputCP(437);
    SetConsoleCP(437);
    // enable console output
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        (void)freopen("CONOUT$", "w", stdout);
        (void)freopen("CONOUT$", "w", stderr);
    }

    std::cout << std::endl;

    InitLanguage();   // setup the language as requested by cfg

    wxArrayString myArgv;
    for (auto ii = 0; ii < argc; ++ii)
        myArgv.push_back(argv[ii]);
    if (CFG_ERROR == cfg::HandleCommandline(myArgv))
    {
        ;   // ??
    }

    //    wxLog::EnableLogging(false);

    m_pMainFrame = new MyFrame(*this);
    m_pMainFrame->Show(true);
    SetTopWindow(m_pMainFrame);

    LogDebug(_("MyApp::OnInit() finished, showing startup picture"));

    return true;
}   //  OnInit()

MyFrame::MyFrame(MyApp& a_theApp) : wxFrame(nullptr, wxID_ANY, ssWinTitle = _("'Bridge' scoring program")) // ssWinTitle: init here because of translation for new frame!
    , m_pActivePage { 0 }
    , m_theApp      { a_theApp }
    , m_oldId       { 0 }
{   // remark: wxFrame() MUST be initialized in constructor, not in its body: it will not be (for sure) the first toplevel window!

    MyLog::SetLevel(MyLog::Level::LOG_Max);
    MyLog::SetMainFrame(this);
    MyLog::SetCallbackOnHide(&UncheckLogMenu);
    // autotest init
    m_pMyEventCatcher = nullptr;
    if (cfg::IsScriptTesting())
    {
        g_pCheckboxBusy   = new wxCheckBox(this, wxID_ANY, ssCheckBoxBusy  , GetStaticRectPosition(), GetStaticRectSize()); g_pCheckboxBusy->Hide();
        g_pCheckboxBusyMC = new wxCheckBox(this, wxID_ANY, ssCheckBoxBusyMC, GetStaticRectPosition(), GetStaticRectSize()); g_pCheckboxBusyMC->Hide();

        g_pCheckboxBusy  ->Bind(wxEVT_CHECKBOX,[this](wxCommandEvent&){ MyLogDebug("Busy -> %i"  ,g_pCheckboxBusy  ->IsChecked());});
        g_pCheckboxBusyMC->Bind(wxEVT_CHECKBOX,[this](wxCommandEvent&){ MyLogDebug("BusyMC -> %i",g_pCheckboxBusyMC->IsChecked());});


        SetExtraStyle(wxWS_EX_PROCESS_IDLE);
        wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED); //  send only idle events to MyFrame
        Bind(wxEVT_IDLE,[](wxIdleEvent&){OnIdle();});
        m_pMyEventCatcher = new EventCatcher;           // we want early notice of mouseclick

        // create hotkey for generating mousepositions of controls for autotest in file "<matchfolder>/AutoTest.pos"
        #define HOTKEY "!+a\"         ; SHIFT+ALT+A" /* AHK2 definition*/
        RegisterHotKey(0, wxMOD_ALT | wxMOD_SHIFT, 'A'); // wxMOD_CONTROL doesn't work...
        Bind(wxEVT_HOTKEY, [this](wxKeyEvent&){AutotestCreatePositions();});
    }

    // check if cmdline has fontscaling setting 'q'
    float scale = 1.0;
    UINT fontIncrease = cfg::GetFontsizeIncrease();
    if (fontIncrease)
    {
        scale = 1.0 + fontIncrease/100.0;
        this->SetFont(GetFont().Scale(scale));      // only sub-frames have adapted fonts, menu's don't change: system settings
        MyLogDebug("Setting scalefactor to %3.1f", scale);
        MyLog::FontScale(scale);                    // also scale logwindow: it was created earlier
    }

    // Set default size of mainwindow
    // will give +/- 1040*560 size on 1920*1080 screen with 125% scalefactor: 1536=1920/1.25 and 864=1080/1.25
    // will give +/- 1040*560 size on 1920*1080 screen with 125% scalefactor: 1536=1920/1.25 and 821=(1080-taskbar)/1.25
    #define HSIZE_WANTED 900 /*1040*/
    #define VSIZE_WANTED 450 /*560*/

    auto displayRect = wxGetClientDisplayRect();
    int maxX = displayRect.width;
    int maxY = displayRect.height;
    int hSize = (maxX * HSIZE_WANTED + 1536/2)/1536;
        hSize = hSize * scale;
        hSize = std::min(hSize,maxX);
    int vSize = (maxY * VSIZE_WANTED +  821/2)/ 821;
        vSize = vSize * scale;
        vSize = std::min(vSize,maxY);

    hSize = std::min(FromDIP(HSIZE_WANTED*scale), maxX-20);
    vSize = std::min(FromDIP(VSIZE_WANTED*scale), maxY-20);
    SetSize({hSize,vSize});

    static wxPoint pos{ {0,0} };
    if (pos == wxPoint({0, 0})) pos = GetPosition();    // use same position after language change
    if (pos.x + hSize > maxX ) pos.x -= pos.x +hSize - maxX;
    if (pos.y + vSize > maxY ) pos.y -= pos.y +vSize - maxY;
    SetPosition(pos);
    //SetPosition({10,10}); // testing mouse-coordinates

    MyLogDebug(_("Mainwindow position: {%i,%i}, size: %i*%i"), pos.x, pos.y, hSize, vSize);

    LogMessage("------------"); // just testing...
    LogMessage("test logging");
    LogError  ("Error");
    LogWarning("Warning");
    LogInfo   ("Info");
    LogVerbose("Verbose");
    LogDebug  ("Debug");
    LogMessage("------------");

    SetIcon(wxICON(wxwin_standard_icon));
    spMainframe = this;    // for clients to reach us

    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_MENU_SETUPNEWMATCH  , _("&New match/session"  ), _("Match/session entry"            ));
    menuFile->Append(ID_MENU_SETUPPRINTER   , _("&Printer choice"     ), _("Printer choice and setup"       ));
    menuFile->Append(ID_MENU_PRINTPAGE      , _("print pa&Ge"         ), _("Print current page"             ));
    menuFile->Append(ID_MENU_PRINTFILE      , _("print &File"         ), _("Print a file"                   ));

    menuFile->AppendSeparator();
    menuFile->Append(ID_EXIT                , _("&Exit"               ), _("This will end the program"      ));
 
    wxMenu *menuSettings = new wxMenu;
    menuSettings->Append(ID_MENU_SETUPGAME  , _("&Match"                 ), _("Setup for the active match"                      ));
    menuSettings->Append(ID_MENU_SETUPSCHEMA, _("&Schema"                ), _("Entry/change of schema"                          ));
    menuSettings->Append(ID_MENU_NAMEEDITOR , _("pairnames &Entry/change"), _("Entry/change of pair/clubnames"                  ));
    menuSettings->Append(ID_MENU_ASSIGNNAMES, _("pairnames &Assigment"   ), _("Connect a global pairname to a sessionpairnumber"));
    
    
    wxMenu *menuScores = new wxMenu;
    menuScores->Append(ID_MENU_SCORE_ENTRY      , _("s&Core-entry"       ), _("Entry/change of scores"             ));
    menuScores->Append(ID_MENU_COR_ENTRY_SESSION, _("&Sessioncorrections"), _("Entry/change of session corrections"));
    menuScores->Append(ID_MENU_COR_ENTRY_END    , _("&Endcorrections"    ), _("Entry/change of end corrections"    ));
    menuScores->Append(ID_MENU_CALC_SCORES      , _("&Results"           ), _("Calculation of session/end result"  ));
    
    wxMenu *menuExtra = new wxMenu;
    menuExtra->AppendCheckItem(ID_MENU_LOG      , _("&Log window"               ), _("Enable/disable logging window"     ));
    menuExtra->Append(ID_MENU_DEBUG             , _("&Debug window"             ), _("Debug window for all kind of stuff"));
    menuExtra->Append(ID_MENU_DEBUG_GUIDES      , _("&Guides"                   ), _("Creation of guides"                ));
    menuExtra->Append(ID_MENU_DEBUG_SCORE_SLIPS , _("&Scoreslips"               ), _("Creation of scoreslips"            ));
    menuExtra->Append(ID_MENU_IMPORT_SCHEMA     , _("&Import schema"            ), _("Import a new schema (.asc) for playing"));
    auto convertData = new wxMenu;
    convertData->Append(ID_MENU_OLD_TO_DBASE  , _("&Old -> database"           ), _("Convert 'old' .ini/data files to single new .db file" ));
    convertData->Append(ID_MENU_DBASE_TO_OLD  , _("&Database -> old"           ), _("Convert .db file to 'old' .ini/data files"            ));
    menuExtra->AppendSubMenu(convertData      , _("&Convert data"              ), _("convert match-data between old/new types"             ));
    auto otherDb = new wxMenu;
    otherDb->AppendRadioItem(ID_MENU_OLD_DBASE, _("&Old datatype (.ini)"       ), _("Use 'old' .ini/data files for data storage"           ));
    otherDb->AppendRadioItem(ID_MENU_NEW_DBASE, _("&New datatype (.db)"        ), _("Use new .db file for data storage"                    ));
    menuExtra->AppendSubMenu(otherDb          , _("s&Witch between datatypes"  ), _("switch between the two datatypes"                     ));
    menuExtra->Append(ID_MENU_SLIP_SERVER     , _("&Slip server"               ), _("Server for receiving slip-data"                       ));
    menuExtra->Append(ID_MENU_LANGUAGE        , _("&Language"                  ), _("language of the userinterface"                        ));

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(ID_SYSTEM_INFO         , _("&System info"                 ), _("Info about the version of wxWidgets"));
    menuHelp->Append(ID_ABOUT               , _("&About ") + __PRG_NAME__ );
 
    m_pMenuBar = new wxMenuBar;
    m_pMenuBar->Append(menuFile,    _("&File"     ));
    m_pMenuBar->Append(menuSettings,_("&Settings" ));
    m_pMenuBar->Append(menuScores,  _("s&Cores"   ));
    m_pMenuBar->Append(menuExtra,   _("&Tools"    ));
    m_pMenuBar->Append(menuHelp,    _("&Help"     ));
 
    SetMenuBar( m_pMenuBar );

    auto type = io::DatabaseTypeGet();
    m_pMenuBar->Check( type == io::DB_ORG ? ID_MENU_OLD_DBASE : ID_MENU_NEW_DBASE, true);
 
    m_pStatusbar = new MyStatusBar(this);
    SetStatusBar(m_pStatusbar);
    SetStatusText(_("Welcome at ") +  __PRG_NAME__);
    UpdateStatusbarInfo();

    Bind(wxEVT_MENU, &MyFrame::OnSystemInfo,   this, ID_SYSTEM_INFO);
    Bind(wxEVT_MENU, &MyFrame::OnAbout,        this, ID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit ,        this, ID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnPrintPage,    this, ID_MENU_PRINTPAGE);
    Bind(wxEVT_MENU, &MyFrame::OnPrintFile,    this, ID_MENU_PRINTFILE);
    Bind(wxEVT_MENU, &MyFrame::OnImportSchema, this, ID_MENU_IMPORT_SCHEMA);
    Bind(wxEVT_MENU, &MyFrame::OnMenuChoice,   this, ID_MENU_SETUP_FIRST, ID_MENU_SETUP_LAST);
    Bind(wxEVT_MENU, &MyFrame::OnLogging,      this, ID_MENU_LOG);
    Bind(wxEVT_MENU, [this](wxCommandEvent&){AUTOTEST_BUSY("menu"); io::ConvertDataBase(io::FromOldToDb); }, ID_MENU_OLD_TO_DBASE);
    Bind(wxEVT_MENU, [this](wxCommandEvent&){AUTOTEST_BUSY("menu"); io::ConvertDataBase (io::FromDbToOld);}, ID_MENU_DBASE_TO_OLD);
    Bind(wxEVT_MENU, [this](wxCommandEvent&){AUTOTEST_BUSY("menu"); cfg::DatabaseTypeSet(io::DB_ORG)     ;}, ID_MENU_OLD_DBASE);
    Bind(wxEVT_MENU, [this](wxCommandEvent&){AUTOTEST_BUSY("menu"); cfg::DatabaseTypeSet(io::DB_DATABASE);}, ID_MENU_NEW_DBASE);
    Bind(wxEVT_USER, &MyFrame::UpdateStatusbarInfo  , this, ID_STATUSBAR_UPDATE);
    Bind(wxEVT_USER, &MyFrame::UpdateStatusbarText  , this, ID_STATUSBAR_SETTEXT);
    Bind(wxEVT_USER, &MyFrame::SetClock             , this, ID_UPDATE_CLOCK);
    Bind(wxEVT_MENU, &MyFrame::OnLanguage           , this, ID_MENU_LANGUAGE);

    if ( cfg::IsDebug() )
    {
        std::cout << '\n' << _("hello in the console!") << std::endl;

        // default logging goes to a logger window
        LogDebug(_("Debug active"));
    }

    //all initialized, now show us!
    ShowStartImage(this);
    LoadExistingSchemaFiles();  // load all known imported schemafiles
}   // MyFrame()
 
void MyFrame::OnExit(wxCommandEvent& )
{
    Close(true);
}   //  OnExit()
 
void MyFrame::OnPrintPage(wxCommandEvent&)
{
    AUTOTEST_BUSY("printPage");
    MyLogDebug(_("PrintPage()"));
    if (m_pActivePage) m_pActivePage->PrintPage();
}   // OnPrintPage()

void MyFrame::OnAbout(wxCommandEvent& )
{
// next include produced by buildDate.exe gives: static const char* buildDate = "woensdag 22 november 2023 @ 12:08:24";
#include "buildDate.h"
  
    wxString about;
    about.Printf(   __PRG_NAME__ + _(", version ") + __VERSION__ + _(", from ") + __YEAR__ +
                    _("\nThis is the 'bridge' scoring program of Harrie/Tinus\n"
                      "on base of %s\n\n"
                      "Build date: %s\n%s\n\n"
                      "(c) wxSystemInformationFrame, PB: https://github.com/PBfordev"
                    ), wxVERSION_STRING, wxString::FromUTF8(buildDate), cfg::GetCopyright()
                );

    MyMessageBox( about, _("About ") + __PRG_NAME__ , wxOK | wxICON_INFORMATION);
}   // OnAbout()
 
#include "wxsysinfoframe.h"
void MyFrame::OnSystemInfo(wxCommandEvent& )
{
    wxSystemInformationFrame* frame = new wxSystemInformationFrame(this);
    frame->Show();
//    wxInfoMessageBox(0);
}   // OnSystemInfo()

void SetStatusbarInfo()
{
    wxString tmp = cfg::GetActiveMatchAndSession() + ", ";
    wxString prn = cfg::GetPrinterName();
    if (prn == cfg::GetFilePrinterName())
        tmp +=  cfg::GetFilePrinterName();
    else
        tmp += cfg::GetWinPrintPrefix();
    reinterpret_cast<MyFrame*>(spMainframe)->StatusBar()->SetInfo(tmp);
}   // SetStatusbarInfo()

void MyFrame::UpdateStatusbarInfo(wxCommandEvent& )
{
    SetStatusbarInfo();
}   //  UpdateStatusbarInfo()

void SetStatusbarText(const wxString& a_msg)
{
    reinterpret_cast<MyFrame*>(spMainframe)->StatusBar()->SetStatusText(a_msg);
}   // SetStatusbarText()

void MyFrame::UpdateStatusbarText(wxCommandEvent& a_event)
{
    auto msg = reinterpret_cast<const wxString*>(a_event.GetClientData());
    m_pStatusbar->SetStatusText(*msg);
}   // UpdateStatusbarText()

void MyFrame::SetClock(wxCommandEvent& /*event*/)
{
    m_pStatusbar->SetClock();
}   // SetClock()

void MyFrame::AutotestCreatePositions()
{
    MyTextFile positionsFile;
    positionsFile.MyCreate(cfg::ConstructFilename(cfg::EXT_MAX)+"AutoTest.pos", MyTextFile::WRITE);
    positionsFile.AddLine(FMT(";definitions to use in automatic testing, created at %s", GetDateTime()));
    // NB do NOT translate the variable names themself, ONLY there value: Autotest relies on these names!
    positionsFile.AddLine(    "WinTitle       := \"" + ssWinTitle       + "\"   ;window title of this program");
    positionsFile.AddLine(    "SystemComBox   := \"" + ssCheckBoxBusy   + "\"   ;name of the checkbox for communication between autohotkey and the program");
    positionsFile.AddLine(    "SystemComBoxMC := \"" + ssCheckBoxBusyMC + "\" ;name of the checkbox for communication between autohotkey and the program for ChoiceMC");
    positionsFile.AddLine(    "HotkeyPos      := \""  HOTKEY  ": hotkey to request mouse-positions to be generated");
    positionsFile.AddLine(    "; menu-definitions");
    positionsFile.AddLine(    _("MenuNewMatch      := \"fn\"       ; File: New match/session"               ));
    positionsFile.AddLine(    _("MenuShutdown      := \"fe\"       ; File: Exit"                            ));
    positionsFile.AddLine(    _("MenuPrinter       := \"fp\"       ; File: Printer choice"                  ));
    positionsFile.AddLine(    _("MenuPrintPage     := \"fg\"       ; File: print paGe"                      ));
    positionsFile.AddLine(    _("MenuSetupMatch    := \"sm\"       ; Settings: setup Match"                 ));
    positionsFile.AddLine(    _("MenuSetupSchema   := \"ss\"       ; Settings: setup Schema"                ));
    positionsFile.AddLine(    _("MenuNamesInit     := \"se\"       ; Settings: pairnames Entry/change"      ));
    positionsFile.AddLine(    _("MenuNamesAssign   := \"sa\"       ; Settings: pairnames Assignment"        ));
    positionsFile.AddLine(    _("MenuScoreEntry    := \"cc\"       ; sCores: sCoreentry"                    ));
    positionsFile.AddLine(    _("MenuCorSession    := \"cs\"       ; sCores: entry Sesssioncorrections"     ));
    positionsFile.AddLine(    _("MenuCorEnd        := \"ce\"       ; sCores: entry Endcorrections"          ));
    positionsFile.AddLine(    _("MenuResult        := \"cr\"       ; sCores: calculate Results"             ));
    positionsFile.AddLine(    _("MenuDebug         := \"td\"       ; Tools: Debug window"                   ));
    positionsFile.AddLine(    _("MenuGuides        := \"tg\"       ; Tools: Guide creation"                 ));
    positionsFile.AddLine(    _("MenuScoreSlips    := \"ts\"       ; Tools: Scoreslip creation"             ));
    positionsFile.AddLine(    _("MenuImportSchema  := \"ti\"       ; Tools: Import a new playing-schema"    ));
    positionsFile.AddLine(    _("MenuDbType        := \"twn\"      ; Tools: sWitch between datatypes: New datatype" ));
    positionsFile.AddLine(    _("MenuOldType       := \"two\"      ; Tools: sWitch between datatypes: Old datatype" ));
    positionsFile.AddLine(    _("AllMenus          := [\"fn\",\"fp\",\"fg\",\"sm\",\"ss\",\"se\",\"sa\",\"cc\",\"cs\",\"ce\",\"cr\",\"two\",\"twn\"]"));
    positionsFile.AddLine(    ";\n;windowrelative-positions");
    positionsFile.AddLine(    ";s* vars are screen-coordinates, TL=TopLeft, TR=TopRight, BR=BottomRight\n;");
    positionsFile.AddLine(    ";some vars and there value in the current language");
    positionsFile.AddLine(    "sChanged  := \"" + _("changed") + '"');
    positionsFile.AddLine(    "sSession  := \"" + _("session") + '"');
    positionsFile.AddLine(    "sPair     := \"" + _("pair"   ) + '"');

  
    wxCommandEvent event;
    for (auto menuId = ID_MENU_SETUP_FIRST + 1; menuId < ID_MENU_SETUP_LAST; ++menuId)
    {
        event.SetId(menuId);
        OnMenuChoice(event);    // create/switch-to menupage
        Update();               // now screen should be uptodate!
        positionsFile.AddLine(";");                                     // visual separation between pages
        m_pActivePage->AutotestRequestMousePositions(&positionsFile);   // add mousepositions, if any
    }

    // after work, setup newmatch-page
    event.SetId(ID_MENU_SETUPNEWMATCH);
    OnMenuChoice(event);
}   // AutotestCreatePositions()

void MyFrame::OnMenuChoice(wxCommandEvent& a_event)
{
    MyLogDebug(ES);                             // separation between menu messages
    AUTOTEST_BUSY("menu");

    if (m_pages.size() == 0)
    {
        GetSizer()->Hide((size_t)0);            // firsttime call: remove/disable picture in mainframe
        GetSizer()->DeleteWindows();
        m_vSizer = new wxBoxSizer(wxVERTICAL);  // ONLY created when needed!!
        SetSizer(m_vSizer);                     // enable 'standard' sizer
    }
    /*
    * The current active page is asked to backup its data.
    * If the new page was created before, it gets a Refresh() command and is then shown.
    * A new page is created and shown, while the previous one is hidden.
    * The new page can/will then use the most uptodate data.
    */
    auto oldPage = m_pActivePage;       // we need to hide old page when we get a new one
    if (oldPage)
    {
        MyLogDebug(_("Menu backup current(%u: %s)"), m_oldId, oldPage->GetDescription());
        oldPage->BackupData();          // backup data BEFORE new page is created/shown
        io::DatabaseFlush();            // and update databases on disk
    }

    m_pStatusbar->SetStatusText(ES);
    UINT id = a_event.GetId();
    auto it = m_pages.find(id);         // check if we already created it

    if (it != m_pages.end())
    {   // already created, so m_pActivePage is non-zero
        MyLogDebug(_("Menu old page(%u: %s)"), id, it->second->GetDescription());
        if (m_pActivePage == it->second)
        {
            if (id == ID_MENU_SETUPNEWMATCH)
                m_pActivePage->RefreshInfo();   // we COULD have changed databasetype...
            return;                             // same page, so do nothing...
        }
        m_pActivePage = it->second;
        MyLogDebug(_("Menu old show(%u)"), id);
        m_pActivePage->RefreshInfo();           // update possible old info on existing page
        m_pActivePage->Show(true);              // show page
    }
    else
    {   // not found, create new page on demand
        MyLogDebug(_("Menu new page(%u)"), id);
        switch (id)
        {
            case ID_MENU_SETUPGAME:
                m_pActivePage = new SetupGame(this,ID_MENU_SETUPGAME);
                break;
            case ID_MENU_NAMEEDITOR:
                m_pActivePage = new NameEditor(this,ID_MENU_NAMEEDITOR);
                break;
            case ID_MENU_SETUPSCHEMA:
                m_pActivePage = new SetupSchema(this,ID_MENU_SETUPSCHEMA);
                break;
            case ID_MENU_SETUPPRINTER:
                m_pActivePage = new SetupPrinter(this,ID_MENU_SETUPPRINTER);
                break;
            case ID_MENU_SETUPNEWMATCH:
                m_pActivePage = new SetupNewMatch(this,ID_MENU_SETUPNEWMATCH);
                break;
            case ID_MENU_SCORE_ENTRY:
                    m_pActivePage = new ScoreEntry(this,ID_MENU_SCORE_ENTRY);
                break;
            case ID_MENU_ASSIGNNAMES:
                m_pActivePage = new AssignNames(this, ID_MENU_ASSIGNNAMES);
                break;
            case ID_MENU_COR_ENTRY_SESSION:
                m_pActivePage = new CorrectionsSession(this,ID_MENU_COR_ENTRY_SESSION);
                break;
            case ID_MENU_COR_ENTRY_END:
                m_pActivePage = new CorrectionsEnd(this,ID_MENU_COR_ENTRY_END);
                break;
            case ID_MENU_DEBUG:
                m_pActivePage = new Debug(this,ID_MENU_DEBUG);
                break;
            case ID_MENU_DEBUG_GUIDES:
                m_pActivePage = new Debug(this,ID_MENU_DEBUG_GUIDES,Debug::Guides);
                break;
            case ID_MENU_DEBUG_SCORE_SLIPS:
                m_pActivePage = new Debug(this,ID_MENU_DEBUG_SCORE_SLIPS, Debug::ScoreSlips);
                break;
            case ID_MENU_CALC_SCORES:
                m_pActivePage = new CalcScore(this,ID_MENU_CALC_SCORES);
                break;
            case ID_MENU_SLIP_SERVER:
                m_pActivePage = new SlipServer(this,ID_MENU_SLIP_SERVER);
                break;
            default:
                MyLogError(_("OnMenu(%u), id unknown!"), id);
                return; // unknow id, so leave all as is
        }

        MyLogDebug(_("Menu new show(%u: %s)"), id, m_pActivePage->GetDescription());
        m_pages[id] = m_pActivePage;    // remember this page
        m_vSizer->Add(m_pActivePage     //    and add to layout
            , 1                         // make vertically stretchable, relative size = 1
            , wxEXPAND                  // make horizontally stretchable
            , 0                         // set border width to 0
        );
    }

    if (oldPage) oldPage->Hide();       // hide old page so ONLY new page is visible one in m_vSizer
    m_oldId = id;                       // remember for next time
    Layout();                           // re-layout all
    Refresh();                          // possibly needed???
    Update();                           // refresh()+update() -> immediate show updated window
}   // OnMenuChoice()

void MyFrame::OnLogging(wxCommandEvent&)
{   // show/hide a logwindow 
    AUTOTEST_BUSY("log");
    bool bShow = m_pMenuBar->IsChecked(ID_MENU_LOG);
    MyLog::Show(bShow); // my own logger --> made this before I knew the wxLogWindow :(
    SetFocus();         // stay with me...
}   // OnLogging()

void MyFrame::OnLanguage(wxCommandEvent&)
{
    wxArrayString   names;
    wxArrayInt      identifiers;
    GetInstalledLanguages(names, identifiers);
    int index = MyGetSingleChoiceIndex(_("Choose a language (and restart)"), _("Language selection"), names, this);
    if (index < 0) return;  // cancel pressed


    int language = identifiers[index];
    if (language != cfg::GetLanguage())
    {   // setting same language would result in database close (if auto restart) and result in a lot of access-errors!
        if (m_pActivePage) m_pActivePage->BackupData();
        cfg::SetLanguage(language, names[index]);
        m_theApp.ReInitLanguage();  // can result in automatic restart!
        MyLogMessage(_("Languagechoice: id=%d, name=%s"), identifiers[index], names[index]); // will be shown in new logwindow, if restarted!
    }
}   // OnLanguage()

void MyFrame::OnPrintFile(wxCommandEvent& )
{
    AUTOTEST_BUSY("printFile");
    wxString filename = wxFileSelector(_("Choose a file for printing"));

    if ( !filename.IsEmpty() )
    {
        LogMessage(_("Printing file <%s>"), filename);
        prn::PrintAFile(filename);
    }
}   // OnPrintFile()

static auto const NBB_SCHEMA_EXTENSION("asc");  // Dutch Bridge Association

void MyFrame::OnImportSchema(wxCommandEvent& )
{
    AUTOTEST_BUSY("ImportSchema");
    wxString schemaFile = wxFileSelector(_("Choose a schema file to import")
                                            , ES, ES
                                            , NBB_SCHEMA_EXTENSION
                                            , ES
                                            , wxFD_FILE_MUST_EXIST
                                        );

    if ( !schemaFile.IsEmpty() )
    {
        MyLogMessage(_("Importing file <%s>"), schemaFile);
        wxFileName fn(schemaFile);
        if (0 == cfg::GetBaseFolder().CmpNoCase(fn.GetPath()))
        {
            wxString error(_("Don't import a schema from the basefolder itself!"));
            MyLogError(error);
            MyMessageBox(error, _("Error"));
        } else
        {   // only import if folders are different!
            if (schema::ImportSchema(schemaFile))
            {   // copy/overwrite file to/in the basefolder: load all schemafiles at startup from here
                wxString basename = wxFileName(cfg::GetBaseFolder(), fn.GetFullName()).GetFullPath();
                if (!wxCopyFile(schemaFile, basename, true))
                {
                    MyLogError("Could not copy <%s> to <%s>", schemaFile, basename);
                }
            }
        }
    }
}   // OnImportSchema()

void MyFrame::LoadExistingSchemaFiles()
{   // load all schemafiles that were successfully imported earlier
    wxArrayString schemas;
    wxString wild("*."); wild += NBB_SCHEMA_EXTENSION;
    (void)wxDir::GetAllFiles(cfg::GetBaseFolder(), &schemas, wild, wxDIR_FILES);
    for (const auto& schema : schemas)
    {
        (void)schema::ImportSchema(schema, true);   // always force-load schema's during startup!
    }
    schema::DebuggingSchemaData();  // for debugging....
}   // LoadExistingSchemaFiles()

void SendEvent2Mainframe(int a_id, void* a_pClientData)
{
    if (GetMainframe())
    {
        wxCommandEvent event(wxEVT_USER, a_id);
        event.SetClientData(a_pClientData);
        GetMainframe()->GetEventHandler()->AddPendingEvent(event);
    }
}   // SendEvent2Mainframe()

void SendEvent2Mainframe(wxWindow* a_pWindow, int a_id, void* const a_pClientData)
{
    wxCommandEvent event(wxEVT_USER, a_id);
    event.SetClientData(a_pClientData);
    a_pWindow->GetEventHandler()->AddPendingEvent(event);
}   // SendEvent2Mainframe()
