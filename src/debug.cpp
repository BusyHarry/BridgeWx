// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h> 
#include <wx/checkbox.h>
#include <wx/filedlg.h>
#include <wx/choicdlg.h>

#include "cfg.h"
#include "names.h"
#include "printer.h"
#include "debug.h"
#include "score.h"
#include "main.h"

/*
* according 'bard'/'gemini
Systeemkaart: Convention card
Gidsbriefje : Bidding card, Cue card, Movements card
Richting    : Direction, Side: We spelen in de Noord-Zuid richting." - "We are playing in the North-South direction."
Ronde       : Round, deal, hand Deal
Tegenstander: Opponent, Declarer's opponent, Adversary
Leider      : Declarer
meer:
    spel            : board (game?) deal
    Troefkleur      : Trump suit
    Bieden          : Bidding
    Spelverdeler    : Dealer
    Slagen          : Tricks
    Contract        : Contract
    Score           : Score
    stilzit tafel   : Sit-out table or Dummy table
    Spellen lenen   : Borrow the cards from a table, Take the cards from a table
    frequentiestaat : frequency table or "hand distribution"
    reken programma : scoring program
    stilzit         : a bye
    scoreslips      : result slips/scorecard, traveler(s)
    arbitrale score : referee/artificial score, adjusted score
    totaal-uitslag  : end score, total score, final score, competition result

Board: https://en.wikipedia.org/wiki/Glossary_of_contract_bridge_terms
 1- One particular allocation of 52 cards to the four players including the bidding, the play of the cards and the scoring based on those cards. Also called deal or hand.
 2- A device that keeps each player's cards separate for duplicate bridge.
 3- The dummy's hand. For example, "You're on the board" means "The lead is in the dummy".

* 
*/
static const wxString ssDbg = "dbg";
#define OUTPUT_TEXT_FORMATTED(fmt,...) OutputText(FMT(fmt, __VA_ARGS__))
#define OUTPUT_TEXT           OutputText

#define CONSOLE_HSIZER      /* defined if you want save/clear buttons*/
#if defined CONSOLE_HSIZER
    #define CONSOLE_VSIZER  /* defined if you want buttons and window in one sizer*/
#endif

#define PNT(x,y) {(int)(x), (int)(y)}   /* create a wxPoint from the supplied x and y, certainly if values are unsigned*/

class Console : public wxTextCtrl
{
    // a user would create a Console and register the 'Enter' event like:
    //  newConsole->Bind(wxEVT_TEXT_ENTER,[this](wxCommandEvent&){wxstring cmd=newConsole->GetCommand();/*do something*/});
    // REMARK: if there are modal windows in your cmd-handling, the prompt is set BEFORE any output of the user.
    //         So possibly you need to set the prompt/insertion position by calling AsyncTextOutBegin()
    //         BEFORE the showmodal and AsyncTextOutEnd() after.
    // If you write asynchrone text to the console, you need to call AsyncTextOutEnd() at the end,
    // else, the console still thinks the prompt is at its original position.

public:
    explicit    Console(wxWindow *parent, const wxString& label, const wxString& prompt = ES);
    virtual    ~Console(){}
    void        SetPrompt       (const wxString& prompt);
    const wxString& GetCommand  ();             // retrieve the last entered commandline
    wxBoxSizer* GetVSizer       ();
    wxBoxSizer* GetHSizer       ();
    void        AsyncTextOutBegin();            // disable outputting prompt
    void        AsyncTextOutEnd ();             // force prompt at end of text
    virtual void AppendText     (const wxString& text) final override;      // add text to console and update prompt-position
    virtual void Clear          (bool bPrompt = true);// final override;    // to update prompt-position
    void        ShowHSizer      (bool bShow = true) {m_pHSizer->Show(bShow);}
    void        HideHSizer      (){ShowHSizer(false);}
    wxWindow*   GetSaveButton   ();
    wxWindow*   GetClearButton  ();
private:
    void        OnChar(wxKeyEvent& event);      // consume keys in unwanted positions
    void        CreateCmdLine();                // prepaire the entered data for the user to retrieve
    void        ShowPrompt();                   // setup the prompt after changes

    wxString    m_prompt;                       // the prompt to show after each \n
    int         m_promptPosition;               // first position after prompt: changes are ONLY allowed AFTER this position
    wxString    m_command;                      // the data entered after the prompt, available at \n entry
    wxBoxSizer* m_pVSizer;                      // sizer for console and buttons
    wxBoxSizer* m_pHSizer;                      // sizer for the console buttons 
    bool        m_bAsyncPrompt;                 // (don't) show prompt
    bool        m_bFirstPrompt;                 // only show initial prompt if set (user could have done it already)
    wxButton*   m_pButtonSave;
    wxButton*   m_pButtonClear;
    public:
    bool bFrozen;
    int sErasecount;
    int sPaintcount;
    int freezeCount;
    int thawCount;
    wxTimer m_freezeTimer;

};

Console::Console(wxWindow* parent, const wxString& a_label, const wxString& a_prompt) 
    : wxTextCtrl(parent, wxID_ANY, a_label, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_PROCESS_ENTER)
{
    ShowNativeCaret(true);
    //SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_pVSizer       = nullptr;
    m_pHSizer       = nullptr;
    m_bAsyncPrompt  = false;
    m_bFirstPrompt  = true;
    Bind(wxEVT_CHAR, &Console::OnChar, this);   // we need this to only allow chars AFTER the prompt
    SetPrompt(a_prompt);                        // default prompt
    CallAfter([this]{ if(m_bFirstPrompt) ShowPrompt();});  // After userinit, we show the prompt if user hasn't done it yet
    m_pButtonSave = new wxButton(parent, wxID_ANY, _("Save"));
    m_pButtonSave->SetToolTip(_("save window-content"));
    m_pButtonSave->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
        {
            AUTOTEST_BUSY("save");
            wxString filename = wxFileSelector(_("Choose a file to save the text to")
                , wxEmptyString, "debug.txt");
            if (!filename.IsEmpty()) this->SaveFile(filename);
            SetInsertionPoint(GetLastPosition());
            CallAfter([this]{this->SetFocus();});
        } );

    m_pButtonClear = new wxButton(parent, wxID_ANY, _("Clear"));
    m_pButtonClear->SetToolTip(_("clear window-content"));
    m_pButtonClear->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { this->Clear();} );

    int size = GetFont().GetPointSize();
    wxFont  celFont ((size*4)/5, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    SetFont(celFont);

#ifdef  CONSOLE_HSIZER
    m_pHSizer = new wxBoxSizer(wxHORIZONTAL);
    m_pHSizer->Add(m_pButtonSave , 0, wxCENTER|wxALL, MY_BORDERSIZE);
    m_pHSizer->AddSpacer(10);
    m_pHSizer->Add(m_pButtonClear, 0, wxCENTER|wxALL, MY_BORDERSIZE);

    #ifdef CONSOLE_VSIZER
        m_pVSizer = new wxBoxSizer(wxVERTICAL);
        m_pVSizer->Add(this     , 1, wxEXPAND);
        m_pVSizer->Add(m_pHSizer, 0);
    #endif
#endif
        bFrozen = false;
        sErasecount=0;
        sPaintcount=0;
        freezeCount = 0;
        thawCount = 0;

#if 0   // test of preventing flicker
        m_freezeTimer.Bind(wxEVT_TIMER,[this](const wxTimerEvent&)
                    {
                        CallAfter([this]{
                                        Thaw();
                                        LogMessage("Thawtimer()");
                                        }
                            );
                    }
            );
#if 0
        Bind( wxEVT_ERASE_BACKGROUND, [this](wxEraseEvent& evt)
#else
        Bind(wxEVT_KILL_FOCUS       , [this](wxFocusEvent& evt)
#endif
            {
                if (!m_freezeTimer.IsRunning())
                {   Freeze();
                    m_freezeTimer.StartOnce(100);
                }
                evt.Skip();
            }); //anti flicker
#endif
#if 0   // test of preventing flicker
        CallAfter([this]{Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& evt)
        {
             evt.Skip();
             Freeze();
             HideNativeCaret();
             CallAfter([this]{Thaw();});
             return;
//             if (!bFrozen)
             {
                 Freeze();
                // bFrozen = true;
                 ++freezeCount;
                 LogMessage("killfocus");
//                 LogMessage("killfocus: freeze=%i, thaw=%i)", freezeCount, thawCount);
                 CallAfter([this]{ wxYield();wxMilliSleep(50);wxYield(); Thaw();});
             }
        }
        );}); 
        CallAfter([this]{ Bind(wxEVT_SET_FOCUS,[this]( wxFocusEvent&evt)
                                                {
                                                    Freeze();
                                                    ShowNativeCaret();
                                                    CallAfter([this]{Thaw();});
                                                    evt.Skip();
                                                    return;

                                                    //if (bFrozen)
                                                    {
                                                        Thaw();
                                                        bFrozen = false;
                                                        ++thawCount;
                                                        LogMessage("setfocus: freeze=%i, thaw=%i)", freezeCount, thawCount);
                                                        //if (freezeCount == thawCount) ShowNativeCaret();
                                                        evt.Skip();
                                                        CallAfter([this] {ShowPosition(GetInsertionPoint());});
                                                    }
                                                }
                            );
                         }
                 );

        Bind( wxEVT_ERASE_BACKGROUND,[this](wxEraseEvent& event){/*if (bFrozen) { Thaw(); bFrozen = false; LogMessage("erase/thaw"); }*/ LogMessage("erase %i", ++sErasecount); });
        Bind( wxEVT_PAINT,           [this](wxPaintEvent& event){ LogMessage( "paint %i", ++sPaintcount );});
#endif
}   // Console()

wxWindow* Console::GetSaveButton()  {return m_pButtonSave;}
wxWindow* Console::GetClearButton() {return m_pButtonClear;}

void Console::AppendText(const wxString& text)
{
    wxTextCtrl::AppendText(text);
    m_promptPosition = GetLastPosition();
}   // AppendText()

void Console::Clear(bool a_bPrompt)
{
    wxTextCtrl::Clear();
    if (a_bPrompt) ShowPrompt();
}   // Clear()

void Console::AsyncTextOutBegin()
{
    m_bAsyncPrompt = true;
}   // AsyncTextOutBegin()

void Console::AsyncTextOutEnd()
{
    m_bAsyncPrompt = false;
    ShowPrompt();
}   // AsyncTextOutEnd()

void Console::ShowPrompt()
{
    if (m_bAsyncPrompt) return;
    Freeze();
    long pos = GetLastPosition();
    if (pos && GetRange(pos - 1, pos)[0] != '\n')
    {
        wxTextCtrl::AppendText('\n');   // force begin of line
    }
    wxTextCtrl::AppendText(m_prompt);
    pos = GetLastPosition();
    m_promptPosition = pos;
    SetInsertionPoint(pos); // makes 'pos' visible
    ShowPosition(-1);
    this->SetFocus();
//    CallAfter([this]{this->SetFocus();});
    Thaw();
    m_bFirstPrompt = false;
}   // ShowPrompt()

wxBoxSizer* Console::GetVSizer(){return m_pVSizer;}
[[maybe_unused]] wxBoxSizer* Console::GetHSizer(){return m_pHSizer;}

const wxString& Console::GetCommand()
{
    return m_command;
}   // GetCommand()

void Console::SetPrompt(const wxString& a_prompt)
{
    m_prompt = a_prompt.IsEmpty() ? _("prompt>") : a_prompt;
}   // SetPrompt()

