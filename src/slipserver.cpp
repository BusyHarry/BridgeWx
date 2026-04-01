// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <ranges>
#define StartFrom1(x) std::views::drop((x),1) /* view-range from 1-->end */

#include <wx/button.h>
#include <wx/fswatcher.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/socket.h>
#include <wx/textctrl.h>
#include <wx/wfstream.h>
#include <wx/wxcrtvararg.h>
#include <map>

#include "cfg.h"
#include "main.h"
#include "names.h"
#include "printer.h"
#include "score.h"
#include "fileio.h"
#include "slipserver.h"

static constexpr auto CHOICE_ROUND  ("ChoiceRound");
static constexpr auto S1            ((size_t)1);
static constexpr auto BORDERSIZE    (5);
static constexpr bool ADD_TIME      (true);

/*******
- the file with results of html-scoreslip entry *.slipdata
- file has lines like:
. <date> <time> <id> ";slipresult format: {<gamenr>, <declarer>, <level>, <suit>, <over/under tricks>, <doubled>, <NSscore>}
. <date> <time> <id> "login for group: <group>, table: <table>, groups: <groups>, fRound: <fRound>, rounds: <rounds>, games: <games>
. <date> <time> <id> "session: <session>, group: <group>, table: <table>, round: <round>, ns: <nsId>, ew: <ewId>, slipResult: <slipresult>
. <date> <time> <id> "ready session: <session>, group: <group>, table: <table>, round: <round>
where:
- <date>       -> 2025.09.21cfg::GetActiveSession()
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

static wxString EscapeHtmlChars (const wxString& str);  // escape special chars in html

SlipServer::SlipServer(wxWindow* a_pParent, UINT a_pageId) : Baseframe(a_pParent, a_pageId)
{
    // cppcheck-suppress useInitializationList
    m_firstActiveMatchPath  = cfg::GetActiveMatchPath();
    m_firstActiveMatch      = cfg::GetActiveMatch();
    m_firstActiveSession    = cfg::GetActiveSession();
    m_firstDescription      = cfg::GetDescription();
    m_logFile               = FMT("%s%s_%u.rx.log" , m_firstActiveMatchPath, m_firstActiveMatch, m_firstActiveSession);
    m_tempLogFile           = FMT("%s%s_%u.tmp.log", m_firstActiveMatchPath, m_firstActiveMatch, m_firstActiveSession);

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

    auto addMatch = new wxButton(this, wxID_ANY, _("add match"));
    addMatch->SetToolTip(_("Add another match by selecting one\nand return here to regenerate the slip-data"));
    addMatch->Bind(wxEVT_BUTTON, &SlipServer::OnAddMatch, this);

    auto pButtonHtmlSlips = new wxButton(this, wxID_ANY, _("html slip data"));
    pButtonHtmlSlips->SetToolTip(_("generate configuration data for score entry through a html-page"));
    pButtonHtmlSlips->Bind(wxEVT_BUTTON,&SlipServer::OnGenHtmlSlipData, this);

    auto* hBox = new wxBoxSizer(wxHORIZONTAL);
    hBox->Add  (m_pInputChoice   , wxSizerFlags(0).Border(wxALL, BORDERSIZE));
    hBox->MyAdd(m_pChoiceBoxRound, wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());
    hBox->Add  (nextRound        , wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());
    hBox->Add  (clearLog         , wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());
    hBox->Add  (addMatch         , wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());
    hBox->AddStretchSpacer(1000);   // 'generate slip-button' in the middle
    hBox->Add  (pButtonHtmlSlips , wxSizerFlags(1).Border(wxALL, BORDERSIZE).Bottom());
    hBox->AddStretchSpacer(1000);
    hBox->Add (okCancel          , wxSizerFlags(0).Border(wxALL, BORDERSIZE).Bottom());

    m_pLog = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL);
    wxString msg = FMT("%s --> %s", wxGetHostName(), GetMyIpv4() );
    Add2Log(msg, ADD_TIME); // show hostname and host-ip in log and statusbar
    SetStatusbarText(msg);
    // add to layout
    auto* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Slip server, automatic handling of slip-data"));
    vBox->Add(m_theGrid, wxSizerFlags(0).Expand().Border(wxALL, BORDERSIZE));
    vBox->Add(m_pLog   , wxSizerFlags(1).Expand().Border(wxALL, BORDERSIZE));
    vBox->Add(hBox     , 0);   //no borders/align: already done in hBox!
    SetSizer(vBox);     // add to panel

    RefreshInfo();                                          // now fill the grid with data
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

struct MatchInfo
{   // info administrated for each added match
    names::PairInfoData pairInfo;                       // original data of this match
    cfg  ::vGroupData   groupData;                      //  --^
    std  ::vector<UINT> session2Global;                 //  --^
    vvScoreData         gameSetData;                    //  --^
    wxString            description;                    //  --^
    UINT                activeSession       = 0;        //  --^
    UINT                setSize             = 0;        //  --^
    UINT                rounds              = 0;        //  --^
    bool                bCurrentMatch       = false;    // this match is the active match
    UINT                sessionPairOffset   = 0;        // pairnrs start here for this match
    bool                bDataChanged        = false;    // scores changed for this match
    wxString            fullName;                       // full db name, needed for updating the changed scores
};

#if 0
struct CompareNocase
{ 
    bool operator() (const wxString& left, const wxString& right) const
    {
        return left.Lower() < right.Lower();
    }
};
static std::map<wxString, MatchInfo, CompareNocase> matchesInfo1;
static std::map<wxString, MatchInfo, decltype([](const wxString& left, const wxString& right) { return left.Lower() < right.Lower(); })> matchesInfo2;
#endif

struct AllGroupsData
{
    cfg::GROUP_DATA data;
    wxString        prefix;
    wxString        description;
    UINT            setSize       = 0U;
    UINT            activeSession = 0U;
};

struct SessionPairInfo
{   // info per pair for easier getting at original match values
    // all group based vectors start at index 1: first 'group' == 1, etc
    UINT        matchOffset;    // pairnrs for this 'match' start here for this pair
    UINT        groupOffset;    // offset for the 'group' of this pair in this 'match'
    UINT        sumOffset;      // 'matchOffset' + 'groupOffset' for this pair: sessionPairnr = 'sumOffset' + 'pairnr_in_this_group'
    UINT        allGroupsIndex; // index in 's_all.groups' for the group this pair belongs to
    bool        bAbsent;        // true if pair is absent
    wxString    matchName;      // 'match' this pair belongs to, index in 's_all.matches'
    UINT        matchId;        // number of this match from 1 to N, used as prefix for groupnames/pairnames
};

struct All
{   // we use static data, else we need too many includes in the .h file
    // combined info, asif we have 1 match
    // all the vectors are 1 based, so entry 0 is a dummy
    // fe info of the first item: groups[1:group], sessionPairInfo[1:pair], globalPairInfo[1:pair], scores[1:game], session2Global[1:pair], setSizes[1:group]
    std  ::map<wxString,MatchInfo>  matches;            // map of all matches
    std  ::vector<AllGroupsData>    groups;             // combined group-info of all matches
    std  ::vector<SessionPairInfo>  sessionPairInfo;    // combined info of pairs, based on session-pairnr
    std  ::vector<names::PairInfo>  globalPairInfo;     // combined global info of pairs, based on global-pairnr
    vvScoreData                     scores;             // combined scores, indexed by game
    UINT_VECTOR                     session2Global;     // translation of a session-pairnr to global-pairnr
};
static All s_all;

static wxString PairnrSession2SessionText(UINT a_sessionPair)
{
    if ( 0 == a_sessionPair ) return names::GetNotSet();

    const auto& spi = s_all.sessionPairInfo[a_sessionPair];
    wxString groupId = s_all.matches.size() > 1
        ? s_all.groups[spi.allGroupsIndex].prefix    // add match id when more then one match
        : wxString("");
    wxString result = spi.bAbsent
                   ? names::GetNotSet()
                   : FMT("%s%s%u", groupId, s_all.groups[spi.allGroupsIndex].data.groupChars.c_str(), a_sessionPair - spi.sumOffset);
    return result;
}   // PairnrSession2SessionText()

static wxString PairnrSession2GlobalText(UINT a_sessionPair)
{
    UINT globalPair = s_all.session2Global[a_sessionPair];
    return s_all.globalPairInfo[globalPair].pairName;
}   // PairnrSession2GlobalText()

struct Upd
{
    wxString    name;
    UINT        groupOffset    = 0;
    UINT        pairInfoOffset = 0;
    UINT        matchNr        = 1;
    UINT        nrOfMatchPairs = 0;
};

void AddGroupInfo(const MatchInfo& match, const Upd& upd)
{   // update the group info
    s_all.groups.insert(s_all.groups.end(), match.groupData.begin(), match.groupData.end());
    size_t count = match.groupData.size();
    for ( auto pOffset = s_all.groups.rbegin(); count; --count, ++pOffset )
        pOffset->data.groupOffset += upd.groupOffset;    // update offset to match global offset
}   // end AddGroupInfo()

static void AddSession2Global(const MatchInfo& match, const Upd& upd)
{   // update the session2Global data
    s_all.session2Global.insert(s_all.session2Global.end(), match.session2Global.begin()+1, match.session2Global.begin()+upd.nrOfMatchPairs + 1);
    auto count = upd.nrOfMatchPairs;
    for ( auto pSession = s_all.session2Global.rbegin(); count ; pSession++,count--)
        *pSession += upd.pairInfoOffset;    // update index to match global offset
}   // AddSession2Global()

static void AddScores(const MatchInfo& match, const Upd& upd)
{   // update scores
    auto currentGames = match.gameSetData.size();   // remark: entry 0 is dummy, games are 1 based, size is always max
    auto globalGames  = s_all.scores.size();
    if ( currentGames > globalGames )
        s_all.scores.resize(currentGames);          // make room for new games
    for ( UINT game = 1; game < currentGames; ++game )
    {
        const auto& gameScores = match.gameSetData[game];
        for ( const auto& score : gameScores )
        {
            auto data    = score;
            data.pairNS += upd.groupOffset;
            data.pairEW += upd.groupOffset;
            s_all.scores[game].emplace_back(data);
        }
    }
}   // AddScores()

static void AddSessionPairInfo(const MatchInfo& match, const Upd& upd)
{   // update session pair info
    SessionPairInfo spi;
    spi.matchOffset     = upd.groupOffset;
    spi.matchName       = upd.name;
    spi.matchId         = upd.matchNr;
    auto allGroupIndex  = s_all.groups.size() - match.groupData.size();
    for ( const auto& grp : match.groupData )   // 0 based
    {
        auto size = s_all.matches.size();
        s_all.groups[allGroupIndex].setSize       = match.setSize;
        s_all.groups[allGroupIndex].prefix        = size > 1 ? FMT("%u:", upd.matchNr) : ES;
        s_all.groups[allGroupIndex].description   = match.description;
        s_all.groups[allGroupIndex].activeSession = match.activeSession;
        UINT pairs = grp.pairs;
        for ( UINT pair = 1; pair <= pairs; ++pair )
        {
            spi.groupOffset     = grp.groupOffset;
            spi.sumOffset       = spi.matchOffset + spi.groupOffset;
            spi.bAbsent         = pair == grp.absent;
            spi.allGroupsIndex  = (UINT)allGroupIndex;
            s_all.sessionPairInfo.push_back(spi);
        }
        ++allGroupIndex;
    }   // end handle all groups in this match
}   // AddSessionPairInfo()

static void AddGlobalPairInfo(const MatchInfo& match)
{   // update global pairinfo
    s_all.globalPairInfo.insert(s_all.globalPairInfo.end(), match.pairInfo.begin()+1, match.pairInfo.end());
}   // AddGlobalPairInfo()

void SlipServer::SetupGrid()
{   // (re-)setup the grid for a/next match
    wxString matchName = cfg::GetActiveMatch();
    s_all.matches[matchName] = MatchInfo();                         // add empty entry
    auto& newMatch = s_all.matches[matchName];                      // and point to newMatch
    names::InitializePairNames();
    newMatch.pairInfo       =  names::GetGlobalPairInfo();          // now update the entry with the new info
    newMatch.session2Global = *names::GetPairnrSession2Global();    // remark: we need copies of data, not references...
    newMatch.gameSetData    = *score::GetScoreData();
    newMatch.groupData      =   *cfg::GetGroupData();
    newMatch.activeSession  =    cfg::GetActiveSession();
    newMatch.setSize        =    cfg::GetSetSize();
    newMatch.description    =    cfg::GetDescription();
    newMatch.rounds         = SchemaInfo(newMatch.groupData[0].schemaId).GetNumberOfRounds(); // assume all groups have equal rounds
    newMatch.fullName       =    cfg::GetActiveMatchPath() + matchName + cfg::GetDbExtension(cfg::EXT_MAX);

    s_all.sessionPairInfo.clear();  s_all.sessionPairInfo.resize(1);
    s_all.groups         .clear();  s_all.groups         .resize(1); // group infos, 1 based
    s_all.scores         .clear();                                   // size will always be set to max nr of games
    s_all.globalPairInfo          .clear();  s_all.globalPairInfo.resize(1);            // dummy entry 0, 1 based
    s_all.session2Global .clear();  s_all.session2Global.resize(1);   // dummy entry 0, 1 based
    struct Upd upd;
    for ( auto& [name, match] : s_all.matches )
    {   // (re-)build the s_all* data
        upd.name                = name;
        match.bCurrentMatch     = false;                // current match will be updated AFTER all matches are handled
        match.sessionPairOffset = upd.groupOffset;      // needed for pairnrs in gameInfo

        upd.nrOfMatchPairs = match.groupData.back().groupOffset + match.groupData.back().pairs;
        AddGroupInfo        (match, upd);
        AddSession2Global   (match, upd);
        AddScores           (match, upd);
        AddSessionPairInfo  (match, upd);
        AddGlobalPairInfo   (match     );
        upd.pairInfoOffset += match.pairInfo.size()-1;
        upd.groupOffset    += upd.nrOfMatchPairs;
        ++upd.matchNr;
    }   // end handle all matches

    newMatch.bCurrentMatch = true;  // after init all matches, set this match to be the current
    m_maxRounds = 0;
    for ( const auto& [name,match] : s_all.matches )
        m_maxRounds = std::max( m_maxRounds, match.rounds);

    m_bDataChanged      = false;
    m_bCancelInProgress = false;
    m_linesReadInResult = 0;                    // fresh start for results-file
    // deleting zero rows/columns when none exist, will give an assert??????
    if ( auto nrOfRows = m_theGrid->GetNumberRows(); nrOfRows ) m_theGrid->DeleteRows(0, nrOfRows);
    if ( auto nrOfCols = m_theGrid->GetNumberCols(); nrOfCols ) m_theGrid->DeleteCols(0, nrOfCols);

    m_groups = s_all.groups.size()-1;                   // s_all.groups: 1 based
    m_theGrid->AppendCols(1+m_groups);                  // need 1 column for each group, column 0 is used as tablenr/label
    m_theGrid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    m_theGrid->CallAfter([this]
        {   // without CallAfter(), the grid will show default collumn-labels ( 'A'  'B' etc)
            //    when collumns have been deleted (so the 2' time this is executed)
            // UseNativeColHeader() causes this...
            m_theGrid->SetColLabelValue(0, _("table"));
        });
    m_theGrid->SetRowLabelSize(0);              // we don't use row-labels: can't suppress row-selection
    auto pAttribC0 = new(wxGridCellAttr);       // SetColAttr() takes ownership!
    pAttribC0->SetAlignment(wxALIGN_CENTER_HORIZONTAL, wxALIGN_CENTER_VERTICAL);
    m_theGrid->SetColAttr(0, pAttribC0);

    const auto COLUMN_SIZE = 10 * GetCharWidth();
    for ( UINT group = 1; group <= m_groups; ++group )
    {
        m_theGrid->SetColSize(group, COLUMN_SIZE);
        wxString label =  m_groups == 1 ? wxString("1") : s_all.groups[group].prefix + s_all.groups[group].data.groupChars;
        m_theGrid->CallAfter([this, group, label]
            {m_theGrid->SetColLabelValue(group, label);});
        auto pAttrib = new(wxGridCellAttr);  // SetColAttr() takes ownership!
        pAttrib->SetAlignment(wxALIGN_CENTER_HORIZONTAL, wxALIGN_CENTER_VERTICAL);
        m_theGrid->SetColAttr(group, pAttrib);
    }
    m_maxTable = 0;
    m_tables.clear();
    m_tables.push_back(0U); // m_tables one-based, entry 0 = dummy

    for ( const auto& group : StartFrom1(s_all.groups) )
    {   // determine the maximum nr of tables of/for all groups
        SchemaInfo schema(group.data.schemaId);
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

    m_activeRound  = 1;
    m_pChoiceBoxRound->Init(m_maxRounds, m_activeRound-1); //m_activeRound is 1 based!
    m_tableInfo.clear();
    m_tableInfo.resize(m_maxRounds+S1); // init m_tableInfo[m_maxRounds+S1][m_groups+S1][m_maxTable+S1]
    for ( auto& grp : m_tableInfo )
    {
        grp.resize(m_groups+S1);
        for ( auto& tbl : grp )
            tbl.resize(m_maxTable + S1);
    }
    for ( UINT round = 1; round <= m_maxRounds; ++round )
    {   // initialise the table info for all rounds
        UpdateTableInfo(round, DO_NOT_DISPLAY);
    }
    HandleInputSelection(m_pInputChoice->GetSelection());   // start input selection
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

void SlipServer::UpdateTableInfo(UINT a_round, bool a_bUpdateDisplay /* = DO_DISPLAY */)
{
    for ( UINT group = 1; group <= m_groups; ++group )
    {
        const auto& grp = s_all.groups[group];
        const SchemaInfo schema(grp.data.schemaId);
        UINT tables     = schema.GetNumberOfTables();
        UINT maxRound   = schema.GetNumberOfRounds();
        auto setSize    = grp.setSize;
        auto maxGames   = setSize*maxRound;

        if ( a_round > maxRound )
            continue;
        for ( UINT table = 1; table <= tables; ++table )
        {
            TableBackground tbg;
            UINT set   = schema.GetSet  (table, a_round);
            auto pairs = schema.GetPairs(table, a_round);

            if ( pairs.ns == grp.data.absent || pairs.ew == grp.data.absent || pairs.ns == 0U || pairs.ew == 0U )
                set = 0;
            if ( set )
            {
                m_tableInfo[a_round][group][table].bPresent = true;
                bool bReady = HasPlayed(pairs, set, grp.data.groupOffset, setSize, maxGames);
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

void SlipServer::OnSelectRound(const wxCommandEvent&)
{
    AUTOTEST_BUSY("OnSelectRound");
    m_activeRound = 1 + m_pChoiceBoxRound->GetSelection();
    LogMessage("SlipServer::SelectRound(%u)", m_activeRound);
    RefreshInfo();
}   // OnSelectRound()

void SlipServer::OnNextRound(const wxCommandEvent&)
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
    m_bDataChanged = false;
    for ( auto& [name, match] : s_all.matches )
    {
        if ( match.bDataChanged )
        {
            match.bDataChanged = false;
            int result = EX_RESULT_OK;  // preset result for current match, as it has no returnvalue
            if ( match.bCurrentMatch )
                score::SetScoreData(match.gameSetData);
            else
                result = io::ScoresWriteEx(match.fullName, match.gameSetData, match.activeSession);
            if ( result == EX_RESULT_CURRENT ) {/*should not happen*/ }
            MyLogDebug(FMT("SlipServer::BackupData('%s':%i)", name, result));
        }
    }
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
    wxString    title       = _("slip-info of") +" " + m_firstDescription;
    UINT        nrOfColumns = m_theGrid->GetNumberCols();
    m_theGrid->PrintGrid(title, nrOfColumns);
}   // PrintPage()

void SlipServer::OnGenHtmlSlipData(const wxCommandEvent&)
{
    AUTOTEST_BUSY("OnGenHtmlSlipData");
#define __(x) EscapeHtmlChars((x))
    BusyBox();
    if ( m_groups == 0 )
    {
        MyMessageBox(_("No session data yet.."));
        return;
    }
    static const wxString weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "unknown day!"};
    auto time = wxDateTime::Now();
    auto day  = time.GetWeekDay();
    auto date = weekdays[day] + time.Format(", %Y.%m.%d");
    wxString header = FMT("/**\n"
        "   Generated by \"%s\" version %s on %s %s\n"
        "   Info for building html pages for inputting scores\n"
        "   input data for the different tables 'in' = input, 'i' = integer, 's' = string\n"
        "**/",  __PRG_NAME__,  __VERSION__, date, GetTime() );

    wxString groupNames         = "array(\"\"";
    wxString matchNames         = "array(\"\"";
    wxString matchDescriptions  = "array(\"\"";
    wxString sessions           = "array(0"   ;
    wxString setSizes           = "array(0"   ;

    for ( const auto& grp : StartFrom1(s_all.groups) )
    {
        if ( grp.setSize == 0 ) continue;
        groupNames          += FMT(", \"%s%s\"", grp.prefix, grp.data.groupChars                            );
        matchNames          += FMT(", \"%s\""  , s_all.sessionPairInfo[grp.data.groupOffset+S1].matchName   );
        matchDescriptions   += FMT(", \"%s\""  , __(grp.description)                                        );
        sessions            += FMT(", %u"      , grp.activeSession                                          );
        setSizes            += FMT(", %u"      , grp.setSize                                                );
    }
    groupNames          += ")";
    matchNames          += ")";
    matchDescriptions   += ")";
    sessions            += ")";
    setSizes            += ")";

    MyTextFile php(m_firstActiveMatchPath + "language.php",  MyTextFile::WRITE);
    php.AddLine("<?php\n" + header);
    php.AddLine(FMT("const MAX_TABLES        = %u;"    , m_maxTable                                ));
    php.AddLine(FMT("const SERVER_MSG_ID     = %u;"    , SERVER_MSG_ID                             ));
    php.AddLine(FMT("const SERVER_PORT       = %u;"    , SERVER_PORT                               ));
    php.AddLine(FMT("const SLIP_E_NONE       = %u;"    , (UINT)SlipResult::ERROR_NONE              ));
    php.AddLine(FMT("const SLIP_E_BAD_CMD    = %u;"    , (UINT)SlipResult::ERROR_BAD_CMD           ));
    php.AddLine(FMT("const SLIP_E_PARAM_COUNT= %u;"    , (UINT)SlipResult::ERROR_PARAM_COUNT       ));
    php.AddLine(FMT("const SLIP_E_PARAM_OOR  = %u;"    , (UINT)SlipResult::ERROR_PARAM_OOR         ));
    php.AddLine(FMT("const SLIP_E_FORMAT     = %u;"    , (UINT)SlipResult::ERROR_FORMAT            ));
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
    php.AddLine(FMT("$ins_doubled            = \"%s\";", __(_("Doubled"                            ))));
    php.AddLine(FMT("$ins_error              = \"%s\";", __(_("Error"                              ))));
    php.AddLine(FMT("$ins_errorNetworkOpen   = \"%s\";", __(_("Error opening network"              ))));
    php.AddLine(FMT("$ins_errorNetworkRespons= \"%s\";", __(_("unexpected response from server"    ))));
    php.AddLine(FMT("$ins_fullScreen         = \"%s\";", __(_("full screen"                        ))));
    php.AddLine(FMT("$ins_game               = \"%s\";", __(_("game"                               ))));
    php.AddLine(FMT("$ins_group              = \"%s\";", __(_("group")                             )));
    php.AddLine(FMT("$ins_groupColumn        = \"%s\";", __(_("group") + ':'                       )));
    php.AddLine(FMT("$ins_groupNames         = %s;  // groupnames[groupnr]"     , groupNames       ));
    php.AddLine(FMT("$ins_matchNames         = %s;  // matchnames[groupnr]"     , matchNames       ));
    php.AddLine(FMT("$ini_setSizes           = %s;  // setSizes[groupnr]"       , setSizes         ));
    php.AddLine(FMT("$ins_descriptions       = %s;  // descriptions[groupnr]"   , matchDescriptions));
    php.AddLine(FMT("$ini_sessions           = %s;  // sessions[groupnr]"       , sessions         ));
    php.AddLine(FMT("$ins_laptopName         = \"%s\";", wxGetHostName()                           ));
    php.AddLine(FMT("$ins_logFile            = \"%s\";", GetLogFile()                              ));
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
    Add2Log(_("language.php generated"), ADD_TIME);
}   // OnGenHtmlSlipData()

void SlipServer::CreateHtmlTableInfo(MyTextFile& a_file) const
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
    a_file.AddLine("$pairNames = array(\"\" // dummy pair 0: pairnrs are 1 based");
#define __(x) EscapeHtmlChars(FMT("%s %s", PairnrSession2SessionText((x)), PairnrSession2GlobalText((x))))
    UINT group = 1;

    for ( const auto& grp : StartFrom1(s_all.groups) )
    {
        wxString pairs=FMT("     /* group %u */ ", group++);
        for ( UINT pair = 1; pair <= grp.data.pairs; ++pair )
        {
            pairs += ", \"" + __(grp.data.groupOffset + pair) + '"';
        }
        a_file.AddLine(pairs);
    }
    a_file.AddLine("   );");
    wxString slipData = "$slipData = array(array()";
    group = 1;
    for ( const auto& grp : StartFrom1(s_all.groups) )
    {   // variable 'g' = groupInfo, 'gr'= grouproundInfo, 'grt' = grouproundtableInfo
        wxString g = FMT("$G%u       = array(array()", group);
        SchemaInfo schema(grp.data.schemaId);
        for ( UINT round = 1; round <= schema.GetNumberOfRounds(); ++round )
        {
            schema::GameInfo info;
            wxString gr = FMT("$G%uR%u     = array(array()", group, round);
            for ( UINT table = 1; table <= schema.GetNumberOfTables(); ++table )
            {
                schema.GetTableRoundInfo(table, round, info);
                if (   grp.data.absent == info.pairs.ns || grp.data.absent == info.pairs.ew
                    || 0U              == info.pairs.ns || 0U              == info.pairs.ew
                   )
                {   // pair is absent or there is just no play at this table, this round
                    info.set = 0;   // noplay this round
                }
                wxString grt = FMT("$G%uR%uT%u   = array(%u, %u, %u);", group, round, table
                    , info.set
                    , info.set == 0 ? 0u : (grp.data.groupOffset + info.pairs.ns)
                    , info.set == 0 ? 0u : (grp.data.groupOffset + info.pairs.ew)
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

static wxString EscapeHtmlChars(const wxString& a_str)
{
    wxString ret(a_str);
    ret.Replace( "&" , "&amp;" );
    ret.Replace( "\"", "&quot;");
    ret.Replace( "\'", "&apos;");
    ret.Replace( "<" , "&lt;"  );
    ret.Replace( ">" , "&gt;"  );
    return ret;
}   // EscapeHtmlChars()

wxString SlipServer::GetBaseResultName() const
{
    return s_all.matches.size() > 1
        ? "slipResults_" + DateYMD()                                // general name if more the one match involved
        : FMT("%s_%u" , m_firstActiveMatch, m_firstActiveSession);  // use matchname+session as identification
}   // GetBaseResultName()

wxString SlipServer::GetSlipResultsFile(bool a_bfilenameOnly /* = false */) const
{
    wxString fileNameOnly = GetBaseResultName() + ".slipdata";
    if ( a_bfilenameOnly )
        return fileNameOnly;
    return m_firstActiveMatchPath + fileNameOnly;    // full path
}   // GetSlipResultsFile()

wxString SlipServer::GetLogFile() const
{
    return m_firstActiveMatchPath + GetBaseResultName() + ".log";
}   // GetLogFile()

void SlipServer::Add2Log(const wxString& a_newMsg, bool a_bAddTime /* =false */)
{
    m_pLog->AppendText('\n');
    if ( a_bAddTime )
        m_pLog->AppendText(DateYMD() + ' ' + GetTime() + ' ');
    m_pLog->AppendText(a_newMsg);
}   // Add2Log()

wxString SlipServer::DateYMD() const
{
    auto time = wxDateTime::Now();
    auto date = time.Format("%Y.%m.%d");
    return date;
}   // DateYMD()

void SlipServer::OnInputChoice(const wxCommandEvent& a_event)
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
        for ( UINT round = 1; bReady && (round <= m_maxRounds); ++round )
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
}   // DisplayGroupsReady()

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

void SlipServer::OnFileSystemEvent(const wxFileSystemWatcherEvent& a_event)
{
    int type = a_event.GetChangeType();
    if ( type == wxFSW_EVENT_MODIFY )
    {
        wxString change = a_event.GetPath().GetFullName();
        wxString slip   = GetSlipResultsFile(true);
        if ( change == slip )
        {
            //Add2Log(FMT("%s: %s (%u)", _("changed"), slip, (UINT)wxFile(GetSlipResultsFile()).Length()), ADD_TIME);
            (void)HandleResultFile();
        }
    }
}   // OnFileSystemEvent()

void SlipServer::CreateFileWatcher(bool a_bCreate)
{
    if ( a_bCreate )
    {
        if ( m_pFsWatcher )
            CreateFileWatcher(false);    // ALWAYS (re-)create filewatcher, resultfilename could have changed
        m_pFsWatcher = new wxFileSystemWatcher();
        m_pFsWatcher->SetOwner(this);
        Bind(wxEVT_FSWATCHER, &SlipServer::OnFileSystemEvent, this);
        wxFileName f1(GetSlipResultsFile());
        wxFileName f2(wxFileName::DirName(f1.GetPath()));
        f1.DontFollowLink();
        m_pFsWatcher->Add(f2);
        Add2Log(_("Watching resultfile") + ' ' + GetSlipResultsFile() , ADD_TIME);
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
    {   // network input
        CreateFileWatcher(false);
        CreateNetworkWatcher(true);
    }
}   // HandleInputSelection();

bool SlipServer::HasPlayed(const schema::NS_EW& a_pairs, UINT a_set, UINT a_groupOffset, UINT a_setSize, UINT a_maxGames) const
{   // check if all games in this set have a score for these pairs
    if ( a_set == 0 ) return true;          // should not happen
    UINT firstGame      = (a_set - 1) * a_setSize + 1;
    UINT okCount        = 0;                // for ok, we need setSize* ok
    schema::NS_EW pairs = a_pairs;
    pairs.ns           += a_groupOffset;    // schema pair is relative to group-offset
    pairs.ew           += a_groupOffset;

    if ( firstGame + a_setSize > a_maxGames + 1 ) return false;    // sanity check
    UINT maxGame = a_setSize + firstGame - 1;
    for ( UINT game = firstGame; game <= maxGame; ++game )
    {
        const std::vector<score::GameSetData>& gameInfo = s_all.scores[game];
        for ( const auto& info : gameInfo )
        {   // check also a switched ns<-->ew game
            if (    (info.pairNS == pairs.ns || info.pairNS == pairs.ew )
                 && (info.pairEW == pairs.ns || info.pairEW == pairs.ew )
               )
            { ++okCount; break; }   // don't search further: game only played once per pair
        }
    }
    return (okCount == a_setSize);
}   // HasPlayed()

static wxString GetBadInputDataString()
{
    return _("^-- bad input data");
}   // GetBadInputDataString()

SlipServer::SlipResult SlipServer::HandleError(SlipServer::SlipResult a_error)
{
    wxString explanation;
    switch ( a_error )
    {
        case SlipResult::ERROR_NONE:
            explanation = _("no error");
            break;
        case SlipResult::ERROR_BAD_CMD:
            explanation = _("unknown command");
            break;
        case SlipResult::ERROR_PARAM_COUNT:
            explanation = _("invalid number of parameters");
            break;
        case SlipResult::ERROR_PARAM_OOR:
            explanation = _("param(s) out of range");
            break;
        case SlipResult::ERROR_FORMAT:
            explanation = _("wrong command format");
            break;
    }
    Add2Log(FMT("%s(%d: %s)", GetBadInputDataString(), (int)a_error, explanation), ADD_TIME);
    return a_error;
}   // HandleError()

SlipServer::SlipResult SlipServer::HandleLogin(const char*& pInput)
{
    UINT table      = 0;
    UINT group      = 0;
    auto count      = sscanf(pInput, "for group: %u, table: %u, ", &group, &table);
    if (   count != 2
        || group == 0 || group > m_groups
        || table == 0 || table > m_tables[group]
       )
        return HandleError(SlipResult::ERROR_PARAM_OOR);
    DisplayTableReady(group, table, TableBackground::LoggedIn);
    return SlipResult::ERROR_NONE;
}   // HandleLogin()

void SlipServer::HandleOneGame(const GameInputData& a_data)
{
    score::GameSetData gameData;
    gameData.pairNS     = a_data.ns;
    gameData.pairEW     = a_data.ew;
    gameData.scoreNS    = a_data.declarer == (UINT)Declarer::NP ? SCORE_NP : a_data.nsScore;
    gameData.scoreEW    = -a_data.nsScore;
    gameData.contractNS = ContractAsString(a_data, true);
    gameData.contractEW = ContractAsString(a_data, false);
    auto& gameResults   = s_all.scores[a_data.game];
    bool bFound         = false;    // not found yet in current data
    for ( const auto& result : gameResults )
    {   // check for matching pairs and switched NS-EW
        if (    (result.pairNS == a_data.ns && result.pairEW == a_data.ew)
             || (result.pairNS == a_data.ew && result.pairEW == a_data.ns)
           )
        {   // matching pairs
            // do NOT replace this result with new data, director COULD have changed the data!
            bFound = true;
            break;
        }
    }
    if ( !bFound )  // append NEW data
    {
        gameResults.push_back(gameData);
        // also save changes for backup
        gameData.pairNS -= s_all.sessionPairInfo[a_data.ns].matchOffset;    // get match-relative pairnr
        gameData.pairEW -= s_all.sessionPairInfo[a_data.ns].matchOffset;
        auto& match      = s_all.matches[s_all.sessionPairInfo[a_data.ns].matchName];
        match.gameSetData[a_data.game].push_back(gameData);
        match.bDataChanged = true;  // this match has changed
        m_bDataChanged     = true;  //  'a' match has changed
    }
}   // HandleOneGame()

SlipServer::SlipResult SlipServer::HandleSession(const char*& a_pInput)
{   // "session: 3, group: 2, table: 4, round: 2, ns: 13, ew: 12, slipresult: {17, 0, 1, 1, 0, 0, 0}@....."
    GameInputData   data;
    SlipResult      error     = SlipResult::ERROR_NONE;
    int             charsRead = 0;
    auto            count     = sscanf(a_pInput, " %u, group: %u, table: %u, round: %u, ns: %u, ew: %u, slipresult: %n"
        , &data.session, &data.group, &data.table, &data.round, &data.ns, &data.ew, &charsRead);
    if ( count != 6 )
        return HandleError(SlipResult::ERROR_PARAM_COUNT);
    if (
           data.group == 0 || data.group    > m_groups
        || data.table == 0 || data.table    > m_tables[data.group]
        || data.round == 0 || data.round    > m_maxRounds
        || !OkPairs(data)  || data.session != s_all.groups[data.group].activeSession
       )
        return HandleError(SlipResult::ERROR_PARAM_OOR);
    a_pInput += charsRead;
    for ( ;; ++a_pInput ) // <setsize> nr of results should follow
    {   // {<gamenr>, <declarer>, <level>, <suit>, <over/under tricks>, <doubled>, <NSscore>}@....
        count = sscanf(a_pInput, "{%u, %u, %u, %u, %i, %u, %i}%n",
            &data.game, &data.declarer, &data.level, &data.suit, &data.tricks, &data.doubled, &data.nsScore, &charsRead);
        if ( count != 7 )
        {
            error = SlipResult::ERROR_PARAM_COUNT;
            Add2Log(FMT("%s(%i): %s", GetBadInputDataString(), (int)error,  a_pInput), ADD_TIME);
        }
        else if ( !OkGameData(data) )
        {
            error = SlipResult::ERROR_PARAM_OOR;
            Add2Log(FMT("%s(%i): %s", GetBadInputDataString(), (int)error,  a_pInput), ADD_TIME);
        }
        else
            HandleOneGame(data);    // no obvious error in input, so get the result of this game
        a_pInput += charsRead;      // point to next gamedata
        if ( *a_pInput != '@' )     // more results may follow
            break;
    }   // end result evaluation
    if ( m_activeRound == data.round )  // update display ONLY if new data is from the active round 
        UpdateTableInfo(m_activeRound, DO_DISPLAY);
    return error;
}   // HandleSession()

SlipServer::SlipResult SlipServer::HandleResultLine(const wxString& a_result)
{
    // common part of all lines: "2025.10.09 17:24:49 2.4" -> date time id (may be '?.?')
    const auto MAX1(20);    // if you change the value, also change the formatstring
    const auto MAX2(300);
    char cmd  [MAX1+1]; cmd  [MAX1] = 0;
    char id   [MAX1+1]; id   [MAX1] = 0; MY_UNUSED(id);
    char inBuf[MAX2+1]; inBuf[MAX2] = 0;
    strncpy(inBuf, a_result.mb_str(wxConvUTF8), MAX2);
    if ( char comment; 1 == sscanf(inBuf, " %c", &comment) && comment == ';' )
        return SlipResult::ERROR_NONE;   // comment line

    const char* pRest       = inBuf;
    int         charsRead   = 0;
    auto        count       = sscanf(pRest, " %*s %*s %20s %20s %n",/* &date, &time,*/ &id, &cmd, &charsRead);
    if ( count != 2 )
        return HandleError(SlipResult::ERROR_FORMAT);

    pRest += charsRead;
    if ( 0 == strcmp("login", cmd) )
        return HandleLogin(pRest);

    if ( 0 == strcmp("session:", cmd) )
        return HandleSession(pRest);

    if (     (';' != cmd[0])                // comment
          && (0   != strcmp("ready", cmd))  // table finished
       )
       return HandleError(SlipResult::ERROR_BAD_CMD);

    return SlipResult::ERROR_NONE;
}   // HandleResultLine()

bool SlipServer::HandleResultFile()
{
    MyTextFile results(GetSlipResultsFile());
    if ( !results.IsOk() )
    {
        Add2Log(_("error opening results file"), ADD_TIME);
        return false;
    }
    auto lineCount = results.GetLineCount();
    if ( lineCount < m_linesReadInResult )
        m_linesReadInResult = 0;    // file deleted, start reading from begin
    bool bOk = true;
    while ( m_linesReadInResult < lineCount )
    {
        wxString line = results.GetLine(m_linesReadInResult++);
        Add2Log(FMT("line %u: %s", (UINT)m_linesReadInResult, line));
        if ( SlipResult::ERROR_NONE != HandleResultLine(line) )
            bOk = false;
    }
    return bOk;
}   // HandleResultFile()

bool SlipServer::OkPairs(const GameInputData& a_data) const
{   // group/table/round are ok
    const auto& group = s_all.groups[a_data.group].data;
    auto offset       = group.groupOffset;
    auto maxPair      = offset + group.pairs;
    return (     offset  <  a_data.ns && offset  <  a_data.ew
              && maxPair >= a_data.ns && maxPair >= a_data.ew
           );
}   // OkPairs()

bool SlipServer::OkGameData(const GameInputData& a_data) const
{   // group/table/round/pairs are ok
    UINT setSize      = s_all.groups[a_data.group].setSize;
    const auto& group = s_all.groups[a_data.group].data;
    SchemaInfo schema(group.schemaId);
    UINT set          = schema.GetSet(a_data.table, a_data.round);
    int totalTricks   = 6 + (int)a_data.level + a_data.tricks;

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

wxString SlipServer::ContractAsString(const GameInputData& a_data, bool a_bNs) const
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

    wxString contract = a_data.tricks
                        ? FMT("%u%s%+i%s", a_data.level, suits[a_data.suit], a_data.tricks, doubled[a_data.doubled])
                        : FMT("%u%s%s"   , a_data.level, suits[a_data.suit]               , doubled[a_data.doubled]);
        return contract;
}   // ContractAsString()

void SlipServer::OnAddMatch(const wxCommandEvent&)
{   // jump to the 'new match' page
    SendEvent2Mainframe(ID_MENU_SETUPNEWMATCH);
}   // OnAddMatch()

void SlipServer::OnClearLog(const wxCommandEvent&)
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

wxString SlipServer::GetMyIpv4() const
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
