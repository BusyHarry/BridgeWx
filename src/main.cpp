﻿// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// Start of wxWidgets Bridge calc Program

#include <wx/image.h>
#include "wx/uilocale.h"
#include <map>
#include <wx/filename.h>
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
#include "fileIo.h"
#include "version.h"

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
    MyFrame* GetMainFrame(){return m_pMainFrame;}
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
        m_keys2String[0x0008] = "{BS}";
        m_keys2String['\r'  ] = "{ENTER}";
        m_keys2String[0x0009] = "{TAB}";
        m_keys2String[0x001b] = "{ESC}";
        m_keys2String[0x007f] = "{DEL}";
        m_keys2String[0x0132] = "{SHIFT}";
        m_keys2String[0x0133] = "{ALT}";
        m_keys2String[0x0134] = "{CTRL}";
        m_keys2String[0x0138] = "{END}";
        m_keys2String[0x0139] = "{HOME}";
        m_keys2String[0x013a] = "{LEFT}";
        m_keys2String[0x013b] = "{UP}";
        m_keys2String[0x013c] = "{RIGHT}";
        m_keys2String[0x013d] = "{DOWN}";
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
    wxMenuBar* m_pMenuBar;

private:
    void OnSystemInfo   (wxCommandEvent& event);
    void OnExit         (wxCommandEvent& event);
    void OnAbout        (wxCommandEvent& event);
    void OnPrintPage    (wxCommandEvent&);
    void OnMenuChoice   (wxCommandEvent& event);
    void OnLogging      (wxCommandEvent& event);
    void OnPrintFile    (wxCommandEvent& event);
    void OnLanguage     (wxCommandEvent& event);
    void AutotestCreatePositions();

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
    a_descriptions.Add(_("Gebruik default taal: 'Dutch/Nederlands'"));  // buildin language
    a_identifiers.Add(wxLANGUAGE_DUTCH_NETHERLANDS);
    wxDir       dir(GetCatalogPath());
    wxString    filename;
    for (bool bFound = dir.GetFirst(&filename, "*.*", wxDIR_DIRS); bFound; bFound = dir.GetNext(&filename))
    {
        MyLogMessage(_("Zoeken naar talen, folder gevonden: '%s'"), dir.GetNameWithSep() + filename);
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
    wxSetlocale(LC_NUMERIC, "C" /*"en"*/);  // need '.' as decimal separator in float values in config files
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
        MyLogMessage(_("Systeemtaal: %d, %s"), langInfo->Language, langInfo->Description);
    else
        MyLogMessage(_("Systeemtaal: de default systeem taal"));

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

MyFrame::MyFrame(MyApp& a_theApp) : wxFrame(nullptr, wxID_ANY, ssWinTitle = _("Bridge rekenprogramma")) // ssWinTitle: init here because of translation for new frame!
    , m_pActivePage { 0 }
    , m_theApp      { a_theApp }
    , m_oldId       { 0 }
{   // remark: wxFrame() MUST be initialized in constructor, not in its body: it will not be (for sure) the first toplevel window!

    MyLogSetLevel(MyLogLevel::MyLOG_Max);
    m_myLogger.SetCallbackOnHide(&UncheckLogMenu);
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
        #define HOTKEY "!+a"    /* AHK2 definition*/
        RegisterHotKey(0, wxMOD_ALT | wxMOD_SHIFT, 'A'); // wxMOD_CONTROL doesn't work...
        Bind(wxEVT_HOTKEY, [this](wxKeyEvent&){AutotestCreatePositions();});
    }

    // check if cmdline has fontscaling setting 'q'
    UINT fontIncrease = cfg::GetFontsizeIncrease();
    if (fontIncrease)
    {
        float factor = 1.0 + fontIncrease/100.0;
        this->SetFont(GetFont().Scale(factor));    // only sub-frames have adapted fonts, menu's don't change: system settings
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
        hSize = (hSize *(100+fontIncrease)+50)/100;
        hSize = std::min(hSize,maxX);
    int vSize = (maxY * VSIZE_WANTED +  821/2)/ 821;
        vSize = (vSize *(100+fontIncrease)+50)/100;
        vSize = std::min(vSize,maxY);

    hSize = FromDIP(HSIZE_WANTED);
    vSize = FromDIP(VSIZE_WANTED);
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
    menuFile->Append(ID_MENU_SETUPNEWMATCH  , _("&Nieuwe wedstrijd/zitting"  ), _("Wedstrijd/zitting ingave"            ));
    menuFile->Append(ID_MENU_SETUPPRINTER   , _("&Printerkeuze"              ), _("Printer keuze en instellingen"       ));
    menuFile->Append(ID_MENU_PRINTPAGE      , _("print pa&Gina"              ), _("Print huidige pagina"                ));
    menuFile->Append(ID_MENU_PRINTFILE      , _("print &Bestand"             ), _("Print een bestand"                   ));

    menuFile->AppendSeparator();
    menuFile->Append(ID_EXIT                , _("&Afsluiten"                 ), _("Dit zal het programma beeindigen"    ));
 
    wxMenu *menuSettings = new wxMenu;
    menuSettings->Append(ID_MENU_SETUPGAME  , _("&Wedstrijd instellingen"    ), _("Instellingen tbv de aktieve wedstrijd"           ));
    menuSettings->Append(ID_MENU_SETUPSCHEMA, _("opzetten &Schema"           ), _("Ingave/wijzigen van schema"                      ));
    menuSettings->Append(ID_MENU_NAMEEDITOR , _("paarnamen &Ingeven/wijzigen"), _("Ingave/wijzigen van paar/clubnamen"              ));
    menuSettings->Append(ID_MENU_ASSIGNNAMES, _("paarnamen &Toekennen"       ), _("Paarnaam koppelen aan paarnummer van zitting"    ));
    
    
    wxMenu *menuScores = new wxMenu;
    menuScores->Append(ID_MENU_SCORE_ENTRY      , _("ingave &Scores"            ), _("Ingeven/aanpassen scores"                        ));
    menuScores->Append(ID_MENU_COR_ENTRY_SESSION, _("ingave &Zitting correcties"), _("Ingeven/aanpassen correcties tbv een zitting"    ));
    menuScores->Append(ID_MENU_COR_ENTRY_END    , _("ingave &Eind correcties"   ), _("Ingeven/aanpassen correcties voor de einduitslag"));
    menuScores->Append(ID_MENU_CALC_SCORES      , _("berekenen &Uitslag"        ), _("Berekenen van de zitting/eind uitslag"           ));
    
    wxMenu *menuExtra = new wxMenu;
    menuExtra->AppendCheckItem(ID_MENU_LOG      , _("&Log window"               ), _("Aan / uitzetten van log window"                  ));
    menuExtra->Append(ID_MENU_DEBUG             , _("&Debug window"             ), _("Debug window voor allerlei zaken"                ));
    menuExtra->Append(ID_MENU_DEBUG_GUIDES      , _("&Gidsbriefjes"             ), _("Aanmaak van gidsbriefjes"                        ));
    menuExtra->Append(ID_MENU_DEBUG_SCORE_SLIPS , _("&Scoreslips"               ), _("Aanmaak van scoreslips"                          ));

    auto convertData = new wxMenu;
    convertData->Append(ID_MENU_OLD_TO_DBASE  , _("&Oud -> database"           ), _("Omzetten van 'oude' .ini/data bestanden naar een enkel .db bestand" ));
    convertData->Append(ID_MENU_DBASE_TO_OLD  , _("&Database -> oud"           ), _("Omzetten van .db bestand naar 'oude' .ini/data bestanden"        ));
    menuExtra->AppendSubMenu(convertData      , _("&Converteren data"          ), _("wedstrijdgegevens omzetten van het ene type naar het andere"));
    auto otherDb = new wxMenu;
    otherDb->AppendRadioItem(ID_MENU_OLD_DBASE, _("&Oud bestandstype (.ini)"    ), _("Gebruik van 'oude' .ini/data bestanden voor opslag"     ));
    otherDb->AppendRadioItem(ID_MENU_NEW_DBASE, _("&Nieuw bestandstype (.db)"   ), _("Gebruik van nieuw .db bestand voor opslag"         ));
    menuExtra->AppendSubMenu(otherDb          , _("&Wissel van bestandstype"    ), _("omschakelen van het ene opslagtype naar het andere"));
    menuExtra->Append(ID_MENU_LANGUAGE        , _("&Taal"                       ), _("taal van de gebruikers interface"));

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(ID_SYSTEM_INFO         , _("&Systeem info"                 ), _("Info over versie van wxWidgets"));
    menuHelp->Append(ID_ABOUT               , _("&Over ") + __PRG_NAME__ );
 
    m_pMenuBar = new wxMenuBar;
    m_pMenuBar->Append(menuFile,    _("&Bestand"      ));
    m_pMenuBar->Append(menuSettings,_("&Instellingen" ));
    m_pMenuBar->Append(menuScores,  _("&Scores"       ));
    m_pMenuBar->Append(menuExtra,   _("&Extra"        ));
    m_pMenuBar->Append(menuHelp,    _("&Help"         ));
 
    SetMenuBar( m_pMenuBar );

    auto type = io::DatabaseTypeGet();
    m_pMenuBar->Check( type == io::DB_ORG ? ID_MENU_OLD_DBASE : ID_MENU_NEW_DBASE, true);
 
    m_pStatusbar = new MyStatusBar(this);
    SetStatusBar(m_pStatusbar);
    SetStatusText(_("Welkom bij ") +  __PRG_NAME__);
    UpdateStatusbarInfo();

    Bind(wxEVT_MENU, &MyFrame::OnSystemInfo,this, ID_SYSTEM_INFO);
    Bind(wxEVT_MENU, &MyFrame::OnAbout,     this, ID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit ,     this, ID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnPrintPage, this, ID_MENU_PRINTPAGE);
    Bind(wxEVT_MENU, &MyFrame::OnPrintFile, this, ID_MENU_PRINTFILE);
    Bind(wxEVT_MENU, &MyFrame::OnMenuChoice,this, ID_MENU_SETUP_FIRST, ID_MENU_SETUP_LAST);
    Bind(wxEVT_MENU, &MyFrame::OnLogging            , this, ID_MENU_LOG);
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
        std::cout << _("\nhallo in de console!") << std::endl;

        // default logging goes to a logger window
        LogDebug(_("Debug active"));
    }

    //all initialized, now show us!
    ShowStartImage(this);
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
    about.Printf(   __PRG_NAME__ + _(", versie ") + __VERSION__ + _(", uit ") + __YEAR__ +
                    _("\nDit is het bridge rekenprogramma van Harrie/Tinus"
                      "\nop basis van WxWidgets"
                      "\n\nBouw datum: %s\n%s"
                      "\n\n(c) wxSystemInformationFrame, PB: https://github.com/PBfordev"
                    ),  wxString::FromUTF8(buildDate), cfg::GetCopyright()
                );

    MyMessageBox( about, _("Over ") + __PRG_NAME__ , wxOK | wxICON_INFORMATION);
}   // OnAbout()
 