void Console::OnChar(wxKeyEvent& a_event)
{
    a_event.Skip();   // default: let system handle key
    wxChar  uniChar         = a_event.GetUnicodeKey();
    int     asciiChar       = a_event.GetKeyCode();
    int     insertionPoint  = GetInsertionPoint();
#if 0
    MyLogMessage(_("Console::OnChar(unikey=0x%04X, key=0x%04X, prompt at %u, insert at %u)"),
        uniChar, asciiChar, m_promptPosition, insertionPoint);
#endif
    if ( uniChar == WXK_NONE )
    {
        // It's a character without any Unicode equivalent at all, e.g. cursor
        // arrow or function key, we never filter those.
        if (asciiChar != WXK_BACK && asciiChar != WXK_DELETE && asciiChar != WXK_NUMPAD_DELETE)     // only set for asciiChar???????
            return;
    }

    if (asciiChar == WXK_BACK) --insertionPoint;
    if ( insertionPoint < m_promptPosition )
    {   
        if (asciiChar != WXK_CONTROL_C) // don't eat ^C: leave this key, so you can copy and paste
            a_event.Skip(false);        // eat key: don't allow changes in prompt area or area before it: readonly part
// enter should be handled!       return;
    }

    if ( asciiChar == WXK_RETURN || asciiChar == WXK_NUMPAD_ENTER )
    {
        SetInsertionPoint(GetLastPosition());   // simulate caret at end of input
        wxTextCtrl::AppendText('\n');           // users "OnEnter()" event will eat this char..
        CreateCmdLine();                        // create commandline now
        // AFTER user has handled the commandline, we will set the prompt if allowed
        if (!m_bAsyncPrompt) CallAfter([this]() { ShowPrompt(); });
    }
}   // OnChar()

void Console::CreateCmdLine()
{
    wxTextPos pos = GetLastPosition();
    m_command = GetRange(m_promptPosition, pos);    // get all chars from end of prompt to last char entered
}   // CreateCmdLine()

#define GROUP_CHOICE    "GroupChoice"
#define PAIR_CHOICE     "PairChoice"
#define FIRST_SET       "FirstSet"
#define NUMBER_OF_SETS  "NrOfSets"
#define PRINT_COUNT     "PrintCount"
#define SET_SIZE        "SetSize"

Debug::Debug(wxWindow* a_pParent, UINT a_pageId, DebugType a_type) :Baseframe(a_pParent, a_pageId)
{
    m_bPrintNext    = false;
    m_bDontTest     = false;
    m_group         = 1;
    m_debugType     = a_type;
    m_bSchema       = false;

    m_pConsole = new Console(this, Unique("Console"));
    m_pConsole->Bind(wxEVT_TEXT_ENTER,[this](wxCommandEvent&){HandleCommandLine(m_pConsole->GetCommand()); });

    m_pCheckBoxPrintNext = new wxCheckBox(this, wxID_ANY, _("print next output"));
    m_pCheckBoxPrintNext->SetToolTip(_("Output of next command will (also) be printed"));
    m_pCheckBoxPrintNext->Bind(wxEVT_CHECKBOX,[this](wxCommandEvent&){ m_pConsole->SetFocus();m_bPrintNext = m_pCheckBoxPrintNext->IsChecked();});

    //setup for guides
    m_pHsizerGuides = new wxBoxSizer(wxHORIZONTAL);
    m_pGroupChoice = new MY_CHOICE(this, _("Group:  "), _("Guides for this group"), Unique(GROUP_CHOICE));
    m_pGroupChoice->Bind(wxEVT_CHOICE, &Debug::OnSelectGroup    , this );
    m_pHsizerGuides->MyAdd(m_pGroupChoice, 0, wxALIGN_CENTER_VERTICAL);  m_pHsizerGuides->AddSpacer( 30 );
    m_pPairChoice = new MyChoiceMC(this,_("Pair: "), _("Guides for choosen pair/all pairs"), Unique(PAIR_CHOICE));
    m_pPairChoice->Bind(wxEVT_CHOICE, &Debug::OnSelectPair, this);
    m_pHsizerGuides->MyAdd(m_pPairChoice, 0, wxALIGN_CENTER_VERTICAL);  m_pHsizerGuides->AddSpacer( 60 );

    m_pChkBoxGuide = new wxCheckBox(this, wxID_ANY, _("Schema"));
    m_pChkBoxGuide->SetToolTip(_("show/print schema-overview"));
    m_pChkBoxGuide->Bind(wxEVT_CHECKBOX,[this](wxCommandEvent&){m_pConsole->SetFocus(); m_bSchema = m_pChkBoxGuide->IsChecked(); });
    m_pHsizerGuides->Add(m_pChkBoxGuide, 0, wxALIGN_CENTER_VERTICAL);

    //setup for scoreslips
    m_pHsizerScoreSlips = new wxBoxSizer(wxHORIZONTAL);
    m_pSetSize = new MY_CHOICE(this, _("Setsize:"), _("Number of games per table"), Unique(SET_SIZE));
    m_pSetSize->Bind(wxEVT_CHOICE, &Debug::SetFocusAfter, this);
    m_pHsizerScoreSlips->MyAdd(m_pSetSize, 0, wxALIGN_CENTER_VERTICAL);  m_pHsizerScoreSlips->AddSpacer( 20 );
    m_pFirstSet = new MyChoiceMC(this,_("First set:"), _("Starting with this set, scoreslips will be printed"),FIRST_SET);
    m_pFirstSet->Bind(wxEVT_CHOICE, &Debug::SetFocusAfter, this);
    m_pHsizerScoreSlips->MyAdd(m_pFirstSet, 0, wxALIGN_CENTER_VERTICAL);  m_pHsizerScoreSlips->AddSpacer( 20 );
    m_pNrOfSets = new MyChoiceMC(this,_("Number of sets:"), _("Number of sets that will be printed"), NUMBER_OF_SETS);
    m_pNrOfSets->Bind(wxEVT_CHOICE, &Debug::SetFocusAfter, this);
    m_pHsizerScoreSlips->MyAdd(m_pNrOfSets, 0, wxALIGN_CENTER_VERTICAL);  m_pHsizerScoreSlips->AddSpacer( 20 );
    m_pRepeatCount = new MyChoiceMC(this,_("Count:"), _("Number of times the selection should be printed"), PRINT_COUNT);
    m_pRepeatCount->Bind(wxEVT_CHOICE, &Debug::SetFocusAfter, this);
    m_pHsizerScoreSlips->MyAdd(m_pRepeatCount, 0, wxALIGN_CENTER_VERTICAL);  m_pHsizerScoreSlips->AddSpacer( 100 );

    m_pHsizerSlipsExtra = new wxBoxSizer(wxHORIZONTAL);
    m_pTxtCtrlExtra = new wxTextCtrl(this, wxID_ANY, Unique("Extra_text"));
    m_pTxtCtrlExtra->Clear();   // remove 'label'-text
    m_pTxtCtrlExtra->SetMaxLength(cfg::MAX_NAME_SIZE);
    m_pTxtCtrlExtra->SetToolTip(_("Any additional text such as 'green group'"));
    m_pTxtCtrlExtra->SetMinSize({(int)cfg::MAX_NAME_SIZE*m_pTxtCtrlExtra->GetCharWidth(),-1});
    m_pHsizerSlipsExtra->Add(new wxStaticText(this, wxID_ANY,_("Additional text:")),0,wxRIGHT|wxALIGN_CENTER_VERTICAL,MY_BORDERSIZE);
    m_pHsizerSlipsExtra->Add(m_pTxtCtrlExtra , 0, wxRIGHT | wxALIGN_CENTER_VERTICAL,MY_BORDERSIZE);

    auto pButtonExample = new wxButton(this, wxID_ANY, Unique(_("Example")));
    pButtonExample->SetToolTip(_("show choosen selection"));
    pButtonExample->Bind(wxEVT_BUTTON,&Debug::OnExample, this);
    m_pHsizerSlipsExtra->Add(pButtonExample);// , 0, wxRIGHT | wxALIGN_CENTER_VERTICAL,MY_BORDERSIZE);

    auto pButtonPrint = new wxButton(this, wxID_ANY, Unique(_("Print")));
    pButtonPrint->SetToolTip(_("print choosen selection"));
    pButtonPrint->Bind(wxEVT_BUTTON,&Debug::OnPrint, this);

    m_pHsizerSlipsExtra->AddStretchSpacer(100);
    m_pHsizerSlipsExtra->Add(pButtonPrint, 1, wxALIGN_CENTER_VERTICAL,MY_BORDERSIZE);

    wxString windowText;
    switch (m_debugType)
    {   // show only needed controls
        default:
        case DebugConsole:
            m_pConsole          ->ShowHSizer();
            m_pHsizerGuides     ->Show(false);
            m_pHsizerScoreSlips ->Show(false);
            m_pCheckBoxPrintNext->Show(true);
            m_pHsizerSlipsExtra ->Show(false);
            m_pStatusBarInfo = _("Guides via cmd: '[@]px', where 'x' is the (first) pair. '@' : all pairs starting with 'x'");
            windowText       = _("Debug console");
            m_description    = "DebugConsole";
            AUTOTEST_ADD_WINDOW(m_pCheckBoxPrintNext        , "PrintNext" );
            AUTOTEST_ADD_WINDOW(m_pConsole->GetSaveButton() , "Save"      );
            AUTOTEST_ADD_WINDOW(m_pConsole->GetClearButton(), "Clear"     );
            break;
        case Guides:
            m_pConsole          ->HideHSizer();
            m_pHsizerGuides     ->Show(true);
            m_pHsizerScoreSlips ->Show(false);
            m_pCheckBoxPrintNext->Show(false);
            m_pHsizerSlipsExtra ->Show(true);
            m_pStatusBarInfo = _("Creation/print of guides");
            windowText       = _("Guides");
            m_description    = "DebugGuides";
            {
                auto pButtonExportSchema = new wxButton(this, wxID_ANY, Unique(_("Export schema")));
                pButtonExportSchema->SetToolTip(_("export current schema to .asc file"));
                pButtonExportSchema->Bind(wxEVT_BUTTON, &Debug::OnExportSchema, this);
                m_pHsizerGuides->AddSpacer(80);
                m_pHsizerGuides->Add(pButtonExportSchema);
                AUTOTEST_ADD_WINDOW(pButtonExportSchema , "ExportSchema");
            }
            AUTOTEST_ADD_WINDOW(m_pPairChoice , PAIR_CHOICE );
            AUTOTEST_ADD_WINDOW(m_pGroupChoice, GROUP_CHOICE);
            break;
        case ScoreSlips:
            m_pTxtCtrlExtra->SetMaxLength(24);
            m_pConsole          ->HideHSizer();
            m_pHsizerGuides     ->Show(false);
            m_pHsizerScoreSlips ->Show(true);
            m_pCheckBoxPrintNext->Show(false);
            m_pHsizerSlipsExtra ->Show(true);
            m_pStatusBarInfo = _("Creation/print of scoreslips");
            windowText       = _("Scoreslips");
            m_description    = "DebugSlips";
            AUTOTEST_ADD_WINDOW(m_pFirstSet     , FIRST_SET     );
            AUTOTEST_ADD_WINDOW(m_pNrOfSets     , NUMBER_OF_SETS);
            AUTOTEST_ADD_WINDOW(m_pRepeatCount  , PRINT_COUNT   );
            AUTOTEST_ADD_WINDOW(m_pSetSize      , SET_SIZE      );
            break;
    }

    AUTOTEST_ADD_WINDOW(m_pConsole, "TextWindow");
    // add to layout
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, windowText);
    vBox->Add(m_pConsole->GetVSizer(), 1 ,wxCENTER|wxALL| wxEXPAND, MY_BORDERSIZE);
    vBox->Add(m_pHsizerGuides        , 0, wxLEFT|wxALL            , MY_BORDERSIZE);
    vBox->Add(m_pHsizerScoreSlips    , 0, wxLEFT|wxALL            , MY_BORDERSIZE);
    vBox->Add(m_pCheckBoxPrintNext   , 0, wxLEFT|wxALL            , MY_BORDERSIZE);
    vBox->Add(m_pHsizerSlipsExtra    , 0, wxLEFT|wxALL            , MY_BORDERSIZE);

    SetSizer(vBox);     // add to panel
    SetDoubleBuffered(true);    // prevents flickering of console window when cursor is showed/hidden
    RefreshInfo();
    //LogMessage("Debug:: \'" + windowText + "\' aangemaakt");
}   // Debug()

Debug::~Debug(){}

