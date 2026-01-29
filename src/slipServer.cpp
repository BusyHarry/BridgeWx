// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/button.h>
#include <wx/fswatcher.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/socket.h>
#include <wx/textctrl.h>
#include <wx/wfstream.h>
#include <wx/wxcrtvararg.h>

#include "cfg.h"
#include "main.h"
#include "names.h"
#include "printer.h"
#include "score.h"
#include "slipServer.h"

static std::vector< std::vector<score::GameSetData> > svGameSetData;
#define CHOICE_ROUND    "ChoiceRound"
static const auto S1((size_t)1);
static const auto BORDERSIZE(5);

/*******
- the file with results of html-scoreslip entry *.slipdata
- file has lines like:
. <date> <time> <id> ";slipresult format: {<gamenr>, <declarer>, <level>, <suit>, <over/under tricks>, <doubled>, <NSscore>}
. <date> <time> <id> "login for group: <group>, table: <table>, groups: <groups>, fRound: <fRound>, rounds: <rounds>, games: <games>
. <date> <time> <id> "session: <session>, group: <group>, table: <table>, round: <round>, ns: <nsId>, ew: <ewId>, slipResult: <slipresult>
. <date> <time> <id> "ready session: <session>, group: <group>, table: <table>, round: <round>
where:
- <date>       -> 2025.09.21
- <time>       -> 14:41:59
- <id>         -> <group>.<table>
- <session>    -> session id: between 0 and nr of sessions
- <group>      -> group number, 1 based
- <groups>     -> number of groups
- <table>      -> table nr, 1 based
- <round>      -> round nr, 1 based
- <rounds>     -> number of rounds
- <fRound>     -< forced round (not starting round at 1)
- <games>      -> number of games
- <nsId>       -> session pairnr of ns pair
- <ewId>       -> session pairnr of ew pair
- <slipResult> -> repeated entries of "result format", separated by '@'
- <format>     -> {<gamenr>, <declarer>, <level>, <suit>, <over/under tricks>, <doubled>, <NSscore>}
where:
- <gamenr>   -> result for this game
- <declarer> -> game leader
    0 -> nodata(ignore this entry)
    1 -> NP(not played)
    2 -> PASS(bye)
    3 -> North
    4 -> East
    5 -> South
    6 -> West
- <level> -> contract level from 1 to 7
- <suit>  -> type of contract
    1 -> clubs
    2 -> diamonts
    3 -> hearts
    4 -> spades
    5 -> NoTrump
- <over/under tricks> from +6 to -13
- <doubled> doubled or not
    0 -> not doubled
    1 -> doubled
    2 -> re-doubled
- <NSscore> the score of NS
example:
2025.09.21 18:11:11 1.2 ;slipresult format: {<gamenr>, <declarer>, <level>, <suit>, <over/under tricks>, <doubled>, <NSscore>}
2025.09.21 15:25:18 1.2 login for group: 1, table: 1, groups: 2, fRound: 0, rounds: 7, games: 28
2025.09.21 16:13:32 1.2 session: 3, group: 2, table: 5, round: 1, ns: 17, ew: 18, slipresult: {17, 0, 1, 1, 0, 0, 0}@{18, 0, 1, 1, 0, 0, 0}@{19, 4, 4, 3, 0, 0, -620}@{20, 0, 1, 1, 0, 0, 0}
2025.09.21 17:08:38 1.2 ready session: 3, group: 2, table: 3, round: 7
*****/