#include "wxsysinfoframe.h"
void MyFrame::OnSystemInfo(wxCommandEvent& )
{
    wxSystemInformationFrame* frame = new wxSystemInformationFrame(this);
    frame->Show();
//    wxInfoMessageBox(0);
}   // OnSystemInfo()

void MyFrame::UpdateStatusbarInfo(wxCommandEvent& )
{
    wxString tmp = cfg::GetActiveMatchAndSession() + ", ";
    wxString prn = cfg::GetPrinterName();
    if (prn == cfg::GetFilePrinterName())
        tmp +=  cfg::GetFilePrinterName();
    else
        tmp += cfg::GetWinPrintPrefix();
    m_pStatusbar->SetInfo(tmp);
}   //  UpdateStatusbarInfo()

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
    positionsFile.AddLine(    "HotkeyPos      := \""  HOTKEY  "\" ;hotkey to request mouse-positions to be generated");
    positionsFile.AddLine(    "; menu-definitions");
    positionsFile.AddLine(    _("MenuNewMatch      := \"bn\"       ; Bestand: Nieuw"             ));
    positionsFile.AddLine(    _("MenuShutdown      := \"ba\"       ; Bestand: Afsluiten"         ));
    positionsFile.AddLine(    _("MenuPrinter       := \"bp\"       ; Bestand: Printer"           ));
    positionsFile.AddLine(    _("MenuPrintPage     := \"bg\"       ; Bestand: printpaGina"       ));
    positionsFile.AddLine(    _("MenuSetupMatch    := \"iw\"       ; Instellingen: Wedstrijd"    ));
    positionsFile.AddLine(    _("MenuSetupSchema   := \"is\"       ; Instellingen: Schema"       ));
    positionsFile.AddLine(    _("MenuNamesInit     := \"ii\"       ; Instellingen: Ingeven"      ));
    positionsFile.AddLine(    _("MenuNamesAssign   := \"it\"       ; Instellingen: Toekennen"    ));
    positionsFile.AddLine(    _("MenuScoreEntry    := \"ss\"       ; Scores: Scores"             ));
    positionsFile.AddLine(    _("MenuCorSession    := \"sz\"       ; Scores: Zittingcorrecties"  ));
    positionsFile.AddLine(    _("MenuCorEnd        := \"se\"       ; Scores: Eindcorrecties"     ));
    positionsFile.AddLine(    _("MenuResult        := \"su\"       ; Scores: Uitslag"            ));
    positionsFile.AddLine(    _("MenuDbType        := \"ewn\"      ; Extra: Wissel Nieuw"        ));
    positionsFile.AddLine(    _("MenuOldType       := \"ewo\"      ; Extra: Wissel Oud"          ));
    positionsFile.AddLine(    _("AllMenus          := [\"bn\",\"bp\",\"bg\",\"iw\",\"is\",\"ii\",\"it\",\"ss\",\"sz\",\"se\",\"su\",\"ewo\",\"ewn\"]"));
    positionsFile.AddLine(    ";\n;windowrelative-positions");
    positionsFile.AddLine(    ";s* vars are screen-coordinates, TL=TopLeft, TR=TopRight, BR=BottomRight\n;");
  
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
    MyLogShow(bShow);   // my own logger --> made this before I knew the wxLogWindow :(
    SetFocus();         // stay with me...
}   // OnLogging()

void MyFrame::OnLanguage(wxCommandEvent&)
{
    wxArrayString   names;
    wxArrayInt      identifiers;
    GetInstalledLanguages(names, identifiers);
    int index = wxGetSingleChoiceIndex(_("Kies een taal (en start opnieuw op)"), _("Taalselectie"), names);
    if (index < 0) return;  // cancel pressed


    int language = identifiers[index];
    if (language != cfg::GetLanguage())
    {   // setting same language would result in database close (if auto restart) and result in a lot of access-errors!
        if (m_pActivePage) m_pActivePage->BackupData();
        cfg::SetLanguage(language, names[index]);
        m_theApp.ReInitLanguage();  // can result in automatic restart!
        MyLogMessage(_("Taalkeuze: id=%d, naam=%s"), identifiers[index], names[index]); // will be shown in new logwindow, if restarted!
    }
}   // OnLanguage()

void MyFrame::OnPrintFile(wxCommandEvent& )
{
    AUTOTEST_BUSY("printFile");
    wxString filename = wxFileSelector(_("Kies een bestand om te printen"));

    if ( !filename.IsEmpty() )
    {
        LogMessage(_("Afdrukken van bestand <%s>"), filename);
        prn::PrintAFile(filename);
    }
}   // OnPrintFile()