void Debug::SetFocusAfter(wxCommandEvent&)
{
    AUTOTEST_BUSY("focusAfter");
    CallAfter([this]{m_pConsole->SetFocus();});
}   // SetFocusAfter()

void Debug::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    switch(m_debugType)
    {
        case DebugConsole:
            AutoTestAddWindowsNames(a_pFile, m_description);
            break;
        case Guides:
            AutoTestAddWindowsNames(a_pFile, m_description);
            break;
        case ScoreSlips:
            AutoTestAddWindowsNames(a_pFile, m_description);
            break;
    }
}   // AutotestRequestMousePositions()

void Debug::OnSelectGroup(wxCommandEvent&)
{
    LogMessage("Debug::OnSelectGroup()");
    m_group = m_pGroupChoice->GetSelection() + 1;
    m_pConsole->Clear(false);
    InitGroupData();
    m_pConsole->AsyncTextOutEnd();
}   // OnSelectGroup()

void Debug::OnSelectPair(wxCommandEvent&)
{
    CallAfter([this]{m_pConsole->SetFocus();}); // doesn't work without CallAfter()...
    // do nothing: action on button example/print, only set focus
}   // OnSelectPair()

void Debug::PrintOrExample()
{
    bool bPrintToFile = m_bPrintNext && prn::IsPrintToFile();
    if (bPrintToFile)
        m_bPrintNext = false;   // printer can't print linechars to a text-file, so we do it here

    if (m_debugType == ScoreSlips)
    {
        //LogMessage("Debug::OnShowSlips()");
        UINT setSize        = m_pSetSize->GetSelection() + 1;
        UINT firstSet       = m_pFirstSet->GetSelection() + 1;
        UINT nrOfSets       = m_pNrOfSets->GetSelection() + 1;
        UINT repeatCount    = m_pRepeatCount->GetSelection() + 1;
        wxString extra      = m_pTxtCtrlExtra->GetValue();

        PrintScoreSlips(setSize, firstSet, nrOfSets, repeatCount, extra);
    }
    else
    {
        if (!m_schema.IsOk())
        {
            m_consoleOutput.Clear();
            m_consoleOutput.push_back(_("No schema available for these data"));
            m_bPrintNext = false;       // force console output
        }
        else
        {
            if (m_bSchema)
            {
                //LogMessage("Debug::OnShowSchema()");
                PrintSchemaOverviewNew();
            }
            else
            {
                //LogMessage("Debug::OnShowPair()");
                PrintGuideNew(m_pPairChoice->GetSelection());
            }
        }
    }

    if (bPrintToFile)
    {
        for (const auto& it : m_consoleOutput)
        {
            prn::PrintLine(it);
            prn::PrintCharacter('\n');
        }
        prn::EndPrint();    // force flush
    }
    else if (!m_bPrintNext)
    {
        m_pConsole->Freeze();           // stop update of display to prevent flicker (well, get less flicker!)
        m_pConsole->freezeCount++;
        m_pConsole->Clear(false);
        for (const auto& it : m_consoleOutput) OUTPUT_TEXT(it + '\n');  // insert generated output
        m_pConsole->Thaw();             // now you may update the console-window!
        m_pConsole->thawCount++;
        m_pConsole->AsyncTextOutEnd();  //    and display prompt at end of output: MUST be AFTER Thaw(), else begin of window is showed
        //LogMessage("example: freeze=%i, thaw=%i)", m_pConsole->freezeCount, m_pConsole->thawCount);
    }
    m_bPrintNext = false;
}   // PrintOrExample()

void Debug::OnPrint(wxCommandEvent&)
{
    m_bPrintNext = true;
    PrintOrExample();
}   // end OnPrint()

void Debug::OnExample(wxCommandEvent&)
{
    m_bPrintNext = false;       // prevent output to printer
    PrintOrExample();
}   // OnExample()

void Debug::OutputText(const wxString& msg)
{
    m_pConsole->AppendText(msg);
    if (m_bPrintNext)
    {
        prn::PrintLine(msg);
        ++m_linesPrinted;
    }
}   // OutputText()

void Debug::PrintPage()
{
    wxString title = _("debug window of ") + cfg::GetDescription();
    int nrOfLines = m_pConsole->GetNumberOfLines();

    prn::BeginPrint(title);
    for (int line = 0; line < nrOfLines; ++line)
    {
        prn::PrintLine(m_pConsole->GetLineText(line) + '\n');
    }
    prn::EndPrint();
}   // PrintPage()

class MyPrint
{
public:
    explicit MyPrint(wxArrayString* outArrayString);
    ~MyPrint();
    void BeginPrint         (const wxString& title);
    void EndPrint           ();
    void EndPage            ();
    void PrintTable         ( const prn::table::TableInfo& tableInfo);
private:
    void    ScreenToBuffer  ();
    void    PrintText       (const wxPoint& position, const wxString& string);
    void    PrintTextLine   (wxPoint& begin, wxPoint& end, int direction);
    void    ClearScreen     ();
    wxChar  SmoothLineChar  (int x, int y) const;
    #define MY_LINES 100
    #define MY_LINESIZE 100
    typedef wxChar MY_SCREEN [MY_LINES][MY_LINESIZE];
    wxChar          (*m_screen)[MY_LINESIZE];
    bool            m_bUseRealPrinter;
    int             m_maxLine;
    size_t          m_screenLength;
    wxArrayString*  m_pOutArrayString;
#if defined _UNICODE
    #define MY_MEMCPY wmemcpy
    #define MY_MEMSET wmemset
#else
    #define MY_MEMCPY memcpy
    #define MY_MEMSET memset
#endif
    MyPrint(const MyPrint&);
    const MyPrint& operator = (const MyPrint&x) const;//{return x;}
};

MyPrint::~MyPrint(){delete[] m_screen;}

MyPrint::MyPrint(wxArrayString* a_pOutArrayString)
{
    m_pOutArrayString   = a_pOutArrayString;
    m_bUseRealPrinter   = m_pOutArrayString == nullptr;
    m_maxLine           = -1;
    m_screen            = 0;
    m_screenLength      = MY_LINES*MY_LINESIZE;

    if (!m_bUseRealPrinter)
    {
        m_pOutArrayString->clear();
        m_screen = new wxChar[MY_LINES][MY_LINESIZE];
        ClearScreen();
    }
}   // MyPrint()

void MyPrint::ClearScreen()
{ 
    MY_MEMSET(&m_screen[0][0], ' ', m_screenLength);
    m_maxLine = -1;
}   // ClearScreen()

void MyPrint::BeginPrint(const wxString& a_title)
{
    if (m_bUseRealPrinter)
    {
        prn::BeginPrint(a_title);
    }
    else
    {
        m_pOutArrayString->clear();
        m_pOutArrayString->push_back('\n' + a_title);
    }
}   // BeginPrint()

void MyPrint::EndPrint()
{
    m_bUseRealPrinter ? prn::EndPrint() : ScreenToBuffer();
}   // EndPrint()

void MyPrint::EndPage()
{
    if  (m_bUseRealPrinter)
        prn::PrintCharacter('\f'); /* force new page*/
    else
        ScreenToBuffer();
}   // EndPage()

void MyPrint::PrintText( const wxPoint& position, const wxString& text)
{
    if (text.IsEmpty()) return;
    if (position.y < MY_LINES)
    {
        if (position.y < 0 || position.y >= MY_LINES) return;
        MY_MEMCPY(&m_screen[position.y][position.x], text.c_str(), text.Len());
        m_maxLine = std::max(m_maxLine,position.y); //assumption: no vertical line goes beyond this value!
    }
}   // PrintText()

#define MY_HLINE_CHAR L'═'  /*char to draw horizontal lines*/
#define MY_VLINE_CHAR L'║'  /*char to draw vertical   lines*/
void MyPrint::PrintTextLine( wxPoint& begin, wxPoint& end, int direction)
{
    // ONLY horizontal or vertical lines.
    // lines are drawn from left to right, top to bottom
    if (begin.x > end.x) std::swap(begin.x, end.x); // x: begin <= end
    if (begin.y > end.y) std::swap(begin.y, end.y); // y: begin <= end

    if (begin.x < 0) begin.x = 0;   if (begin.x >= MY_LINESIZE) return;
    if (begin.y < 0) begin.y = 0;   if (begin.y >= MY_LINES   ) return;
    if (end.x   < 0) end.x   = 0;   if (end.x   >= MY_LINESIZE) end.x = MY_LINESIZE-1;
    if (end.y   < 0) end.y   = 0;   if (end.y   >= MY_LINES)    end.y = MY_LINES-1;
    m_maxLine = std::max(m_maxLine, end.y);
    bool bOnlyHorizontal = direction == wxHORIZONTAL;
    if  (begin.y == end.y)
    {   // horizontal
        if (bOnlyHorizontal)
        {
            PrintText(begin, wxString(MY_HLINE_CHAR,(size_t)1 + end.x - begin.x));
        }
    }
    else
    {   // vertical
        if (!bOnlyHorizontal)
        {
            for (int y = begin.y; y <= end.y; ++y)
            {   // first: just draw the line
                    m_screen[y][begin.x] = MY_VLINE_CHAR;
            }
            for (int y = begin.y; y <= end.y; ++y)
            {   // now check what char I need here for a nice table
                m_screen[y][begin.x] = SmoothLineChar(begin.x, y);
            }
        }
    }
}   // PrintTextLine()

static wxChar translate[16]=
{   // translate the vertical linecharacter '║' to a 'smoother' one
/*00*/    MY_VLINE_CHAR,  //hLeft   = 1,    // left   has a horizontal char
/*01*/    MY_VLINE_CHAR,  //hRight  = 2,    // right  has a horizontal char
/*02*/    MY_VLINE_CHAR,  //vTop    = 4,    // top    has a vertical   char
/*03*/    MY_VLINE_CHAR,  //vBottom = 8     // bottom has a vertical   char
/*04*/    MY_VLINE_CHAR,
/*05*/    L'╝',
/*06*/    L'╚',
/*07*/    L'╩',
/*08*/    MY_VLINE_CHAR,
/*09*/    L'╗',
/*10*/    L'╔',
/*11*/    L'╦',
/*12*/    MY_VLINE_CHAR,
/*13*/    L'╣',
/*14*/    L'╠',
/*15*/    L'╬'
};

wxChar MyPrint::SmoothLineChar(int x, int y) const
{
    // (x,y) is point of a vertical line, y values are incremental.
    // Check what type of char we really need to get a smooth lining
    enum LineType
    {
        hLeft   = 1,    // left   has a horizontal line
        hRight  = 2,    // right  has a horizontal line
        vTop    = 4,    // top    has a vertical   line
        vBottem = 8     // bottom has a vertical line
    };
    UINT state = 0;
    // for now, assume that all 0x2500 <= char <= 0x2580 are line-chars
    wxChar testA=0; MY_UNUSED(testA);
    wxChar testR=0; MY_UNUSED(testR);
    wxChar testB=0; MY_UNUSED(testB);
    wxChar testL=0; MY_UNUSED(testL);

    if (y > 0)
    {   // check above
        testA = m_screen[y-1][x];
        if ( (testA == MY_VLINE_CHAR) || ((testA >= 0x2500)  && (testA <= 0x2580)) )
            state |= vTop;
    }
    if ( x < MY_LINESIZE-1)
    {   // check right
        testR = m_screen[y][x+1];
        if (testR == MY_HLINE_CHAR) state |= hRight;
    }
    if ( y < MY_LINES-1)
    {   // check below
        testB = m_screen[y+1][x];
        if (testB == MY_VLINE_CHAR) state |= vBottem;
    }
    if ( x > 0)
    {   // check left
        testL = m_screen[y][x-1];
        if (testL == MY_HLINE_CHAR) state |= hLeft;
    }

    return translate[state];
}   // SmoothLineChar()