SlipServer::SlipServer(wxWindow* a_pParent, UINT a_pageId) : Baseframe(a_pParent, a_pageId), m_theGrid(0)
{
    m_linesReadInResult = 0;
    m_logFile     = FMT("%s%s_%u.rx.log" , cfg::GetActiveMatchPath(), cfg::GetActiveMatch(), cfg::GetActiveSession());
    m_tempLogFile = FMT("%s%s_%u.tmp.log", cfg::GetActiveMatchPath(), cfg::GetActiveMatch(), cfg::GetActiveSession());
    wxRemoveFile(m_logFile);
    // m_pChoiceBoxRound MUST exist before calling SetupGrid()
    m_pChoiceBoxRound = new MY_CHOICE(this, _("Round:"), _("The info for this round"), Unique(CHOICE_ROUND));
    m_pChoiceBoxRound->Bind(wxEVT_CHOICE, &SlipServer::OnSelectRound, this);
    m_theGrid = new MyGrid(this, "GridSlipServer");
    m_theGrid->CreateGrid(0, 0);        // create empty grid
    m_theGrid->SetSelectionMode(wxGrid::wxGridSelectNone);  // no selection allowed
    m_theGrid->DisableDragRowSize();    // disable row/column resize
    m_theGrid->DisableDragColSize();
    const auto FACTOR = 1.2f;
    m_theGrid->SetLabelFont       (m_theGrid->GetLabelFont()      .Scale(FACTOR));
    m_theGrid->SetDefaultCellFont (m_theGrid->GetDefaultCellFont().Scale(FACTOR));
    m_theGrid->SetDefaultRowSize  (m_theGrid->GetDefaultRowSize() * FACTOR);
    // create command buttons
    auto okCancel = CreateOkCancelButtons();
    wxArrayString choices;
    choices.Insert(_("file"   ), (size_t)InputChoice::InputFile   );
    choices.Insert(_("network"), (size_t)InputChoice::InputNetwork);
    m_pInputChoice = CreateRadioBox(_("select input method"), choices, EVT_CMD_HANDLER( &SlipServer::OnInputChoice), "SlipInputChoice");
    m_pInputChoice->SetItemToolTip(0U, FMT(_("data is coming from file '%s' in the match-folder"),GetSlipResultsFile(true)));
    m_pInputChoice->SetItemToolTip(1U,     _("data is coming through the network, directly from the php-server"));

    auto nextRound = new wxButton(this, wxID_ANY, _("++round"));
    nextRound->SetToolTip(_("Info for the next round"));
    nextRound->Bind(wxEVT_BUTTON, &SlipServer::OnNextRound, this );

    auto clearLog = new wxButton(this, wxID_ANY, _("clear log"));
    clearLog->SetToolTip(_("Clear the log window"));
    clearLog->Bind(wxEVT_BUTTON, &SlipServer::OnClearLog, this);

    auto pButtonHtmlSlips = new wxButton(this, wxID_ANY, _("html slip data"));
    pButtonHtmlSlips->SetToolTip(_("generate configuration data for score entry through a html-page"));
    pButtonHtmlSlips->Bind(wxEVT_BUTTON,&SlipServer::OnGenHtmlSlipData, this);

    wxBoxSizer* hBox = new wxBoxSizer(wxHORIZONTAL);
    hBox->Add  (m_pInputChoice   , wxSizerFlags(0).Border(wxALL, BORDERSIZE));
    hBox->MyAdd(m_pChoiceBoxRound, wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());
    hBox->Add  (nextRound        , wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());
    hBox->Add  (clearLog         , wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());
    hBox->AddStretchSpacer(1000);   // 'generate slip-button' in the middle
    hBox->Add  (pButtonHtmlSlips , wxSizerFlags(1).Border(wxALL, BORDERSIZE).Bottom());
    hBox->AddStretchSpacer(1000);
    hBox->Add (okCancel          , wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());

    m_pLog = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);
    wxString msg = FMT("%s --> %s", wxGetHostName(), GetMyIpv4() );
    Add2Log(msg, true); // show hostname and host-ip in log and statusbar
    SetStatusbarText(msg);
    // add to layout
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Slip server, automatic handling of slip-data"));
    vBox->Add(m_theGrid, wxSizerFlags(0).Expand().Border(wxALL, BORDERSIZE));
    vBox->Add(m_pLog   , wxSizerFlags(1).Expand().Border(wxALL, BORDERSIZE));
    vBox->Add(hBox     , 0);   //no borders/align: already done in hBox!
    SetSizer(vBox);     // add to panel

    m_bDataChanged      = false;
    m_bCancelInProgress = false;
    m_pFsWatcher        = nullptr;
    m_pSocketServer     = nullptr;
    RefreshInfo();                                          // now fill the grid with data
    HandleInputSelection(m_pInputChoice->GetSelection());   // start input selection
    AUTOTEST_ADD_WINDOW(pButtonHtmlSlips, "GeneratePhp");
    AUTOTEST_ADD_WINDOW(this            , "Panel"      );
    m_description = "SlipServer";
}   // SlipServer()

SlipServer::~SlipServer()
{
    wxRemoveFile(m_tempLogFile);
    CreateFileWatcher   (false);
    CreateNetworkWatcher(false);
}   // ~SlipServer()

void SlipServer::SetupGrid()
{   // (re-)setup the grid for a match
    svGameSetData = *score::GetScoreData();     // get the current game data, need it verify..
    m_bDataChanged      = false;
    m_bCancelInProgress = false;
    m_linesReadInResult = 0;                    // fresh start for results-file
    // deleting zero rows/columns when none exist, will give an assert??????
    auto nrOfRows = m_theGrid->GetNumberRows();
    if ( nrOfRows ) m_theGrid->DeleteRows(0, nrOfRows);
    auto nrOfCols = m_theGrid->GetNumberCols();
    if ( nrOfCols ) m_theGrid->DeleteCols(0, nrOfCols);
    const auto& groupData = *cfg::GetGroupData();
    m_groups = groupData.size();
    m_theGrid->AppendCols(1+m_groups);                  // need 1 column for each group, column 0 is used as tablenr/label
    m_theGrid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    m_theGrid->SetColLabelValue(0, _("table"));
    m_theGrid->SetRowLabelSize(0);                      // we don't use row-labels: can't suppress row-selection
    wxGridCellAttr* pAttribC0 = new(wxGridCellAttr);    // SetColAttr() takes ownership!
    pAttribC0->SetAlignment(wxALIGN_CENTER_HORIZONTAL, wxALIGN_CENTER_VERTICAL);
    m_theGrid->SetColAttr(0, pAttribC0);

    const auto COLUMN_SIZE = 10 * GetCharWidth();
    for ( UINT group = 1; group <= m_groups; ++group )
    {
        m_theGrid->SetColSize(group, COLUMN_SIZE);
        m_theGrid->SetColLabelValue(group, m_groups == 1 ? wxString("1") : groupData[group-1].groupChars);
        wxGridCellAttr* pAttrib = new(wxGridCellAttr);  // SetColAttr() takes ownership!
        pAttrib->SetAlignment(wxALIGN_CENTER_HORIZONTAL, wxALIGN_CENTER_VERTICAL);
        m_theGrid->SetColAttr(group, pAttrib);
    }
    m_maxTable = 0;
    m_tables.clear();
    m_tables.push_back(0U); // m_tables one-based, entry 0 = dummy

    for ( const auto& group : groupData )
    {   // determine the maximum nr of tables of/for all groups
        SchemaInfo schema(group.schemaId);
        UINT tables = schema.GetNumberOfTables();
        m_tables.push_back(tables);
        m_maxTable = std::max(m_maxTable, tables);
    }

    auto tableColor = cfg::GetLightOrDark({ 220,220,220,255 }); // just a bit different from default label-color
    for ( UINT table = 1; table <= m_maxTable; ++table )
    {   // create for each table its columns, column 0 is simulated row-label!
        m_theGrid->AppendRows(1);
        m_theGrid->SetCellValue(table-1, 0, FMT("%u", table));
        m_theGrid->SetCellBackgroundColour(table-1, 0, tableColor);
        for ( UINT group = 1; group <= m_groups; ++group )
        {
            if ( table > m_tables[group] ) // mark 'not present'
                m_theGrid->SetCellValue(table-1, group, "-");
        }
    }

    m_theGrid->AppendRows(1);   // create the 'match-ready' row
    m_theGrid->SetCellValue(m_maxTable, 0 , _("ready"));
    m_theGrid->SetCellBackgroundColour(m_maxTable, 0, tableColor);

    m_theGrid->EnableEditing(false);
    m_theGrid->SetCellHighlightPenWidth(0);
    m_theGrid->SetCellHighlightROPenWidth(0);

    m_rounds       = SchemaInfo(groupData[0].schemaId).GetNumberOfRounds();
    m_activeRound  = 1;
    m_pChoiceBoxRound->Init(m_rounds, m_activeRound-1); //m_activeRound is 1 based!
    m_tableInfo.resize(m_rounds+S1); // init m_tableInfo[m_rounds+S1][m_groups+S1][m_maxTable+S1]
    for ( auto& grp : m_tableInfo )
    {
        grp.resize(m_groups+S1);
        for ( auto& tbl : grp )
            tbl.resize(m_maxTable+S1);
    }
    for ( UINT round = 1; round <= m_rounds; ++round )
    {   // initialise the table info for all rounds
        UpdateTableInfo(round, false);
    }
}   // SetupGrid()

void SlipServer::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    AutoTestAddWindowsNames(a_pFile, m_description);
    AutoTestAddGridInfo    (a_pFile, m_description, m_theGrid->GetGridInfo());
}   // AutotestRequestMousePositions()

void SlipServer::RefreshInfo()
{   // update grid with actual info
    // LogMessage("SlipServer::RefreshInfo()");
    // BeginBatch()/EndBatch() results in columnlabels A B C etc ????
    if ( ConfigChanged() || m_bCancelInProgress )
        SetupGrid();                    // dynamically change row/column info
    UpdateTableInfo(m_activeRound);     // get/show updated data
    Refresh();
    Layout();
}   // RefreshInfo()

void SlipServer::UpdateTableInfo(UINT a_round, bool a_bUpdateDisplay)
{
    const auto& groupData = *cfg::GetGroupData();
    for ( UINT group = 1; group <= m_groups; ++group )
    {
        auto grp = groupData[group-1];
        const SchemaInfo schema(grp.schemaId);
        UINT tables = schema.GetNumberOfTables();
        for ( UINT table = 1; table <= tables; ++table )
        {
            TableBackground tbg;
            UINT set   = schema.GetSet  (table, a_round);
            auto pairs = schema.GetPairs(table, a_round);

            if ( pairs.ns == grp.absent || pairs.ew == grp.absent )
                set = 0;
            if ( set )
            {
                m_tableInfo[a_round][group][table].bPresent = true;
                bool bReady = HasPlayed(pairs, set, grp.groupOffset);
                m_tableInfo[a_round][group][table].bReady = bReady;
                tbg = bReady? TableBackground::Ready : TableBackground::NotReady;
            }
            else
                tbg = TableBackground::NotPresent;

            if ( a_bUpdateDisplay )
                DisplayTableReady(group, table, tbg);
        }
    }

    if ( a_bUpdateDisplay )
        DisplayGroupsReady();   // match-ready info
}   // UpdateTableInfo()

void SlipServer::OnSelectRound(wxCommandEvent&)
{
    AUTOTEST_BUSY("OnSelectRound");
    m_activeRound = 1 + m_pChoiceBoxRound->GetSelection();
    LogMessage("SlipServer::SelectRound(%u)", m_activeRound);
    RefreshInfo();
}   // OnSelectRound()

void SlipServer::OnNextRound(wxCommandEvent&)
{
    AUTOTEST_BUSY("nextRound");
    UINT count  = m_pChoiceBoxRound->GetCount();
    if (count == 1) return; // nothing to do
    UINT as     = m_pChoiceBoxRound->GetSelection();    //ActiveSelection
    as          = (as+1) % count;
    m_pChoiceBoxRound->SetSelection(as);
    m_activeRound = 1+as;
    LogMessage("SlipServer::OnNextRound(%u)",as);
    RefreshInfo();
}   // OnNextRound()

/*virtual*/ void SlipServer::BackupData()
{
    MyLogMessage("SlipServer::BackupData()");
    if ( !m_bDataChanged ) return;
    score::SetScoreData(svGameSetData);
    m_bDataChanged = false;
}   // BackupData()

void SlipServer::OnOk()
{
    AUTOTEST_BUSY("OnOk");
    BackupData();
    cfg::FLushConfigs();    // update diskfiles
}   // OnOk()

void SlipServer::OnCancel()
{   // does not make much sense!
    // reload of resultfile will get you the same data!
    // only manual changes will be removed, but thats not what you expect, do you????
    AUTOTEST_BUSY("OnCancel");
    m_bCancelInProgress = true;     // force reload of gamedata
    RefreshInfo();
    m_linesReadInResult = 0;        // force reload of resultfile
    (void)HandleResultFile();
}   // OnCancel()

void SlipServer::DoSearch(wxString& a_theString)
{   // not used
    (void) m_theGrid->Search(a_theString);
}   // DoSearch()

void SlipServer::PrintPage()
{   // doesn't really show something..
    wxString    title       = _("slip-info of") +" " + cfg::GetDescription();
    UINT        nrOfColumns = m_theGrid->GetNumberCols();
    m_theGrid->PrintGrid(title, nrOfColumns);
}   // PrintPage()