void MyPrint::PrintTable(const prn::table::TableInfo& a_tableInfo)
{
    if (m_bUseRealPrinter)
    {
        prn::PrintTable(a_tableInfo);  
    }
    else
    {   // interprete tableInfo and put in screenbuffer
        for ( size_t index = 0; index < a_tableInfo.textCount; ++index)
        {   // print all the texts in the textarray
            wxPoint  pos  = a_tableInfo.texts[index].begin + a_tableInfo.origin;
            PrintText(pos, a_tableInfo.texts[index].text);
        }
        for (const auto& textIt : a_tableInfo.textsV)
        {   // print all the texts in the textvector
            wxPoint  pos  = textIt.begin + a_tableInfo.origin;
            PrintText(pos, textIt.text);
        }
        int type = wxHORIZONTAL;
        for ( int run = 1; run <= 2; ++run)
        {   // first run only horizontal, second run only vertical lines
            for (size_t index = 0; index < a_tableInfo.lineCount; ++index)
            {   // draw all the lines in the linearray
                wxPoint pos     = a_tableInfo.origin + a_tableInfo.lines[index].begin;
                wxPoint endPos  = a_tableInfo.origin + a_tableInfo.lines[index].end;
                PrintTextLine(pos, endPos, type);
            }
            for (const auto& lineIt : a_tableInfo.linesV)
            {   // draw all the lines in the linevector
                wxPoint pos     = a_tableInfo.origin + lineIt.begin;
                wxPoint endPos  = a_tableInfo.origin + lineIt.end;
                PrintTextLine(pos, endPos, type);
            }
            type = wxVERTICAL;
        }
    }
}   // PrintTable()

void MyPrint::ScreenToBuffer()
{
    if (m_maxLine < 0) return;
    for (int line = 0; line <= m_maxLine; ++line)
    {
        wxString tmp(m_screen[line], MY_LINESIZE);
        tmp.Trim(TRIM_RIGHT);
        m_pOutArrayString->push_back(tmp);
    }
    m_pOutArrayString->push_back('\f');
    ClearScreen();
}   // ScreenToBuffer()

static void SkipWhite(const wxChar* &pBuf)        // skip white chars
{
    while (std::isspace (*pBuf)) pBuf++;
}   // SkipWhite()

static void SkipDigits(const wxChar* &pBuf)       // skip digits
{
    SkipWhite(pBuf);
    while (std::isdigit(*pBuf)) pBuf++;
    SkipWhite(pBuf);
}   // SkipDigits()

UINT Debug::AtoiRound(const wxChar * pBuf)
{
    UINT round = wxAtoi(pBuf);   // 'wide' atoi

    if ((round < 1) || (round > m_rounds))
    {
        round = 1;
        OUTPUT_TEXT(_("     ----> round error <---, default round 1\n"));
    }
    return round;
}   // atoironde()

UINT Debug::AtoiPair(const wxChar * pBuf)
{
    UINT pair = wxAtoi(pBuf);
    if (pair == 0 )
        pair = 1;   // empty string, use pair 1
    if ((pair < 1) || (pair > m_pairs))
    {
        pair = 1;
        OUTPUT_TEXT(_("     ----> pair error <---, default pair 1\n"));
    }
    return pair;
}   // AtoiPair()

[[maybe_unused]] UINT Debug::AtoiGroup(const wxChar * pBuf)
{
    UINT group = wxAtoi(pBuf);
    if ((group < 1) || (group >= m_groupData.size()))
    {
        group = 1;
        OUTPUT_TEXT(_("     ----> group error <---, default group 1\n"));
    }
    return group;
}   // AtoiGroup()

UINT Debug::AtoiTable(const wxChar * pBuf)
{
    UINT table = wxAtoi(pBuf);
    if ((table < 1) || (table > m_tables))
    {
        table = 1;
        OUTPUT_TEXT(_("     ----> table error <---\n"));
    }
    return table;
}   // atoitable()

UINT Debug::AtoiGame2Set(const wxChar* pBuf)
{
    UINT game = wxAtoi(pBuf) - cfg::GetFirstGame()+1;
    if ((game < 1) || (game > m_setSize*m_rounds))
    {
        game = 1;
        OUTPUT_TEXT(_("     ----> game error <---, default game 1\n"));
    }
    return (game-1)/m_setSize+1;
}   // AtoiGame2Set()

static wxString prompt=ssDbg;

void Debug::SetPrompt()
{
    prompt = _("group ");
    prompt += m_groupData[m_group-1].groupChars.IsEmpty() ? U2String(m_group) : m_groupData[m_group-1].groupChars;
    prompt += '>';

    Layout();
    m_pConsole->SetPrompt(prompt);
}   // SetPrompt()

void Debug::RefreshInfo()
{
    names::InitializePairNames();
    OUTPUT_TEXT_FORMATTED(_("\ntest-mode, group %u (%u)\n"), m_group, cfg::MAX_GROUPS);
    auto pTmp = cfg::GetGroupData();
    if ( pTmp->size() >= m_groupData.size())
        m_groupData = *pTmp;                            // get active groupdata
    else
    {   // we have locally extra groups from previous 'debug', so only update possibly changed groupdata
        for (size_t grp = 0; grp < pTmp->size(); ++grp )
        {
            m_groupData[grp] = (*pTmp)[grp];
        }
    }

    InitGroupData();
    m_pConsole->SetFocus();
    m_pConsole->AsyncTextOutEnd();
    InitSlips();
    SendEvent2Mainframe(this, ID_STATUSBAR_SETTEXT, &m_pStatusBarInfo);
    Layout();
}   // RefreshInfo()

void Debug::InitSlips()
{
    if (m_debugType != ScoreSlips) return;
    m_pSetSize      ->Init(6,3,0);      // setsizes from 1-6, default: 4 games/set
    m_pFirstSet     ->Init(24,0,0);     // first set to print from 1-24, default: 1
    m_pNrOfSets     ->Init(24,5,0);     // nr of sets to print from 1-24, default 6
    m_pRepeatCount  ->Init(10,0,0);     // how many times to print all from 1-10, default 1
    m_pConsole->Clear();
}   // InitSlips()

void Debug::InitGuideStuff()
{
    if (m_debugType != Guides) return;
    auto& groupData = *cfg::GetGroupData();
    wxArrayString choices;
    for (const auto& group : groupData)
    {
        choices.push_back(group.groupChars);
    }
    while (choices.size() < m_group)
        choices.push_back(FMT("%u", (UINT)choices.size()+1));
    if (choices.size() == 1 && choices[0].IsEmpty()) choices[0] = "1";  // 'just' show group "1" for the one and only group!
    m_pGroupChoice->Init(choices, m_group-1);

    m_pPairChoice->Init(m_pairs, 0);
    m_pPairChoice->InsertItem(0, _("all"));
    m_pPairChoice->SetSelection(1);     // set selection to second item: first one will be 'all'
    m_pConsole->Clear(false);
}   // InitGuideStuff()

void Debug::HandleCommandLine(wxString cmd)
{
    m_linesPrinted = 0;
    if (cmd.IsEmpty()) return;
    if (cmd.EndsWith ('\n')) cmd.RemoveLast();
    cmd.Trim(TRIM_LEFT);
    cmd.Trim(TRIM_RIGHT);
    if (!cmd.IsEmpty())
    {
        cmd.MakeUpper();
        if (cmd.Len() > 1 && score::GetCardName(score::CiPass).MakeUpper().StartsWith(cmd))
            OUTPUT_TEXT_FORMATTED("%s, %s 0", score::GetCardName(score::CiPass), _("score"));
        else
            DoCommand(cmd);
        m_pCheckBoxPrintNext->SetValue(false);
        m_bPrintNext = false;
    }
}   // HandleCommandLine()

/******************************* C-Stuf **********************************************/
void Debug::PrintSeparator()
{
    #define halfSeparator "  .   .   .   .   .   .   .   .   .   ."
    if (m_bPrintNext )
    {
        OUTPUT_TEXT(halfSeparator halfSeparator);
    }
    OUTPUT_TEXT("\n\n");
}   // PrintSeparator()

void Debug::StringDoubler(const wxString& msg, UINT pair)
{
    wxString all(msg);

    if (m_bAllPairs && (pair+1 <= m_pairs))
        all += msg;
    all += "\n";
    OUTPUT_TEXT(all);
}   // StringDoubler()

wxString Debug::GetNsEwString(UINT pair, UINT round)
{
    return m_schema.IsNs( pair, round) ? _("NS") : _("EW");
}   // GetNsEwString()

wxString Debug::SetToGamesAsString(UINT set, bool bWide)
{
    if (set == 0) return bWide ? "       " : "     ";   // wide: "17 - 20", non-wide: "17-20"

    UINT offset = cfg::GetFirstGame()-1;
    UINT setSize = cfg::GetSetSize();
    return FMT(bWide ? "%2u - %-2u" : "%2u-%-2u", offset + (set - 1) * setSize + 1, offset + set * setSize);
}   // SetToGamesAsString()

UINT Debug::GetOpponent(UINT pair, UINT round)
{
    UINT opponent;
    UINT table = m_schema.GetTable( pair, round);
    for (opponent=1; opponent <= m_pairs; ++opponent)
    {
        if (m_schema.GetTable( opponent, round) == table)
        {
            if (opponent != pair)
                break;
        }
    }
    return opponent > m_pairs ? 0 : opponent;
}   // GetOpponent()

wxString Debug::GetBorrowTableAsString(UINT a_table, UINT a_round)
{
    UINT table = m_schema.GetBorrowTable( a_table, a_round);
    if (table == 0) return "  ";    // no borrowing
    //xgettext:TRANSLATORS: 'B' -> table to 'B'orrow boards from (same boards on different tables)
    return FMT(_("B%u"), table);
}   // GetBorrowTableAsString()