void SlipServer::OnGenHtmlSlipData(wxCommandEvent&)
{
    AUTOTEST_BUSY("OnGenHtmlSlipData");
#define __(x) EscapeHtmlChars((x))
    BusyBox();
    if ( 0 == cfg::GetGroupData()->size() )
    {
        MyMessageBox(_("No session data yet.."));
        return;
    }
    names::InitializePairNames();   // need it for pair-names
    static const wxString weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "unknown day!"};
    auto time = wxDateTime::Now();
    auto day  = time.GetWeekDay();
    auto date = weekdays[day] + time.Format(", %Y.%m.%d");
    wxString header = FMT("/**\n"
        "   Generated by \"%s\" version %s on %s %s\n"
        "   Info for building html pages for inputting scores\n"
        "   input data for the different tables 'in' = input, 'i' = integer, 's' = string\n"
        "**/",  __PRG_NAME__,  __VERSION__, date, GetTime() );

    wxString groupNames = "array(\"\"";
//    wxChar comma = ' ';
    for ( const auto& grp : *cfg::GetGroupData() )
    {
//        groupNames += FMT("%c \"%s\"", comma, grp.groupChars);
        groupNames += FMT(", \"%s\"", grp.groupChars);
//        comma = ',';
    }
    groupNames += ")";

    MyTextFile php(cfg::GetActiveMatchPath() + "language.php",  MyTextFile::WRITE);
    php.AddLine("<?php\n" + header);
    php.AddLine(FMT("const ACTIVE_SESSION    = %u;"    , cfg::GetActiveSession()                   ));
    php.AddLine(FMT("const MAX_TABLES        = %u;"    , m_maxTable                                ));
    php.AddLine(FMT("const NR_OF_ROUNDS      = %u;"    , m_rounds                                  ));
    php.AddLine(FMT("const SERVER_MSG_ID     = %u;"    , SERVER_MSG_ID                             ));
    php.AddLine(FMT("const SERVER_PORT       = %u;"    , SERVER_PORT                               ));
    php.AddLine(FMT("const SET_SIZE          = %u;"    , cfg::GetSetSize()                         ));
    php.AddLine(FMT("const SLIP_E_BAD_CMD    = %u;"    , (UINT)SlipResult::ERROR_BAD_CMD           ));
    php.AddLine(FMT("const SLIP_E_FORMAT     = %u;"    , (UINT)SlipResult::ERROR_FORMAT            ));
    php.AddLine(FMT("const SLIP_E_NONE       = %u;"    , (UINT)SlipResult::ERROR_NONE              ));
    php.AddLine(FMT("const SLIP_E_PARAM_COUNT= %u;"    , (UINT)SlipResult::ERROR_PARAM_COUNT       ));
    php.AddLine(FMT("const SLIP_E_PARAM_OOR  = %u;"    , (UINT)SlipResult::ERROR_PARAM_OOR         ));
    php.AddLine(FMT("$ins_appIpAddress       = \"%s\";", GetMyIpv4()                               ));
    php.AddLine(FMT("$ins_apply              = \"%s\";", __(_("ok"                                 ))));
    php.AddLine(FMT("$ins_applyTip           = \"%s\";", __(_("accept input and de-select the game"))));
    php.AddLine(FMT("$ins_badInput           = \"%s\";", __(_("bad input - data..."                ))));
    php.AddLine(FMT("$ins_bidClubs           = \"%s\";", __(_("Clubs"                              ))));
    php.AddLine(FMT("$ins_bidDiamonds        = \"%s\";", __(_("Diamonds"                           ))));
    php.AddLine(FMT("$ins_bidEast            = \"%s\";", __(_("East"                               ))));
    php.AddLine(FMT("$ins_bidHearts          = \"%s\";", __(_("Hearts"                             ))));
    php.AddLine(FMT("$ins_bidNorth           = \"%s\";", __(_("North"                              ))));
    php.AddLine(FMT("$ins_bidNoTrump         = \"%s\";", __(_("NoTrump"                            ))));
    php.AddLine(FMT("$ins_bidNP              = \"%s\";", __(_("NP"                                 ))));   // not played
    php.AddLine(FMT("$ins_bidPass            = \"%s\";", __(_("Bye"                                ))));   // pass
    php.AddLine(FMT("$ins_bidSouth           = \"%s\";", __(_("South"                              ))));
    php.AddLine(FMT("$ins_bidSpades          = \"%s\";", __(_("Spades"                             ))));
    php.AddLine(FMT("$ins_bidWest            = \"%s\";", __(_("West"                               ))));
    php.AddLine(FMT("$ins_browserRefresh     = \"%s\";", __(_("probably a browser refresh activated"))));
    php.AddLine(FMT("$ins_checkAgree         = \"%s\";", __(_("check for ok"                       ))));
    php.AddLine(FMT("$ins_clear              = \"%s\";", __(_("clear"                              ))));
    php.AddLine(FMT("$ins_contract           = \"%s\";", __(_("Contract"                           ))));
    php.AddLine(FMT("$ins_declarer           = \"%s\";", __(_("Declarer"                           ))));
    php.AddLine(FMT("$ins_description        = \"%s\";", __(cfg::GetDescription()                  )));
    php.AddLine(FMT("$ins_doubled            = \"%s\";", __(_("Doubled"                            ))));
    php.AddLine(FMT("$ins_error              = \"%s\";", __(_("Error"                              ))));
    php.AddLine(FMT("$ins_errorNetworkOpen   = \"%s\";", __(_("Error opening network"              ))));
    php.AddLine(FMT("$ins_errorNetworkRespons= \"%s\";", __(_("unexpected response from server"    ))));
    php.AddLine(FMT("$ins_fullScreen         = \"%s\";", __(_("full screen"                        ))));
    php.AddLine(FMT("$ins_game               = \"%s\";", __(_("game"                               ))));
    php.AddLine(FMT("$ins_group              = \"%s\";", __(_("group")                             )));
    php.AddLine(FMT("$ins_groupColumn        = \"%s\";", __(_("group") + ':'                       )));
    php.AddLine(FMT("$ins_groupNames         = %s;  // groupnames are one based", groupNames       ));
    php.AddLine(FMT("$ins_laptopName         = \"%s\";", wxGetHostName()                           ));
    php.AddLine(FMT("$ins_logFile            = \"%s\";", GetLogFile()                              ));
    php.AddLine(FMT("$ins_match              = \"%s\";", __(cfg::GetActiveMatch()                  )));
    php.AddLine(FMT("$ins_matchReady         = \"%s\";", __(_("Match ready!"                       ))));
    php.AddLine(FMT("$ins_noDirectStart      = \"%s\";", __(_("cannot be started directly!"        ))));
    php.AddLine(FMT("$ins_notComplete        = \"%s\";", __(_("Not all games have data!"           ))));
    php.AddLine(FMT("$ins_result             = \"%s\";", __(_("Result"                             ))));
    php.AddLine(FMT("$ins_resultNs           = \"%s\";", __(_("NS result"                          ))));
    php.AddLine(FMT("$ins_round              = \"%s\";", __(_("round"                              ))));
    php.AddLine(FMT("$ins_roundColumn        = \"%s\";", __(_("round") + ':'                       )));
    php.AddLine(FMT("$ins_scoreEntry         = \"%s\";", __(_("Score entry for"                    ))));
    php.AddLine(FMT("$ins_selectGroupTable   = \"%s\";", __(_("Select your group and table:"       ))));
    php.AddLine(FMT("$ins_sessionIdFRound    = \"FROUND\";" ));
    php.AddLine(FMT("$ins_sessionIdGroup     = \"GROUP\";"  ));
    php.AddLine(FMT("$ins_sessionIdRound     = \"ROUND\";"  ));
    php.AddLine(FMT("$ins_sessionIdTable     = \"TABLE\";"  ));
    php.AddLine(FMT("$ins_setSessionData     = \"%s\";", __(_("setting session data for"           ))));
    php.AddLine(FMT("$ins_slipCaption        = \"%s\";", __(_("Select a game to add data"          ))));
    php.AddLine(FMT("$ins_slipResultsFile    = \"%s\";", GetSlipResultsFile()                      ));
    php.AddLine(FMT("$ins_submit             = \"%s\";", __(_("submit"                             ))));
    php.AddLine(FMT("$ins_submitLogin        = \"%s\";", __(_("Start..."                           ))));
    php.AddLine(FMT("$ins_suit               = \"%s\";", __(_("Suit"                               ))));
    php.AddLine(FMT("$ins_table              = \"%s\";", __(_("table"                              ))));
    php.AddLine(FMT("$ins_tableColumn        = \"%s\";", __(_("table") + ':'                       )));
    php.AddLine(FMT("$ins_tableErrorInfo     = \"%s\";", __(_("Incorrect input, enter again"       ))));
    php.AddLine(FMT("$ins_title              = \"%s\";", __(_("login score-slips"                  ))));
    php.AddLine(FMT("$ins_tricks             = \"%s\";", __(_("Tricks"                             ))));
    php.AddLine(FMT("$ins_unexpectedInput    = \"%s\";", __(_("unexpected input data"              ))));

    CreateHtmlTableInfo(php);
    php.AddLine("?>");
#undef __
    Add2Log(_("language.php generated"), true);
}   // OnGenHtmlSlipData()

void SlipServer::CreateHtmlTableInfo(MyTextFile& a_file)
{
    a_file.AddLine("/**");
    a_file.AddLine("   the 'G*R*T*'are table-data for Group[x][Round[y]Table[z]");
    a_file.AddLine("   the arrays are 1 based: so table-data for g<x>, r<y>, t<z> is at $slipData[x][y][z]");
    a_file.AddLine("   table-data: {<set>, <pairNS>, <pairEW>}");
    a_file.AddLine("   if 'set' == 0, then there is no play at this table this round");
    a_file.AddLine("**/");
    a_file.AddLine("const INDEX_SET = 0;  // index in table-data");
    a_file.AddLine("const INDEX_NS  = 1;  // index in table-data");
    a_file.AddLine("const INDEX_EW  = 2;  // index in table-data");
    auto groupData = cfg::GetGroupData();
    a_file.AddLine("$pairNames = array(\"\" // dummy pair 0: pairnrs are 1 based");
#define __(x) EscapeHtmlChars(FMT("%s %s", names::PairnrSession2SessionText((x)), names::PairnrSession2GlobalText((x))))
    UINT group = 1;
    for ( const auto& grp : *groupData )
    {
        wxString pairs=FMT("     /* group %u */ ", group++);
        for ( UINT pair = 1; pair <= grp.pairs; ++pair )
        {
            pairs += ", \"" + __(grp.groupOffset + pair) + '"';
        }
        a_file.AddLine(pairs);
    }
    a_file.AddLine("   );");
    wxString slipData = "$slipData = array(array()";
    group = 1;
    for ( const auto& grp : *groupData )
    {   // variable 'g' = groupInfo, 'gr'= grouproundInfo, 'grt' = grouproundtableInfo
        wxString g = FMT("$G%u       = array(array()", group);
        SchemaInfo schema(grp.schemaId);
        for ( UINT round = 1; round <= schema.GetNumberOfRounds(); ++round )
        {
            schema::GameInfo info;
            wxString gr = FMT("$G%uR%u     = array(array()", group, round);
            for ( UINT table = 1; table <= schema.GetNumberOfTables(); ++table )
            {
                schema.GetTableRoundInfo(table, round, info);
                if ( grp.absent == info.pairs.ns || grp.absent == info.pairs.ew )
                {
                    info.set = 0;   // noplay this round
                    info.pairs.ns = info.pairs.ew = grp.absent; // results in empty names
                }
                wxString grt = FMT("$G%uR%uT%u   = array(%u, %u, %u);", group, round, table
                    , info.set
                    , info.set == 0 ? 0u : (grp.groupOffset + info.pairs.ns)
                    , info.set == 0 ? 0u : (grp.groupOffset + info.pairs.ew)
                );

                a_file.AddLine(grt);
                gr += FMT(", $G%uR%uT%u", group, round, table);
            }
            a_file.AddLine(gr + ");");
            g += FMT(", $G%uR%u", group, round);
        }
        a_file.AddLine(g + ");");
        slipData += FMT(", $G%u", group);
        ++group;
    }
    a_file.AddLine(slipData + ");");
#undef __
}   // CreateHtmlTableInfo()