/* example guide-card:
.   .   .   .   .   .   .   .   .   .  .   .   .   .   .   .   .   .   .   .

ds Janssen - Pietersen                 hh Klaassen - Jacobs                 
extra info................extra     .  extra info................extra     .
╔════════════════════════════════╗  .  ╔════════════════════════════════╗  .
║ Paar 13               14 paren ║  .  ║ Paar 14               14 paren ║  .
║ schema:               4stayr14 ║  .  ║ schema:               4stayr14 ║  .
╠═════╦═══════╦═════╦════════════╣  .  ╠═════╦═══════╦═════╦════════════╣  .
║ronde║ tafel ║tegen║  spellen   ║  .  ║ronde║ tafel ║tegen║  spellen   ║  .
╠═════╬═══════╬═════╬════════════╣  .  ╠═════╬═══════╬═════╬════════════╣  .
║  1  ║  7 NZ ║   6 ║  9 - 12  L3║  .  ║  1  ║  3 OW ║   5 ║  9 - 12    ║  .
║  2  ║  2 OW ║   8 ║  5 - 8     ║  .  ║  2  ║  5 NZ ║   1 ║  5 - 8   L2║  .
║  3  ║  7 NZ ║   2 ║ 13 - 16  L4║  .  ║  3  ║  4 OW ║   3 ║ 13 - 16    ║  .
║  4  ║  5 NZ ║   4 ║  1 - 4   L1║  .  ║  4  ║  1 OW ║   7 ║  1 - 4     ║  .
╚═════╩═══════╩═════╩════════════╝  .  ╚═════╩═══════╩═════╩════════════╝  .
.   .   .   .   .   .   .   .   .   .  .   .   .   .   .   .   .   .   .   .
*/
void Debug::GuideCard( UINT pair, bool a_bAskExtra )
{   // 'old' method, don't translate!
#if 0
    OUTPUT_TEXT_FORMATTED("Debug::GuideCard(%u, alle paren=%s, print=%s)\n"
        , pair, m_bAllPairs ? "true" : "false", bPrint ? "true" : "false");
#endif
    #define NEED_SECOND m_bAllPairs && (pair+1 <= m_pairs)
    wxString explanation;
    wxString tmp;
    if (m_bPrintNext)
    {   // ask for special text on guide
        m_pConsole->AsyncTextOutBegin();    // delay prompt till ready (printing)
        #define SCHEMA_WIDTH 34
        m_explanation = a_bAskExtra ? wxGetTextFromUser(_("any text above each schema:"),ES,m_explanation).Left(SCHEMA_WIDTH)
                                    : m_pTxtCtrlExtra->GetValue();
        explanation   = FMT("  %-*s  .",SCHEMA_WIDTH, m_explanation);
        prn::BeginPrint();
        m_linesPrinted = 0; // winprint always starts with a new page...
    }
    do
    {
        if (m_bPrintNext)
        {
            if (m_linesPrinted + 9 + m_rounds > (UINT)prn::GetLinesPerPage())
            {
                PrintSeparator();
                OUTPUT_TEXT("\f");          // next guide doesn't fit on current page
                m_linesPrinted = 0;         // goto next page
            }

            PrintSeparator();               // cutting line
                                            // names of players
            tmp = FMT("  %-37s", names::PairnrSession2GlobalText(pair+m_pActiveGroupInfo->groupOffset));
            if (NEED_SECOND)                // 2 guides next to each other
                tmp +=FMT("  %-37s",names::PairnrSession2GlobalText(pair+1+m_pActiveGroupInfo->groupOffset));
            OUTPUT_TEXT(tmp);
        }
        OUTPUT_TEXT("\n");
        StringDoubler(explanation, pair);
        StringDoubler(                          L"  ╔════════════════════════════════╗  ."      , pair);
        tmp = FMT(                 L"  ║ Paar %-2u               %2u paren ║  ."   , pair, m_pairs);
        if (NEED_SECOND)
            tmp += FMT(            L"  ║ Paar %-2u               %2u paren ║  ."   , pair+1, m_pairs);
        tmp += "\n";
        OUTPUT_TEXT(tmp);
        tmp = FMT(                 L"  ║ schema: %22s ║  ."                        , m_pActiveGroupInfo->schema);
        StringDoubler(tmp, pair);
        StringDoubler(                          L"  ╠═════╦═══════╦═════╦════════════╣  ."      , pair);
        StringDoubler(                          L"  ║ronde║ tafel ║tegen║  spellen   ║  ."      , pair);
        StringDoubler(                          L"  ╠═════╬═══════╬═════╬════════════╣  ."      , pair);

        for (UINT round=1;round <= m_rounds;++round)
        {
            UINT table  = m_schema.GetTable(pair, round);
            UINT set    = m_schema.GetSet  (table,round);   // set/table == 0 means: not playing this round
            #define RX                          L"  ║ %2u  ║ %2u %s ║  %2u ║ %s  %2s║  ."
            #define NP                          L"  ║ %2u  ║       ║     ║            ║  ."
            tmp = FMT( set == 0 ? NP : RX,
                            round, table,
                            GetNsEwString(pair,round),
                            m_schema.GetOpponent(pair,round),
                            SetToGamesAsString(set),
                            GetBorrowTableAsString(table, round)
                        );
            if (NEED_SECOND)
            {
                table = m_schema.GetTable(pair+1, round);
                set   = m_schema.GetSet  (table , round);
                tmp += FMT(set == 0 ? NP : RX,
                                round, table,
                                GetNsEwString(pair+1,round),
                                m_schema.GetOpponent(pair+1,round),
                                SetToGamesAsString(set),
                                GetBorrowTableAsString(table, round)
                            );
            }
            tmp += '\n';
            OUTPUT_TEXT(tmp);
        }

        StringDoubler(L"  ╚═════╩═══════╩═════╩════════════╝  .", pair);
        pair +=2; 
    } while ( m_bAllPairs && (pair <= m_pairs) );
    PrintSeparator();
    if (m_bPrintNext)
    {
        prn::EndPrint();
        m_pConsole->AsyncTextOutEnd();      // prompt wanted
    }
}   // GuideCard()

void Debug::List(const wxChar* pBuf)
{
    if (strchr("@PRST",*pBuf) == nullptr)
    {
        Usage();    // unknown command
        return;
    }
    enum ListTypes
    {
         SET    = 1     // game involved
        ,ROUND  = 2     // round involved
        ,PAIR   = 4     // pair involved
        ,TABLE  = 8     // table involved
    };

    UINT pair=0,set=0,table=0,round=0,mode=0,game=0;
    bool bFound;
    while(*pBuf)
    {
        switch (*pBuf++)
        {
            case 'R':
                mode    |= ROUND;
                round   = AtoiRound(pBuf);
                break;
            case 'P':
                mode    |= PAIR;
                pair    = AtoiPair(pBuf);
                break;
            case 'S':
                mode    |= SET;
                game    = wxAtoi(pBuf);
                set     = AtoiGame2Set(pBuf);
                break;
            case 'T':
                mode    |= TABLE;
                table   = AtoiTable(pBuf);
                break;
            case '@':
                m_bAllPairs = true;
                break;
            default:
                Usage();
                return;
        }
        SkipDigits(pBuf);
    }

    if (!m_schema.IsOk())
    {
        OUTPUT_TEXT(_("No schema available for these data"));
        return;
    }

    switch (mode)
    {
        case PAIR:
            GuideCard( pair );
            break;
        case PAIR + ROUND:
            table = m_schema.GetTable(pair,round);
            OUTPUT_TEXT_FORMATTED (_("pair %u, round %u, table %u, direction %s, games %s, opponent %u\n"),
                        pair,round,table,
                        GetNsEwString(pair,round),
                        SetToGamesAsString(m_schema.GetSet(table,round)),
                        m_schema.GetOpponent(pair,round)
                    );
            break;
        case PAIR + SET:
            OUTPUT_TEXT_FORMATTED(_("pair %u, game %u:\n"), pair, game);
            for (round = 1; round <= m_rounds; ++round)
            {
                table = m_schema.GetTable(pair,round);
                if (m_schema.GetSet(table, round) == set)
                {
                    OUTPUT_TEXT_FORMATTED(_("pair %u, games %s, table %u, direction %s, round %u, opponent %u\n"),
                                        pair, SetToGamesAsString(set), table, GetNsEwString(pair, round), round,
                                        m_schema.GetOpponent(pair,round)
                                    );
                }
            }
            break;
        case PAIR+TABLE:
            OUTPUT_TEXT_FORMATTED(_("pair %u, table %u:\n"), pair, table);
            for (round = 1; round <= m_rounds; ++round)
            {
                if (m_schema.GetTable(pair,round) == table)
                    OUTPUT_TEXT_FORMATTED(_("pair %u, table %u, round %u, direction %s, games %s, opponent %u\n"),
                                    pair, table, round, GetNsEwString(pair,round),
                                    SetToGamesAsString(m_schema.GetSet(table,round)),
                                    m_schema.GetOpponent(pair, round)
                                );
            }
            break;
        case ROUND:
            OUTPUT_TEXT_FORMATTED(_("Round %u:\n"), round);
            for (pair = 1; pair <= m_pairs; ++pair)
            {
                table = m_schema.GetTable(pair,round);
                OUTPUT_TEXT_FORMATTED(_("pair %2u, direction %s, table %2u, games %s\n"),
                                        pair,GetNsEwString(pair,round),
                                        table, SetToGamesAsString(m_schema.GetSet(table,round))
                                    );
            }
            break;
        case ROUND + SET:
            OUTPUT_TEXT_FORMATTED(_("Round %u, game %u:\n"), round, game);
            bFound = false;
            for (table = 1; table <= m_tables; ++table)
            {
                if (m_schema.GetSet(table,round) == set)
                {
                    for (pair = 1;pair<=m_pairs; ++pair)
                    {
                        if (m_schema.GetTable(pair,round) == table)
                        {
                            bFound = true;
                            OUTPUT_TEXT_FORMATTED(_("round %u, games %s, pair %2u, direction %s, table %u\n"),
                                                    round,SetToGamesAsString(set),pair,
                                                    GetNsEwString(pair,round),table
                                                );
                        }
                    }
                }
            }
            if (!bFound)
                OUTPUT_TEXT_FORMATTED(_("games %s are not played in round %u\n"),
                                        SetToGamesAsString(set),round
                                    );
            break;
        case ROUND + TABLE:
            OUTPUT_TEXT_FORMATTED(_("Round %u, table %u:\n"), round, table);
            for (pair = 1; pair <= m_pairs; ++pair)
            {
                if ((m_schema.GetTable(pair,round) == table) && m_schema.IsNs(pair,round) )
                    OUTPUT_TEXT_FORMATTED(_("round %u, table %u, games %s, pairs %u (NS) + %u (EW)\n"),
                                            round,table,SetToGamesAsString(m_schema.GetSet(table,round)),
                                            pair, m_schema.GetOpponent(pair,round)
                                        );
            }
            break;
        case SET:
            OUTPUT_TEXT_FORMATTED(_("Game %u:\n"), game);
            for (round = 1; round <= m_rounds; ++round)
            {
                for (pair = 1; pair <= m_pairs; ++pair)
                {
                    if (m_schema.IsNs(pair,round))
                    {
                        table = m_schema.GetTable(pair,round);
                        if (m_schema.GetSet(table,round) == set)
                            OUTPUT_TEXT_FORMATTED(_("round %u, table %u, pairs %2u (NS) + %2u (EW)\n"),
                                round,table,pair,m_schema.GetOpponent(pair,round)
                            );
                    }
                }
            }
            break;
        case TABLE:
            OUTPUT_TEXT_FORMATTED(_("Table %u:\n"), table);
            for (round = 1; round <= m_rounds; ++round)
            {
                for (pair = 1; pair <= m_pairs; ++pair)
                {
                    if ( (m_schema.GetTable(pair,round) == table) && m_schema.IsNs(pair,round) ) 
                        OUTPUT_TEXT_FORMATTED(_("round %u, games %s, pairs %2u (NS) + %2u (EW)  %s\n"),
                                                round,SetToGamesAsString(m_schema.GetSet(table,round)),
                                                pair,m_schema.GetOpponent(pair,round),
                                                GetBorrowTableAsString(table,round)
                                            );
                }
            }
            break;
        default:
            Usage();
            return;
    }
}   // List()


void Debug::DoCommand(const wxString& a_cmd)
{   // input is in uppercase
    const wxChar* pBuf = a_cmd.c_str(); //(const wxChar*)a_cmd;  // NOT a cast, but function conversion from wxString to a const char buffer

    SkipWhite(pBuf);
    m_bAllPairs = false;

    switch (*pBuf++)
    {
        case 'G':               // goto new group
            {
                SkipWhite(pBuf);
                if (isalpha(*pBuf))
                {
                    UINT group;
                    for (group = 0; group < m_groupData.size(); ++group)
                    {
                        if (pBuf == m_groupData[group].groupChars) break;
                    }
                    m_group = group >= m_groupData.size() ? 1+cfg::MAX_GROUPS : group+1;
                }
                else
                    m_group = wxAtoi(pBuf);

                if ( (m_group > cfg::MAX_GROUPS) || (m_group == 0) )
                {
                    m_group = 1;
                    OUTPUT_TEXT(_("     ----> group error <---, default group 1\n"));
                }

                InitGroupData();
            }
            break;
        case 'I':       //initialize group
            {
                UINT    rounds,pairs;
                UINT    maxGroup = cfg::GetGroupData()->size();
                if (m_group <= maxGroup)    // don't touch!
                {
                    OUTPUT_TEXT_FORMATTED(_("Current group is within match-schema, take group > %u\n"), maxGroup);
                    break;
                }
                rounds = wxAtoi(pBuf); SkipDigits(pBuf);
                pairs  = wxAtoi(pBuf); SkipDigits(pBuf);
                wxArrayString schemas;
                INT_VECTOR    nameIds;
                schema::FindSchema(rounds, pairs, nameIds, &schemas);
                if (nameIds.size() == 0)
                    OUTPUT_TEXT(_(" no matching schemas found!\n"));
                else
                {
                    int id=0;
                    wxSingleChoiceDialog sc;
                    sc.SetFont(GetFont());  // child does not inherit fontsize of its parent!
                    sc.Create(this, _("choose a schema"), ES, schemas);
                    m_pConsole->AsyncTextOutBegin();  // pause the prompt
                    if ( wxID_OK == sc.ShowModal() )
                        id = sc.GetSelection();

                    m_pActiveGroupInfo->schemaId = nameIds[id];
                    m_pActiveGroupInfo->schema   = schemas[id];
                    m_pActiveGroupInfo->pairs    = pairs;
                    InitGroupData();
                    m_pConsole->AsyncTextOutEnd(); // resume prompt
                }
            }
            break;
        case 'O':
            GroupOverview();
            break;
        case 'D':               // test all present schema's
            TestSchemas();
            break;
        case 'L':               //list command
            List(pBuf);
            break;
        case 'N':               // do/do-not test in testrsp(): not used anymore, can't go wrong!
            m_bDontTest = !m_bDontTest;
            OUTPUT_TEXT_FORMATTED(_("testing is now %s for TestRSP()\n"), m_bDontTest ? _("off") : _("on"));
            break;
        default:
            --pBuf;         // get first char back
            if ((*pBuf == '-') || isdigit(*pBuf))
                CalcScore(pBuf);
            else
                List(pBuf);
            break;
    }
}   // DoCommand()