wxString SlipServer::EscapeHtmlChars(const wxString& a_str)
{
    wxString ret(a_str);
    ret.Replace( "&" , "&amp;" );
    ret.Replace( "\"", "&quot;");
    ret.Replace( "\'", "&apos;");
    ret.Replace( "<" , "&lt;"  );
    ret.Replace( ">" , "&gt;"  );
    return ret;
}   // EscapeHtmlChars();

wxString SlipServer::GetSlipResultsFile(bool a_bfilenameOnly /* = false */)
{
    wxString fileNameOnly = FMT("%s_%u.slipdata", cfg::GetActiveMatch(), cfg::GetActiveSession());
    if ( a_bfilenameOnly )
        return fileNameOnly;
    return cfg::GetActiveMatchPath() + fileNameOnly;    // full path
}   // GetSlipResultsFile()

wxString SlipServer::GetLogFile()
{
    return FMT("%s%s_%u.log", cfg::GetActiveMatchPath(), cfg::GetActiveMatch(), cfg::GetActiveSession());
}   // GetLogFile()

void SlipServer::Add2Log(const wxString& a_newMsg, bool a_bAddTime /* =false */)
{
    m_pLog->AppendText('\n');
    if ( a_bAddTime )
    {
        m_pLog->AppendText(DateYMD() + ' ' + GetTime() + ' ');
    }
    m_pLog->AppendText(a_newMsg);
}   // AddLog()

wxString SlipServer::DateYMD()
{
    auto time = wxDateTime::Now();
    auto date = time.Format("%Y.%m.%d");
    return date;
}   // DateYMD()

void SlipServer::OnInputChoice(wxCommandEvent& a_event)
{
    AUTOTEST_BUSY("OnInputChoice");
    auto id = a_event.GetSelection();
    HandleInputSelection(id);
}   // OnInputChoice()

void SlipServer::DisplayGroupsReady()
{   // show for each group if its ready
    for ( UINT group = 1; group <= m_groups; ++group )
    {
        bool bReady = true;
        for ( UINT round = 1; bReady && (round <= m_rounds); ++round )
        {
            for ( UINT table = 1; bReady && (table <= m_maxTable); ++table )
            {
                if ( m_tableInfo[round][group][table].bPresent && !m_tableInfo[round][group][table].bReady )
                {
                    bReady = false;
                }
            }
        }
        m_theGrid->SetCellBackgroundColour(m_maxTable, (int)group, bReady ? *wxGREEN : *wxRED);
    }
}   // DisplayGroupsReady();

void SlipServer::DisplayTableReady(UINT a_group, UINT a_table, TableBackground tbg)
{
    int row          = (int)a_table - 1;
    int col          = (int)a_group;
    auto color       = cfg::GetLightOrDark(*wxWHITE);
    wxString celText = "";

    switch ( tbg )
    {
        case TableBackground::NotPresent:
            celText = "-";
            break;
        case TableBackground::NotReady:
            color = *wxRED;
            break;
        case TableBackground::Ready:
            color = *wxGREEN;
            break;
        case TableBackground::LoggedIn:
            color = *wxYELLOW;
            break;
    }

    m_theGrid->SetCellValue(row, col, celText);
    m_theGrid->SetCellBackgroundColour(row, col, color);
    Refresh();  // needed, else it will only be visible after other changes/resize
}   // DisplayTableReady()

void SlipServer::OnFileSystemEvent(wxFileSystemWatcherEvent& a_event)
{
    int type = a_event.GetChangeType();
    if ( type == wxFSW_EVENT_MODIFY )
    {
        wxString change = a_event.GetPath().GetFullName();
        wxString slip   = GetSlipResultsFile(true);
        if ( change == slip )
        {
            //Add2Log(FMT("%s: %s (%u)", _("changed"), slip, (UINT)wxFile(GetSlipResultsFile()).Length()), true);
            (void)HandleResultFile();
        }
    }
}   // OnFileSystemEvent()

void SlipServer::CreateFileWatcher(bool a_bCreate)
{
    if ( a_bCreate )
    {
        if ( nullptr == m_pFsWatcher )
        {   // only ceate if non exists
            m_pFsWatcher = new wxFileSystemWatcher();
            m_pFsWatcher->SetOwner(this);
            Bind(wxEVT_FSWATCHER, &SlipServer::OnFileSystemEvent, this);
            wxFileName f1(GetSlipResultsFile());
            wxFileName f2(wxFileName::DirName(f1.GetPath()));
            f1.DontFollowLink();
            m_pFsWatcher->Add(f2);
            Add2Log(_("Watching resultfile") + ' ' + GetSlipResultsFile() , true);
        }
    }
    else
    {
        if ( m_pFsWatcher )
        {
            Unbind(wxEVT_FSWATCHER, &SlipServer::OnFileSystemEvent, this);
            m_pFsWatcher->RemoveAll();
            delete m_pFsWatcher;
            m_pFsWatcher = nullptr;
            m_linesReadInResult = 0;
        }
    }
}   // CreateFileWatcher()

void SlipServer::HandleInputSelection(int a_selection)
{
    if ( a_selection == (int)InputChoice::InputFile )
    {   // file-input
        CreateNetworkWatcher(false);
        CreateFileWatcher(true);
        (void)HandleResultFile();           // handle file, if it exists
    }
    else
    {   // network input;
        CreateFileWatcher(false);
        CreateNetworkWatcher(true);
    }
}   // HandleInputSelection();