void Debug::CalcScore(const wxChar* pBuf)
{
    wxString resultAsString;
    int scoreNv = score::GetContractScoreFromString(pBuf, false, resultAsString);
    OUTPUT_TEXT(resultAsString);
    switch (scoreNv)
    {
        case CONTRACT_NOT_CONSISTENT:  // contract values not consistent, error already shown through 'resultAsString'
            break;
        case CONTRACT_MALFORMED:       // bad contract description (empty 'resultAsString')
            Usage();
            break;
        default:
        {           // we have a good result, show it
            int scoreV = score::GetContractScoreFromString(pBuf, true,  resultAsString);
            OUTPUT_TEXT_FORMATTED(_(", score vulnerable: %i, not vulnerable: %i\n"), scoreV, scoreNv);
        }
    }
}   // CalcScore()

void Debug::Usage()
{
    wxString sp(' ', prompt.size());
    sp += _(
            "^-- unknown command\n"
            "   [l]Rx[@]PySzTq = list [Round x] [Pair y] [Game z] [Table q]\n"
            "   i x y     = initialize new group with x Rounds, y Pairs\n"
            "   gx        = go to group x\n"
            "   n         = yes<->no testing of setentry\n"
            "   d         = debug: test schema's\n"
            "   o         = overview active schema\n"
           );
    sp += "   " + score::GetContractExplanation();

    OUTPUT_TEXT(sp);
}   // Usage()

void Debug::InitGroupData()
{   // assume: m_group is set correctly
    if (m_group-1 >= m_groupData.size())
    {
        cfg::GROUP_DATA data = m_groupData.back();
        while ( m_groupData.size() < m_group)       // was: m_groupData.resize(m_group);
        {   // update the groupoffset, else pairnames of first group will be used!
            data.groupOffset += data.pairs;     // so sessionpairnr will result in ""
            m_groupData.push_back(data);
        }

        m_groupData[m_group-1].groupChars = U2String(m_group);
        OUTPUT_TEXT(_("this group is not initialized (yet), defaults:\n"));
    }

    m_pActiveGroupInfo = &m_groupData[m_group-1];   // ptr to active group
    m_schema.SetId(m_pActiveGroupInfo->schemaId);   // init new schema
    m_rounds    = m_schema.GetNumberOfRounds();
    m_pairs     = m_pActiveGroupInfo->pairs;
    m_tables    = m_schema.GetNumberOfTables();
    m_setSize   = cfg::GetSetSize();
    OUTPUT_TEXT_FORMATTED(_("rounds %u, pairs %u, tables %u (%s)\n")
                            , m_rounds
                            , m_pActiveGroupInfo->pairs
                            , m_tables
                            , m_pActiveGroupInfo->schema
                        );
    SetPrompt();
    InitGuideStuff();
}   // InitGroupData()

void Debug::GroupOverview()
{   // 'old' method, don't translate!
    if (!m_schema.IsOk())
    {
        OUTPUT_TEXT(_("No schema available for these data"));
        return;
    }

    if (m_bPrintNext) prn::BeginPrint();

    UINT    round;
    UINT    table;
    UINT    ow;
    UINT    pair;
    UINT    set;
    UINT    count;
    UINT    setSize = cfg::GetSetSize();
    #define BUF_SIZE 256
    wchar_t buf[BUF_SIZE];
    UINT    width = 1+1+9+4+m_rounds*8; // width in characters of 1 line
    static const wchar_t RONDE_     [] = L" ║            ║ ronde ->";
    static const wchar_t SPELLEN_TFL[] = L" ║spellen ║tfl║";
    static const wchar_t R4         [] = L" ╠════════════╦";
    static const wchar_t R6         [] = L" ╠════════╦═══╬";
    static const wchar_t R6_2       [] = L" ╠════════╬═══╬";
    static const wchar_t RN         [] = L" ╚════════╩═══";
    #define LEN(x) (sizeof(x)/sizeof(x[0]))

/* example
╔═══════════════════════════════════════════════════════════════════════════╗
║14 paren, 6 rondes, 24 spellen                                             ║
║Schema: 6multi14_nieuw                                                     ║
╠════════╦═══╦══════════════════════════════════════════════════════════════╣
║        ║   ║ ronde ->                                                     ║
╠════════╬═══╬════════╦════════╦════════╦════════╦════════╦════════╦════════╣
║spellen ║tfl║    1   ║    2   ║    3   ║    4   ║    5   ║    6   ║    7   ║
╠════════╬═══╬════════╬════════╬════════╬════════╬════════╬════════╬════════╣
║A:  1-4 ║ 1 ║    1-2 ║    3-4 ║    5-6 ║    7-8 ║    9-10║   11-12║   13-14║
╠════════╬═══╬════════╬════════╬════════╬════════╬════════╬════════╬════════╣
║B: 13-16║ 2 ║A:  2-3 ║    4-5 ║    6-7 ║C:  8-9 ║   10-11║   12-13║A: 14-1 ║
╚════════╩═══╩════════╩════════╩════════╩════════╩════════╩════════╩════════╝
*/
    OUTPUT_TEXT('\n');
    buf[0] = L' ';
    buf[1] = L'╔';
    for (round = 2; round < width-1; ++round) buf[round] = L'═';
    wcscpy(buf+round, L"╗\n");
    OUTPUT_TEXT(buf);
    count = swprintf_s(buf, BUF_SIZE, L" ║%u paren, %u rondes, %u spellen", m_pairs, m_rounds, m_rounds*setSize);
    swprintf_s(buf+count, BUF_SIZE-count, L" %*s║\n", width-count-2, L"");
    OUTPUT_TEXT(buf);
    OUTPUT_TEXT_FORMATTED(L" ║Schema: %-*s║\n", width-11, m_pActiveGroupInfo->schema);

    count = swprintf_s(buf, BUF_SIZE, R4);
    while ( count < width)  buf[count++] = L'═';
    wcscpy(buf+width-1, L"╣\n");
    OUTPUT_TEXT(buf);
    int width_ = width - LEN(RONDE_);
    OUTPUT_TEXT_FORMATTED(L"%s%*s║\n", RONDE_, width_, ES);
    wcscpy(buf,R6);
    for (round = 1; round <= m_rounds; ++round) wcscat(buf, L"═══════╦");
    wcscpy(buf+width-1,L"╣\n");
    OUTPUT_TEXT(buf);
    count = swprintf_s(buf, BUF_SIZE, L"%s", SPELLEN_TFL);
    for (round = 1; round <= m_rounds; ++round)
        count += swprintf_s(buf+count, BUF_SIZE-count, L"  %2u   ║", round);
    wcscat(buf, L"\n");
    OUTPUT_TEXT(buf);
    for (table = 1; table <= m_tables; ++table)
    {
        wchar_t bufje[3];       // contains "a:" or "  "
        UINT     firstGame=cfg::GetFirstGame()+(table-1)*setSize;
        wcscpy(buf, R6_2);
        for (round = 1; round <= m_rounds; ++round) wcscat(buf, L"═══════╬");
        wcscpy(buf+width-1, L"╣\n");
        OUTPUT_TEXT(buf);
        if (table <= m_rounds)    // another existing set
            count = swprintf_s(buf, BUF_SIZE, L" ║%c: %2u-%-2u║%2u ║",
                'A'-1+table, firstGame, firstGame+setSize-1, table);
        else
            count = swprintf_s(buf, BUF_SIZE, L" ║        ║%2u ║", table);
        for (round = 1; round <= m_rounds; ++round)
        {
            ow = 9999;
            for (pair = 1; pair <= m_pairs;++pair)
                if ((m_schema.GetTable(pair, round) == table) && m_schema.IsNs(pair, round) )
                {
                    ow = m_schema.GetOpponent(pair, round);
                    break;
                }
            if (ow == 9999)   // this table is not played at this round!
                count += swprintf_s(buf+count, BUF_SIZE-count, L"       ║");
            else
            {
                set = m_schema.GetSet (table, round);
                if (set == table)           // 'normal' games on this table
                    wcscpy(bufje, L"  ");
                else                        // borrow from other table
                    swprintf_s(bufje, 3, L"%c:", 'A'-1+set);
                count += swprintf_s(buf+count, BUF_SIZE-count, L"%s%2u-%-2u║", bufje, pair, ow);
            }
        }
        wcscat(buf, L"\n");
        OUTPUT_TEXT(buf);
    }
    wcscpy(buf,RN);
    for (round = 1; round <= m_rounds; ++round) wcscat(buf, L"╩═══════");
    wcscpy(buf+width-1, L"╝\n");
    OUTPUT_TEXT(buf);
    if (m_tables < m_rounds)                        // only game definitions
    {
        buf[0] = buf[1] = ' ';                      // 2 spaces offset for this line
        count = 2;
        for (table = m_tables+1; table <= m_rounds; ++table)
            count += swprintf_s(buf+count, BUF_SIZE-count, L"%c: %2u-%-2u   ",
                'A'-1+table, cfg::GetFirstGame()+(table-1)*setSize, cfg::GetFirstGame()+table*setSize -1);
        wcscpy(buf+count, L"\n");
        OUTPUT_TEXT(buf);
    }

    OUTPUT_TEXT('\n');
    if (m_bPrintNext) prn::EndPrint();
}   // GroupOverview()

void Debug::TestSchemas(void)
{
    std::vector<UINT>    opponents;
    std::vector<UINT>    sets;
    SchemaInfo           schema;

    for (int schemaId=0; schema.SetId(schemaId); ++schemaId)
    {
        UINT tables = schema.Tables();
        UINT rounds = schema.Rounds();
        UINT pairs  = schema.Pairs();

        OUTPUT_TEXT_FORMATTED(_("schema for %2u tables, %2u rounds, %2u pairs (%s)\n"),
                    tables, rounds, pairs, schema.GetName());

        for (UINT pair = 1; pair <= pairs; ++pair)
        {
            opponents.clear(); opponents.resize(cfg::MAX_PAIRS_PER_GROUP+1, 0);
            sets.clear(); sets.resize(cfg::MAX_SETS+1, 0);
            UINT opponent = 0;
            for (UINT round = 1; round <= rounds; ++round)
            {
                UINT tp = schema.GetOpponent(pair,round);
                if (tp == 0)
                    OUTPUT_TEXT_FORMATTED(_("pair %u has no opponent in round %u\n"), pair, round);
                else
                    ++opponent;
                if (tp >= opponents.size())
                {
                    OUTPUT_TEXT_FORMATTED(_("pair %u has too high opponent %u in round %u\n"), pair, tp, round);
                    tp = 0;
                }
                ++opponents[tp];
                if (opponents[tp] > 1)
                    OUTPUT_TEXT_FORMATTED(_("pair %u plays %u times against pair %u\n"), pair, opponents[tp], tp);
                UINT set = schema.GetSet(schema.GetTable(pair,round),round);
                if (set >= sets.size())
                {
                    OUTPUT_TEXT_FORMATTED(_("pair %u has a too large gameset %u in round %u\n"), pair, set, round);
                    set = 0;
                }
                if (set > rounds)
                {
                    OUTPUT_TEXT_FORMATTED(_("pair %u has unknown gameset %u in round %u\n"), pair, set, round);
                    set = 0;
                }
                ++sets[set];
                if (sets[set] > 1)
                    OUTPUT_TEXT_FORMATTED(_("pair %u playes %u times set %u\n"), pair, sets[set], set);
            }   /* end for (rounds) */
            if (opponent != rounds)
                OUTPUT_TEXT_FORMATTED(_("pair %u playes only %u times\n"), pair, opponent);
        }       /* end for (paren) */

        for (UINT round = 1; round <= rounds; ++round)
        {   // check if pairs at a table are opponents
            for (UINT pair1 = 1; pair1 <= pairs; ++pair1)
            {
                UINT table1 = schema.GetTable(pair1, round);
                for (UINT pair2 = pair1+1; pair2 <= pairs; ++pair2)
                {
                    UINT table2 = schema.GetTable(pair2, round);
                    if (table1 == table2)
                    {
                        bool bPair1Ns = schema.IsNs(pair1, round);
                        bool bPair2Ns = schema.IsNs(pair2, round);
                        if ( bPair1Ns == bPair2Ns || !bPair1Ns == !bPair2Ns)
                        {
                            OUTPUT_TEXT_FORMATTED(_("in round %u, table %u pair %u and %u are playing in the same direction (%s)\n"),
                            round, table1, pair1, pair2, bPair1Ns ? _("NS") : _("EW"));
                        }
                    }
                }
            }
        }
    }
}   // TestSchemas()

void Debug::PrintScoreSlips(UINT a_setSize, UINT a_firstSet, UINT a_nrOfSets, UINT a_repeatCount, const wxString& a_extra)
{
#if 0
    if (a_setSize > 6)
    {
        wxString errorMsg = FMT(_("Set-size(%u) too large,\nmaximum is 6"), a_setSize);
        MyMessageBox(errorMsg,_("Scoreslips"));
        return;
    }
#endif
/* score slips
                10   15   20   25   30   35   40   45   50   55   60   65   70   75
       .1...5....0....5....0....5....0....5....0....5....0....5....0....5....0....5.... V1 V2 V3 V4 V5
     0 ================================================================================  |  |  |  |  |
     1 |                         |      |    Contract     |         |                 |  |  |  |  |  |
     2 | ROUND :                 | Game |   NS       EW   |Resultaat| Score NS + or - |  |  |  |  |  |
     3 |=========================|======|===========================|=================|  =  |  |  |  | 
     4 | TABLE :                 |  X   |*       |*       |         |                 |  3  |  |  |  |
     5 |=========================|======|========|========|=========|=================|     =  |  |  |
     6 |    NS :                 |  X+1 |*       |*       |         |                 |     5  |  |  |
     7 |=========================|======|========|========|=========|=================|        =  |  |
     8 |    EW :                 |  X+2 |*       |*       |         |                 |        7  |  |
     9 |=========================|======|========|========|=========|=================|           =  |
       ~                         ~      ~        ~        ~         ~                 ~           9  |
    10 |ini.NS      |ini.EW      | X+N-1|*       |*       |         |                 |              |
    11 ================================================================================              =
       -----H1----->13                                                                              11
       -----H2----------------->26
       -----H3------------------------>33
       -----H4--------------------------------->42
       -----H5------------------------------------------>51
       -----H6---------------------------------------------------->61
       -----H7---------------------------------------------------------------------->79
*/

    #define ARRAY_LEN(x) (sizeof(x)/sizeof((x)[0]))

    #define SL_H0  0    /* SLip position/size H0 etc*/
    #define SL_H1 13
    #define SL_H2 26
    #define SL_H3 33
    #define SL_H4 42
    #define SL_H5 51
    #define SL_H6 61
    #define SL_H7 79

    #define SL_V0 0
    #define SL_V1 3
    #define SL_V2 5
    #define SL_V3 7
    #define SL_V4 9
    #define SL_V5 11    /* if 4 or less games else 11 + (games-4)*2  */

    UINT firstSet       = a_firstSet;       // input param
    UINT setSize        = a_setSize;        // input param
    UINT nrOfSets       = a_nrOfSets;       // input param
    UINT repeatCount    = a_repeatCount;    // input param
    wxString extra      = a_extra;          // input param
    
    int maxVPos         = SL_V5 + (setSize > 4 ? (setSize - 4)*2 : 0);

    prn::table::Line lines[]=
    {
       {{SL_H0,SL_V0    }, {SL_H0,maxVPos}}
     , {{SL_H2,SL_V0    }, {SL_H2,maxVPos}}
     , {{SL_H3,SL_V0    }, {SL_H3,maxVPos}}
     , {{SL_H5,SL_V0    }, {SL_H5,maxVPos}}
     , {{SL_H6,SL_V0    }, {SL_H6,maxVPos}}
     , {{SL_H7,SL_V0    }, {SL_H7,maxVPos}}
     , {{SL_H4,SL_V1    }, {SL_H4,maxVPos}}
     , {{SL_H1,maxVPos-2}, {SL_H1,maxVPos}} // last line with initials
    };
    size_t lineCount = ARRAY_LEN(lines);

    prn::table::Text texts[] =
    {
          {{SL_H0+2, SL_V0+1}  , extra           }  // extra text
        , {{SL_H0+2, SL_V0+2}  , _("ROUND :"    )}  // TRANSLATORS: translated strings 'ROUND :' , 'TABLE :', '   NS :' and '   EW :' should have SAME size!
        , {{SL_H0+2, SL_V1+1}  , _("TABLE :"    )}  // TRANSLATORS: translated strings 'ROUND :' , 'TABLE :', '   NS :' and '   EW :' should have SAME size!
        , {{SL_H0+2, SL_V2+1}  , _("   NS :"    )}  // TRANSLATORS: translated strings 'ROUND :' , 'TABLE :', '   NS :' and '   EW :' should have SAME size!
        , {{SL_H0+2, SL_V3+1}  , _("   EW :"    )}
        , {{SL_H0+1, maxVPos-1}, _("ini.NS"     )}
        , {{SL_H1+1, maxVPos-1}, _("ini.EW"     )}
        , {{SL_H2+1, SL_V0+2}  , CenterText(_("Game"           ),  6)}
        , {{SL_H3+1, SL_V0+1}  , CenterText(_("Contract"       ), 17)}  // TRANSLATORS: translated string should have SAME size as original text!
        , {{SL_H3+1, SL_V0+2}  , CenterText(_("NS       EW"    ), 17)}  // above comment works on this line!
        , {{SL_H5+1, SL_V0+2}  , CenterText(_("Result"         ),  9)}
        , {{SL_H6+1, SL_V0+2}  , CenterText(_("NS score + or -"), 17)}
    };
    size_t textCount = ARRAY_LEN(texts);

    prn::table::TableInfo table =
    {
        {2,0},      // start at 2 chars from left-side and first line of page
        lines,      // set of (static) H+V Lines to draw
        texts,      // set of texts to print
        lineCount,  // number of (static) lines
        textCount   // number of (static) texts
    };

    UINT nrOfHLines = 6 + (a_setSize > 4 ? a_setSize - 4 : 0);
    table.linesV.reserve(nrOfHLines);
    table.textsV.reserve(setSize*(size_t)3);
    int vPos = SL_V0;
    for (UINT hLine = 0; hLine < nrOfHLines; ++hLine)
    {
        table.linesV.push_back({ {SL_H0,vPos} , {SL_H7,vPos} });
        vPos += (vPos == SL_V0) ? 3 : 2;
    }

    int originY = table.origin.y;   // needed when starting new-page

    MyPrint myPrint(m_bPrintNext ? nullptr : &m_consoleOutput); // zero: no screen output, only real print, non-zero: only output to wxArrayString a_pTextOutput

    myPrint.BeginPrint(_("Scoreslips"));
    for (; repeatCount; --repeatCount)
    {
        UINT firstGame    = (firstSet - 1)*setSize + 1;
        UINT linesPrinted = 0;
        for (UINT slips = 1 ; slips <= nrOfSets; ++slips)
        {
            table.textsV.clear();
            vPos = SL_V1+1;
            for (UINT game = 0; game < setSize ; ++game, vPos += 2)
            {
                table.textsV.push_back( {{SL_H2+3, vPos}, FMT("%2u", game + firstGame)                   } );
                table.textsV.push_back( {{SL_H3+1, vPos}, score::VulnerableChar(game + firstGame, true ) } );
                table.textsV.push_back( {{SL_H4+1, vPos}, score::VulnerableChar(game + firstGame, false) } );
            }
            if (linesPrinted + maxVPos >= prn::GetLinesPerPage())
            {
                linesPrinted   = 0;
                table.origin.y = originY;
                myPrint.EndPage();              // force new page
            }
            myPrint.PrintTable(table);          // 1 slip
            table.origin.y += maxVPos+3;
            linesPrinted   += maxVPos+3;
            firstGame      += setSize;
        }   // end slips
        table.origin.y = originY;
        myPrint.EndPage();
    }   // end repeatCount
    myPrint.EndPrint();
}   // PrintScoreSlips()

static wxString MakeSeparator(UINT len)
{
    len += len & 1; // make even, so loop will end...
    wxString sep;
    while (len) {sep += ". "; len -=2; }
    return sep;
}   // MakeSeparator()