bool SlipServer::HasPlayed(const schema::NS_EW& a_pairs, UINT a_set, UINT a_groupOffset)
{   // check if all games in this set have a score for these pairs
    if ( a_set == 0 ) return true;          // should not happen
    UINT setSize        = cfg::GetSetSize();
    UINT nrOfGames      = cfg::GetNrOfGames();
    UINT firstGame      = (a_set - 1) * setSize + 1;
    UINT okCount        = 0;                // for ok, we need setSize* ok
    schema::NS_EW pairs = a_pairs;
    pairs.ns           += a_groupOffset;    // schema pair is relative to group-offset
    pairs.ew           += a_groupOffset;

    if ( firstGame + setSize > nrOfGames + 1 ) return false;    // sanity check
    UINT maxGame = setSize + firstGame - 1;
    for ( UINT game = firstGame; game <= maxGame; ++game )
    {
        const std::vector<score::GameSetData>& gameInfo = svGameSetData[game];
        for ( const auto& info : gameInfo )
        {   // check also a switched ns<-->ew game
            if (    (info.pairNS == pairs.ns || info.pairNS == pairs.ew )
                 && (info.pairEW == pairs.ns || info.pairEW == pairs.ew )
               )
            { ++okCount; break; }   // don't search further: game only played once per pair
        }
    }
    return (okCount == setSize);
}   // HasPlayed()

SlipServer::SlipResult SlipServer::HandleResultLine(const wxString& a_result)
{
    #define BAD(error) {Add2Log(_("^-- bad input data"), true); return (error);}
    // common part of all lines: "2025.10.09 17:24:49 2.4" -> date time id (may be '?.?')
    SlipResult error = SlipResult::ERROR_NONE;
    const auto MAX1(20);    // if you change the value, also change the formatstring
    const auto MAX2(300);
    char cmd  [MAX1+1]; cmd  [MAX1] = 0;
    char id   [MAX1+1]; id   [MAX1] = 0; MY_UNUSED(id);
    char inBuf[MAX2+1]; inBuf[MAX2] = 0;
    strncpy(inBuf, a_result.mb_str(wxConvUTF8), MAX2);
    char comment;
    if ( 1 == sscanf(inBuf, " %c", &comment) && comment == ';' )
        return SlipResult::ERROR_NONE;   // comment line

    char* pRest     = inBuf;
    int   charsRead = 0;
    auto  count     = sscanf(pRest, " %*s %*s %20s %20s %n",/* &date, &time,*/ &id, &cmd, &charsRead);
    if ( count != 2 ) BAD(SlipResult::ERROR_FORMAT);
    pRest          += charsRead;
    if ( 0 == strcmp("login", cmd) )
    {
        UINT table=0, group=0;
        count = sscanf(pRest, "for group: %u, table: %u, %n", &group, &table, &charsRead);
        if (   count != 2
            || group == 0 || group > m_groups
            || table == 0 || table > m_tables[group]
           )
            BAD(SlipResult::ERROR_PARAM_OOR);
        DisplayTableReady(group, table, TableBackground::LoggedIn);
    }
    else if ( 0 == strcmp("session:", cmd) )
    {   // "session: 3, group: 2, table: 4, round: 2, ns: 13, ew: 12, slipresult: {17, 0, 1, 1, 0, 0, 0}@....."
        GameInputData data;
        count = sscanf(pRest, " %u, group: %u, table: %u, round: %u, ns: %u, ew: %u, slipresult: %n"
            , &data.session, &data.group, &data.table, &data.round, &data.ns, &data.ew, &charsRead);
        if ( count != 6 )
            BAD(SlipResult::ERROR_PARAM_COUNT);
        if (                      data.session != cfg::GetActiveSession()
            || data.group == 0 || data.group    > m_groups
            || data.table == 0 || data.table    > m_tables[data.group]
            || data.round == 0 || data.round    > m_rounds
            || !OkPairs(data)
           )
            BAD(SlipResult::ERROR_PARAM_OOR);
        pRest += charsRead;
        for (;;++pRest) // <setsize> nr of results should follow
        {   // {<gamenr>, <declarer>, <level>, <suit>, <over/under tricks>, <doubled>, <NSscore>}@....
            count = sscanf(pRest, "{%u, %u, %u, %u, %i, %u, %i}%n",
                        &data.game, &data.declarer, &data.level, &data.suit, &data.tricks,&data.doubled, &data.nsScore, &charsRead);
            if ( count != 7 )
            {
                Add2Log(_("^-- bad input data") + ": " + pRest, true);
                error = SlipResult::ERROR_PARAM_COUNT;
            }
            else if ( !OkGameData(data) )
            {
                Add2Log(_("^-- bad input data") + ": " + pRest, true);
                error = SlipResult::ERROR_PARAM_OOR;
            }
            else
            {
                score::GameSetData gameData;
                gameData.pairNS     = data.ns;
                gameData.pairEW     = data.ew;
                gameData.scoreNS    = data.nsScore;
                gameData.scoreEW    = -data.nsScore;
                gameData.contractNS = ContractAsString(data, true);
                gameData.contractEW = ContractAsString(data, false);
                auto& gameResults   = svGameSetData[data.game];
                bool bFound         = false;    // not found yet in current data
                for ( auto& result : gameResults )
                {   // check for matching pairs and switched NS-EW
                    if (    (result.pairNS == data.ns && result.pairEW == data.ew)
                         || (result.pairNS == data.ew && result.pairEW == data.ns)
                       )
                    {   // matching pairs
                        // do NOT insert data, director COULD have changed the data!
                        bFound = true;
                        break;
                    }
                }
                if ( !bFound )  // append NEW data
                {
                    gameResults.push_back(gameData);
                    m_bDataChanged = true;
                }
            }
            pRest += charsRead;
            if ( *pRest != '@' )    // more results may follow
                break;
        }   // end result evaluation
        UpdateTableInfo(data.round, true);
    }   // end session data
    else if (    (';' != cmd[0])                        // comment
              && (0   != strcmp("ready"     , cmd))     // table finished
            )
       BAD(SlipResult::ERROR_BAD_CMD);
    return error;
#undef BAD
}   // HandleResultLine()