void Debug::PrintGuideNew(UINT a_pair)
{   // print guidecard(s), if a_pair == 0, print all cards
/* example guide-card:
                 10   15   20   25   30   35
        .1...5....0....5....0....5....0....5..   V1 V2 V3 V4 V5
      0 ds Janssen - Pietersen                    |  |  |  |
      1 extra info................extra      .    |  |  |  |
      2 ==================================   .    =  |  |  |
      3 | Pair 13               14 pairs |   .    2  |  |  |
      4 | schema:               4stayr14 |   .       |  |  |
      5 |================================|   .       =  |  |
      6 |round| table |opp. |  games     |   .       5  |  |
      7 |================================|   .          =  |
      8 |  1  |  7 NS |   6 |  9 - 12  B3|   .          7  |
      . |  2  |  2 EW |   8 |  5 - 8     |   .             |
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                 |
      . |  N  |  7 NS |   2 | 13 - 16  B4|   .             |
      . ==================================   .             =
                                                         7+N+1  where N = number of rounds
      . .   .   .   .   .   .   .   .   .   .
        --H1-->6
        --H2---------->14
        --H3---------------->20
        --H4----------------------------->33
        --H5-------------------------------->37
        
*/
#define G_H0    0
#define G_H1    6
#define G_H2    14
#define G_H3    20
#define G_H4    33
#define G_H5    37

#define G_V0    0
#define G_V1    2
#define G_V2    5
#define G_V3    7
#define G_V4    (m_rounds+G_V3+1)
#define LINES_IN_GUIDE          ((int)(G_V4+1))
#define LINES_PRINTED_PER_CARD  (LINES_IN_GUIDE+2)

    prn::table::Line lines[]=
    {
          {{G_H0,G_V1},{G_H4,G_V1}}     // first horizontal  line
        , {{G_H0,G_V2},{G_H4,G_V2}}     // second horizontal line
        , {{G_H0,G_V3},{G_H4,G_V3}}     // third horizontal line
        , {{G_H0,G_V3+(int)m_rounds+1},{G_H4,G_V3+(int)m_rounds+1}}  // last horizontal line
        , {{G_H0,G_V1},{G_H0,G_V3+(int)m_rounds+1}}  // first v-line
        , {{G_H4,G_V1},{G_H4,G_V3+(int)m_rounds+1}}  // last v-line
        , {{G_H1,G_V2},{G_H1,G_V3+(int)m_rounds+1}}  // column round
        , {{G_H2,G_V2},{G_H2,G_V3+(int)m_rounds+1}}  // column table
        , {{G_H3,G_V2},{G_H3,G_V3+(int)m_rounds+1}}  // column opponent
    };
    size_t lineCount = ARRAY_LEN(lines);

    wxString schema = _("Schema");
    schema = schema.Left(12);     // assume reasonable size!
    schema += FMT(": %*s", 30 - 2 - static_cast<int>(schema.Len()), m_pActiveGroupInfo->schema);
    prn::table::Text texts[]=
    {
          {{G_H0+1,G_V2+1}, CenterText(_("round"),  5)}
        , {{G_H1+1,G_V2+1}, CenterText(_("table"),  7)}
        , {{G_H2+1,G_V2+1}, CenterText(_("opp." ),  5)}
        , {{G_H3+1,G_V2+1}, CenterText(_("games"), 12)}
        , {{G_H0+0,G_V0+1}, m_pTxtCtrlExtra->GetValue()}
        , {{G_H0+2,G_V1+2}, schema}
    };
    size_t textCount = ARRAY_LEN(texts);

    prn::table::TableInfo tableInfo =
    {
        {0,0},      // start at left top of page
        lines,      // static set of horizontal lines to draw
        texts,      // static set of texts to print
        lineCount,  // number of static horizontal lines
        textCount   // number of static texts
    };

    bool bAllPairs  = a_pair == 0;
    UINT pair       = bAllPairs ? 1 : a_pair;

    MyPrint myPrint(m_bPrintNext ? nullptr : &m_consoleOutput); // zero: no screen output, only real print, non-zero: only output to wxArrayString a_pTextOutput

    myPrint.BeginPrint(_("Guides"));
    UINT linesPrinted = 0;
    for ( ; pair <= m_pairs; ++pair)
    {   // act, asif all cards must be printed. Conditions are checked at the end of the loop.

        tableInfo.textsV.clear();   // only dynamic texts change per pair, so clear them
        // now build the dynamic parts
        tableInfo.textsV.push_back({{G_H0+0,G_V0+0}, names::PairnrSession2GlobalText(pair+m_pActiveGroupInfo->groupOffset)});
        // TRANSLATORS: translated string should have SAME size as original!
        tableInfo.textsV.push_back({{G_H0+2,G_V1+1}, FMT(_("Pair %2s%-2u             %2u pairs"), m_pActiveGroupInfo->groupChars, pair, m_pairs)});
        for (UINT round = 1; round <= m_rounds; ++round)
        {
            tableInfo.textsV.push_back({{G_H0+1,G_V3+(int)round}, FMT("%2u", round)});  //ronde
            UINT table  = m_schema.GetTable(pair, round);
            UINT set    = m_schema.GetSet  (table,round);
            if (set != 0)
            {   // set/table == 0 means: pair does not play this round
                tableInfo.textsV.push_back({{G_H1+2,G_V3+(int)round}, FMT("%2u %s" , table,GetNsEwString(pair,round))});    // table + NS/EW
                tableInfo.textsV.push_back({{G_H2+2,G_V3+(int)round}, FMT("%2u"    , m_schema.GetOpponent(pair,round))});   // opponent
                tableInfo.textsV.push_back({{G_H3+2,G_V3+(int)round}, FMT("%s  %2s",SetToGamesAsString(set),GetBorrowTableAsString(table, round))});  // games/borrow-table
            }
        }

        if (bAllPairs)
        {   // add '.' separator
            for (int line = 0; line < LINES_IN_GUIDE; ++line)
            {
                tableInfo.textsV.push_back({{G_H5,line},"."});
            }

            tableInfo.textsV.push_back({{G_H0,LINES_IN_GUIDE+1}, MakeSeparator(G_H5+1)});
        }
     
        myPrint.PrintTable(tableInfo);      // print current guide
        if (!bAllPairs) break;              // we are done!

        if ( (pair & 1) && (2*G_H5 <= prn::GetCharsPerLine() )) // check if 2 guides fit on 1 line
        {   // next card will be placed at the right side of current one
            tableInfo.origin.x = G_H5+4;
        }
        else
        {   // prepare for next row of cards
            tableInfo.origin.x  = 0;
            linesPrinted       += LINES_PRINTED_PER_CARD;
            tableInfo.origin.y += LINES_PRINTED_PER_CARD+1;
            if (linesPrinted + LINES_IN_GUIDE > prn::GetLinesPerPage())
            {
                linesPrinted        = 0;
                tableInfo.origin.y  = 0;
                myPrint.EndPage();      // force new page
            }
        }
    }   // for all pairs
    myPrint.EndPrint();
}   // PrintGuideNew()

void Debug::PrintSchemaOverviewNew()
{
#define O_H0    0
#define O_H1    9
#define O_H2    13
#define O_H1    9
#define O_DH    8

#define O_V0    0
#define O_V1    3
#define O_V2    5
#define O_V3    7
#define O_DV    2

/*
                 5   10   15   20   25   30   35   40   45   50   55   60
            0....5....0....5....0....5....0....5....0....5....0....5....0.        V1 V2 V3  Vn
            ╔════════════════════════════════════════════════════════════╗  0      |  |  |   |
            ║14 pairs, 6 rounds, 24 games                                ║  1      |  |  |   |
            ║Schema: 6multi14                                            ║  2      |  |  |   |
            ╠════════════╦═══════════════════════════════════════════════╣  3      =  |  |   |
            ║            ║ round ->                                      ║  4      3  |  |   |
            ╠════════╦═══╬═══════╦═══════╦═══════╦═══════╦═══════╦═══════╣  5         =  |   |
            ║games   ║tbl║   1   ║   2   ║   3   ║   4   ║   5   ║   6   ║  6 DV      5  |   |
            ╠════════╬═══╬═══════╬═══════╬═══════╬═══════╬═══════╬═══════╣  7  =         =   |
            ║A:  1-4 ║ 1 ║   1-2 ║   6-11║   8-3 ║   5-4 ║D: 5-12║   9-12║  8  |         7   |
            ╠════════╬═══╬═══════╬═══════╬═══════╬═══════╬═══════╬═══════╣  9  =             |
            ║B:  5-8 ║ 2 ║   3-4 ║  12-7 ║   6-9 ║  11-13║   1-8 ║   5-2 ║     2             |
                                                                                             .
                                                                                             .
            ╠════════╬═══╬═══════╬═══════╬═══════╬═══════╬═══════╬═══════╣                   .
            ║F: 21-24║ 6 ║  11-12║   2-3 ║  10-5 ║   9-8 ║  13-6 ║   1-4 ║                   .
            ╠════════╬═══╬═══════╬═══════╬═══════╬═══════╬═══════╬═══════╣                   .
            ║        ║ 7 ║A:13-14║D:14-9 ║E:14-1 ║B:10-14║F: 7-14║C:11-14║                   .
            ╚════════╩═══╩═══════╩═══════╩═══════╩═══════╩═══════╩═══════╝                   =
            0....5....0....5....0....5....0....5....0....5....0....5....0.               V3+Table*DV
                 5   10   15   20   25   30   35   40   45   50   55   60
                         <--DH--->8
            --H1----->9
            --H2--------->13
            --Hx------------------------->H2+ DH*2
            --Hn---------------------------------------------------------> H2+DH*Rounds

*/
    prn::table::Line lines[]=
    {
          {PNT(O_H0, O_V0)                , PNT(O_H2+O_DH*m_rounds  , O_V0)                       }   // top-line
        , {PNT(O_H0, O_V0)                , PNT(O_H0                , O_V1 + O_DV*(m_tables+2))   }   // left side
        , {PNT(O_H2+(m_rounds*O_DH),O_V0) , PNT(O_H2+(m_rounds*O_DH), O_V2 + O_DV*(m_tables+1))   }   // right side
        , {PNT(O_H1, O_V2)                , PNT(O_H1                , O_V2 + O_DV*(m_tables+1))   }   // v between games/table
        , {PNT(O_H2, O_V1)                , PNT(O_H2                , O_V2 + O_DV*(m_tables+1))   }   // v after table
    };
    size_t lineCount = ARRAY_LEN(lines);

    prn::table::Text texts[]=
    {
          {PNT(O_H2+2, O_V1+1), _("round ->")}
        , {PNT(O_H0+1, O_V2+1), CenterText(_("games"), 8)}
        , {PNT(O_H1+1, O_V2+1), CenterText(_("tbl"  ), 3)}
        , {PNT(O_H0+1, O_V0+1), FMT(_("%u pairs, %u rounds, %u games"), m_pairs, m_rounds, m_rounds*cfg::GetSetSize())}
        , {PNT(O_H0+1, O_V0+2), FMT("%s: %s", _("Schema"), m_pActiveGroupInfo->schema)}
    };
    size_t textCount = ARRAY_LEN(texts);

    prn::table::TableInfo tableInfo =
    {
        {0,0},      // start at left top of page
        lines,      // static set of horizontal lines to draw
        texts,      // static set of texts to print
        lineCount,  // number of static horizontal lines
        textCount   // number of static texts
    };

    for (UINT hor = 0; hor <= m_tables + 2; ++hor)
    {   // add all dynamic horizontal lines
        prn::table::Line aa(PNT(O_H0,O_V1+hor*O_DV),PNT(O_H2+O_DH*m_rounds,O_V1+hor*O_DV));
        tableInfo.linesV.push_back( aa );
    }
    for (UINT ver = 1; ver < m_rounds; ++ver)   // last line not needed: already done in static lines: right side
    {   // add all dynamic vertical lines
        prn::table::Line aa(PNT(O_H2+O_DH*ver,O_V2),PNT(O_H2+O_DH*ver,O_V3+m_tables*O_DV));
        tableInfo.linesV.push_back( aa );
    }

    for (UINT table = 1; table <= m_tables; ++table)
    {
        if (table <= m_rounds)  // show set-info
            tableInfo.textsV.push_back( {PNT(O_H0+1,O_V2+table*O_DV+1), FMT("%c: %s",'A'-1+table, SetToGamesAsString(table, false))});
        tableInfo.textsV.push_back    ( {PNT(O_H1+1,O_V2+table*O_DV+1), FMT("%2u", table)});
        // POS(round,table) points to the infoblocks for the rounds/tables where POS(1,1) is the topleft block
        #define POS(x,y) PNT(O_H2 + (x-1)*O_DH + 1, O_V3 + (y-1)*O_DV + 1)
        schema::GameInfo info;
        for (UINT round = 1; round <= m_rounds; ++round)
        {
            if (table == 1) tableInfo.textsV.push_back( {POS(round, 0), FMT("%4u", round)});    // print roundnr itself
            if (m_schema.GetTableRoundInfo(table, round, info))
            {   // table has players
                wxString setData = (info.set == table) ? ES : FMT("%c:",'A'-1+info.set );
                tableInfo.textsV.push_back({POS(round, table), FMT("%2s%2u-%-2u", setData, info.pairs.ns, info.pairs.ew)});
            }
        }
    }

    if (m_tables < m_rounds)                        // only set definitions left
    {
        wxString tmp;
        for (UINT table = m_tables+1; table <= m_rounds; ++table)
        {
            tmp += FMT("%c: %s   ", 'A'-1+table, SetToGamesAsString(table, false));
        }
        tableInfo.textsV.push_back({PNT(0, O_V3+O_DV*m_tables+1), tmp});    // rest of gameset description at bottom of table
    }

    MyPrint myPrint(m_bPrintNext ? nullptr : &m_consoleOutput); // zero: no screen output, only real print, non-zero: only output to wxArrayString a_pTextOutput
    myPrint.BeginPrint(_("Schemaoverview"));
    myPrint.PrintTable(tableInfo);      // print current guide
    myPrint.EndPrint();
}   // PrintSchemaOverviewNew()

wxString Debug::CenterText(const wxString& a_text, size_t a_length)
{   // add spaces in front of the supplied text, so the new string will be centered in a space of 'a_length' characters
    if (a_length > 100) a_length = 100; //maximize stringlength to 100
    size_t len = a_text.Len();
    if (len > a_length) return a_text.substr(0, a_length);  // return 'a_length' chars at max
    size_t dif = a_length - len;
    return wxString(' ', dif/2) + a_text;   // add spaces to the front of the string
}   // CenterText()

void Debug::OnExportSchema(wxCommandEvent&)
{   // export active group-schema
    bool bResult = schema::ExportSchema(m_pActiveGroupInfo->schema);
    if (!bResult) {}
}   // OnExportSchema()