bool SlipServer::HandleResultFile()
{
    MyTextFile results(GetSlipResultsFile());
    if ( !results.IsOk() )
    {
        Add2Log(_("error opening results file"), true);
        return false;
    }
    auto lineCount = results.GetLineCount();
    if ( lineCount < m_linesReadInResult )
    {
        m_linesReadInResult = 0;    // file deleted, start reading from begin
    }
    bool bResult = true;
    while ( m_linesReadInResult < lineCount )
    {
        wxString line = results.GetLine(m_linesReadInResult++);
        Add2Log(FMT("line %u: %s", (UINT)m_linesReadInResult, line));
        if ( SlipResult::ERROR_NONE != HandleResultLine(line) )
            bResult = false;
    }
    return bResult;
}   // HandleResultFile()

bool SlipServer::OkPairs(const GameInputData& a_data)
{   // group/table/round are ok
    const auto& group = (*cfg::GetGroupData())[a_data.group-1];
    auto offset       = group.groupOffset;
    auto maxPair      = offset + group.pairs;
    return (     offset  <  a_data.ns && offset  <  a_data.ew
              && maxPair >= a_data.ns && maxPair >= a_data.ew
           );
}   // OkPairs()

bool SlipServer::OkGameData(const GameInputData& a_data)
{   // group/table/round/pairs are ok
    const auto& group = (*cfg::GetGroupData())[a_data.group-1];
    SchemaInfo schema(group.schemaId);
    UINT set        = schema.GetSet(a_data.table, a_data.round);
    UINT setSize    = cfg::GetSetSize();
    int totalTricks = 6 + (int)a_data.level + a_data.tricks;

    if (    ( ((a_data.game - 1) / setSize) != (set - 1) )
         || ( a_data.declarer > (UINT)Declarer::dclMax ) /* ||( a_data.declarer < Declarer::dclMin )*/
         || ( a_data.doubled  > (UINT)Doubled ::dblMax )
         || ( a_data.level    < 1                 || a_data.level > 7           )
         || ( a_data.suit     < (UINT)Suits::sMin || a_data.suit  > (UINT)Suits::sMax )
         || ( totalTricks     < 0                 || totalTricks  > 13          )
         || ( score::ScoreValidation::ScoreInvalid == score::IsScoreValid(a_data.nsScore, a_data.game, true) )
       )
        return false;
    return true;
}   // OkGameData

wxString SlipServer::ContractAsString(const GameInputData& a_data, bool a_bNs)
{
    switch ( static_cast<Declarer>(a_data.declarer) )
    {
        case Declarer::Nodata:
        case Declarer::NP:
            return ES;
            break;
        case Declarer::Pass:
            return a_bNs ? _("Bye") : ES;
            break;
        case Declarer::North:
        case Declarer::South:
            if ( !a_bNs )
                return ES;
            break;
        case Declarer::East:
        case Declarer::West:
            if ( a_bNs )
                return ES;
            break;
    }

    static const wxString suits  [] = {"", _("Clubs"), _("Diamonds"), _("Hearts"), _("Spades"), _("NoTrump")};
    static const wxString doubled[] = {"", "*", "**"};
    wxString contract(FMT("%u%s%+i%s", a_data.level, suits[a_data.suit], a_data.tricks, doubled[a_data.doubled]));
    return contract;
}   // ContractAsString()

void SlipServer::OnClearLog(wxCommandEvent&)
{
    if ( m_pLog->SaveFile(m_tempLogFile) )          // temp file deleted at program-exit
    {
        wxFileInputStream   in (m_tempLogFile);
        wxFFileOutputStream out(m_logFile, "ab+");  // file deleted at program-start

        if ( in.IsOk() && out.IsOk() )
            out.Write(in);
    }
    m_pLog->Clear();    // always clear the logwindow
}   // OnClearLog()

wxString SlipServer::GetMyIpv4()
{
    /*
    ping -a -4 -n 1 127.0.0.1      | findstr /C:[ --> "Pinging laptop-BTO17 [127.0.0.1] with 32 bytes of data:"
    ping -a -4 -n 1 laptop-bto17   | findstr /C:[ --> "Pinging laptop-BTO17 [192.168.2.46] with 32 bytes of data:"
    ping -a -4 -n 1 %COMPUTERNAME% | findstr /C:[ --> "Pinging laptop-BTO17 [192.168.2.46] with 32 bytes of data:"
    */
    wxString      hostName  = wxGetHostName();
    wxString      hostIp    = hostName; // return hostname if getting ip fails
    wxString      cmd       = FMT("ping -a -4 -n 1 \"%s\"", hostName);
    wxArrayString output;
    wxArrayString errors;
    if ( 0 == wxExecute(cmd, output, errors, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE) )
    {   // scan for line "pinging ....
        hostName.MakeLower();   // at least in MSW, the hostname COULD be case-different in the ping result!
        for ( const auto& out : output )
        {
            if ( wxString::npos != out.Lower().find(hostName) )
            {   // line with hostname also has ip-address
                auto start = out.find('[');
                auto end   = out.find(']');
                if ( end > start )  // found something!
                {
                    hostIp = out.Mid(start + 1, end - start - 1);
                }
                break;  // got only one chance to find ip...
            }
        }
    }
    return hostIp;
}   // GetMyIpv4()
