﻿// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include "wx/radiobox.h"

#include <wx/filename.h>

#include "cfg.h"
#include "names.h"
#include "mygrid.h"
#include "printer.h"
#include "score.h"

#include "scoreEntry.h"

static std::vector< std::vector<score::GameSetData> > svGameSetData;   // [cfg::max_games+1]
static bool     GetScore        (UINT theGame, UINT nsPair, score::GameSetData& setData, bool& a_bReversed); // return requested data. true if found. if NS-EW are a_bReversed, flag is set

#define CHOICE_ID_SLIP    0ULL  /* slip/game added FIRST to this sizer*/
#define CHOICE_ID_GAME_NS 1ULL  /* game/ns added SECOND to this sizer*/
#define CHOICE_ID_ROUND   2ULL  /* choice round added THIRD to this sizer*/
#define CHOICE_ID_GAME    3ULL  /* choice round added FOURTH to this sizer*/

#define CHOICE_GAME     "ChoiceGame"
#define CHOICE_ROUND    "ChoiceRound"
ScoreEntry::ScoreEntry(wxWindow* a_pParent, UINT a_pageId) :Baseframe(a_pParent, a_pageId), m_theGrid(0)
{
    // create and populate grid
    m_theGrid = new MyGrid(this, "GridScoreEntry");
    m_theGrid->CreateGrid(0, COL_NR_OF);
//    m_theGrid->SetRowLabelSize( 4*GetCharWidth() );   // room for 3 digit numbers
    m_theGrid->HideRowLabels();                         // don't need 1 to N
    int sizeOne = GetCharWidth();
    #define SIZE_PAIRNAME   ((cfg::MAX_NAME_SIZE+1)* sizeOne)   /* original name        */
    #define SIZE_PAIRNR     (6 * sizeOne)                       /* "AB12 *"             */
    #define SIZE_ID         (5 * sizeOne)                       /* just numbers 1-120   */
    #define SIZE_SCORE      (8 * sizeOne)                       /* 'score nz' / 'R-9999'*/
    m_theGrid->SetColSize(COL_GAME      , SIZE_ID       ); m_theGrid->SetColLabelValue(COL_GAME     , _("spel"    ));
    m_theGrid->SetColSize(COL_NS        , SIZE_PAIRNR   ); m_theGrid->SetColLabelValue(COL_NS       , _("nz"      ));
    m_theGrid->SetColSize(COL_EW        , SIZE_PAIRNR   ); m_theGrid->SetColLabelValue(COL_EW       , _("ow"      ));
    m_theGrid->SetColSize(COL_SCORE_NS  , SIZE_SCORE    ); m_theGrid->SetColLabelValue(COL_SCORE_NS , _("score nz"));
    m_theGrid->SetColSize(COL_SCORE_EW  , SIZE_SCORE    ); m_theGrid->SetColLabelValue(COL_SCORE_EW , _("score ow"));
    m_theGrid->SetColSize(COL_NAME_NS   , SIZE_PAIRNAME ); m_theGrid->SetColLabelValue(COL_NAME_NS  , _("naam nz" ));
    m_theGrid->SetColSize(COL_NAME_EW   , SIZE_PAIRNAME ); m_theGrid->SetColLabelValue(COL_NAME_EW  , _("naam ow" ));

    wxGridCellAttr* pAttr = new wxGridCellAttr;
    // apparently can be used only once
    // need this IncRef() if using it for 2 colums? strange?? using it in SetColAttr() invalidates it?????
    // other solution: use it only once, then create a new wxGridCellAttr ....
    pAttr->IncRef();
    pAttr->SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTER_VERTICAL);
    m_theGrid->SetColAttr(COL_SCORE_NS, pAttr);
    m_theGrid->SetColAttr(COL_SCORE_EW, pAttr);      //<------ without IncRef() something wrong at exit

//    std::vector<MyGrid::SortMethod> methods;
//    methods.push_back(MyGrid::SORT_STRING);
//    methods.push_back(MyGrid::SORT_SESSIONNAME);
//    methods.push_back(MyGrid::SORT_SESSIONNAME);
//    methods.push_back(MyGrid::SORT_INTNUMBER);
//    methods.push_back(MyGrid::SORT_INTNUMBER);
//    m_theGrid->SetSortMethod(methods);          // set sort hints,  without plain string compare is done

    m_pSizerAllChoices = new wxBoxSizer(wxHORIZONTAL);
    wxArrayString choices;
    choices.push_back(_("Score slip"));            // score entry method
    choices.push_back(_("Spel nummer "));          // NB: extra space to have different string for autotest, see 4 lines down
    m_pRadioBoxSlipGame = CreateRadioBox(_("Ingave volgorde:"), choices, EVT_CMD_HANDLER( &ScoreEntry::OnRbSlipGame),"InputOrder");
    m_pSizerAllChoices->Add(m_pRadioBoxSlipGame, 0, wxBOTH | wxALL, MY_BORDERSIZE); // CHOICE_ID_SLIP slip/game added FIRST to this sizer
    choices.clear();
    choices.push_back(_("Spel nummer"));           // if slip, then entry order based on game-nr or pair-nr
    choices.push_back(Unique(_("NZ nummer")));
    m_pRadioBoxGameNs = CreateRadioBox(_("Score slip volgorde:"), choices, EVT_CMD_HANDLER(&ScoreEntry::OnRbGameNS),"SlipOrder");
    m_pSizerAllChoices->Add(m_pRadioBoxGameNs, 0, wxBOTH | wxALL, MY_BORDERSIZE); // CHOICE_ID_GAME_NS game/ns added SECOND to this sizer

    m_pChoiceRound = new MY_CHOICE(this, _("Ronde:"), _("Scores voor deze ronde"),Unique(CHOICE_ROUND));
    m_pChoiceRound->Bind(wxEVT_CHOICE, &ScoreEntry::OnSelectRound, this);
    m_pSizerAllChoices->MyAdd(m_pChoiceRound, 0, wxBOTH | wxALL | wxALIGN_CENTER_VERTICAL, MY_BORDERSIZE);

    m_pChoiceGame = new MyChoiceMC(this, _("Spel:") , _("Scores voor dit spel"), Unique(CHOICE_GAME));
    m_pChoiceGame->Bind(wxEVT_CHOICE, &ScoreEntry::OnSelectGame, this);
    m_pSizerAllChoices->MyAdd(m_pChoiceGame, 0, wxBOTH | wxALL | wxALIGN_CENTER_VERTICAL, MY_BORDERSIZE);

    // CHOICE_ID_ROUND  choice round added THIRD to this sizer
    // CHOICE_ID_GAME   choice round added FOURTH to this sizer

    auto pButtonNextEmptyScore = new wxButton(this, wxID_ANY , _("++lege score") );
    pButtonNextEmptyScore->Bind(wxEVT_BUTTON,&ScoreEntry::OnNextEmptyScore, this);
    pButtonNextEmptyScore->SetToolTip(_("gaat naar volgende lege score"));

    auto pButtonSwitchNsEw = new wxButton(this, wxID_ANY , _("nz<-->ow") );
    pButtonSwitchNsEw->Bind(wxEVT_BUTTON,&ScoreEntry::OnSwitchNsEw, this);
    pButtonSwitchNsEw->SetToolTip(_("wissel NZ paar met OW paar: draaitafel"));

    auto search   = CreateSearchBox();
    auto okCancel = CreateOkCancelButtons();
    wxBoxSizer* vBoxOk = new wxBoxSizer(wxVERTICAL);
    vBoxOk->Add(okCancel, 0, wxALIGN_RIGHT);

    wxBoxSizer* hBoxSearchOk = new wxBoxSizer(wxHORIZONTAL);

    hBoxSearchOk->Add(search                , 1, wxBOTH | wxALL, MY_BORDERSIZE);
    hBoxSearchOk->Add(pButtonNextEmptyScore , 0, wxBOTH | wxALL, MY_BORDERSIZE);
    hBoxSearchOk->Add(pButtonSwitchNsEw     , 0, wxBOTH | wxALL, MY_BORDERSIZE);    hBoxSearchOk->AddStretchSpacer(1000);
    hBoxSearchOk->Add(vBoxOk                , 0, wxBOTH | wxALL, MY_BORDERSIZE);

    // add to layout
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Ingave/Wijzigen scores"));
    vBox->Add(m_theGrid         , 1, wxEXPAND | wxALL, MY_BORDERSIZE);
    vBox->Add(m_pSizerAllChoices     , 0);
    vBox->Add(hBoxSearchOk      , 0);
    SetSizer(vBox);     // add to panel

    m_bCancelInProgress     = false;
    m_bSlipOrder            = true;
    m_bGameOrder            = true;
    m_bDataChanged          = false;
    m_startrow4EmptyScore   = -1;
    m_uActiveRound          = 1;
    m_uActiveGame           = cfg::GetFirstGame();
    svGameSetData.resize(cfg::MAX_GAMES+1); //expected to be present: no checks are done...

    RefreshInfo();                  // fill the grid with data
    Bind(wxEVT_GRID_CELL_LEFT_CLICK, &ScoreEntry::OnSelectCell, this, wxID_ANY);    // AFTER refreshinfo

    AUTOTEST_ADD_WINDOW(pButtonNextEmptyScore, "NextEmpty"  );  // mousepositions
    AUTOTEST_ADD_WINDOW(pButtonSwitchNsEw    , "SwitchNsEw" );
    AUTOTEST_ADD_WINDOW(m_pChoiceRound       , CHOICE_ROUND );
    AUTOTEST_ADD_WINDOW(m_pChoiceGame        , CHOICE_GAME  );
    m_description = "ScoreEntry";
}   // ScoreEntry()

ScoreEntry::~ScoreEntry(){}

/*virtual*/ void ScoreEntry::BackupData()
{
    //LogMessage("ScoreEntry::BackupData()");
    WriteScoresToDisk();
}   // BackupData()

bool ScoreEntry::OnCellChanging(const CellInfo& a_cellInfo)
{
    AUTOTEST_BUSY("cellChanging");
    if (a_cellInfo.pList != m_theGrid)
    {
        return CELL_CHANGE_OK;  //hm, its not my grid
    }

    if ( (a_cellInfo.column != COL_SCORE_NS ) && (a_cellInfo.column != COL_SCORE_EW) )
        return CELL_CHANGE_REJECTED;    // should not happen...

    if (m_bIsScriptTesting)
    {
        wxString msg;
        msg.Printf(_("ScoreEntry:: rij %2d, colom %d verandert van <%s> naar <%s>")
            , a_cellInfo.row
            , a_cellInfo.column
            , a_cellInfo.oldData.c_str()
            , a_cellInfo.newData.c_str()
        );
        LogMessage("%s", msg); // there COULD be a "%" sign in the message.....
    }

    int         row         = a_cellInfo.row;
    int         col         = a_cellInfo.column;
    bool        bNS         = (col == COL_SCORE_NS);
    wxString    newData     = a_cellInfo.newData;
    UINT        game        = wxAtoi(m_theGrid->GetCellValue(row, COL_GAME));
    int         score       = score::ScoreFromString(newData);
    m_startrow4EmptyScore   = row;

    if (score == SCORE_NONE)
    {   // empty string
        if (bNS)    // remove EW score, if present...
            m_theGrid->SetCellValue(row, COL_SCORE_EW, ES);
    }
    else 
    {   // we have a non-empty score, now test if its a possible one
        if ((score != SCORE_NP) && !score::IsScoreValid(score, game, bNS))
        {
            wxColour org = m_theGrid->GetCellBackgroundColour(row, col);
            m_theGrid->SetCellBackgroundColour(row, col, *wxRED );

            // grid shows old data during msgbox, so put new data in...
            m_theGrid->SetCellValue(row, col, newData);
            m_theGrid->Refresh();
            bool bNo = (wxNO == wxMessageBox(_("ongeldige score, toch opslaan?"), "???", wxYES_NO | wxICON_INFORMATION));
            m_theGrid->SetCellBackgroundColour(row, col, org );
            m_theGrid->Refresh();
            if (bNo)
            {
                // 'new' data stays in grid, so putback old again.....
                m_theGrid->SetCellValue(row, col, a_cellInfo.oldData);
                // still need to 'eat' the enter now. Without, cursor gows one down...
                m_theGrid->CallAfter([this,a_cellInfo](){this->m_theGrid->GoToCell(a_cellInfo.row, a_cellInfo.column);});
                return CELL_CHANGE_REJECTED;
            }
        }

        // now we have valid score or accepted a for me unknown score...
        // check if NS and arbitrary --> goto EW for arbitrary score
        if (score::IsReal(score))
        {
            if (!bNS)
            {   // EW should ALWAYS be arbitrary!
                wxBell();
                m_theGrid->CallAfter([this,a_cellInfo](){this->m_theGrid->GoToCell(a_cellInfo.row, COL_SCORE_EW);});
                return CELL_CHANGE_REJECTED;
            }
        }
        else
        {
            m_theGrid->CallAfter([this,a_cellInfo](){this->m_theGrid->SetCellValue(a_cellInfo.row, a_cellInfo.column, a_cellInfo.newData.Upper());});
            if (bNS && (score != SCORE_NP))
            {
                m_theGrid->SetReadOnly(a_cellInfo.row, COL_SCORE_EW, false );   // set it editable...
                m_theGrid->CallAfter([this,a_cellInfo](){this->m_theGrid->GoToCell(a_cellInfo.row, COL_SCORE_EW);});
                return CELL_CHANGE_OK;   // accept change and goto EW entry
            }
        }
    }

    // now position to next empty score: do it delayed to prevent interaction with 'enter'
    // If there are 2 or more empty scores in the next lines, the 'enter' would work on the repositioning 
    CallAfter(&ScoreEntry::GotoNextEmptyScore);

    // now we have a 'real' NS score, or a complete NS/EW arbitrary score, so save the data
    m_iRowToSave = row;
    CallAfter(&ScoreEntry::SaveRowData);        // delayed, because new data is not copied yet to grid...
    return CELL_CHANGE_OK;  // accept change
}   // OnCellChanging()

static UINT GetSessionPairnrFromGrid(const MyGrid* a_theGrid, int a_row, int a_col)
{
    wxString pair = a_theGrid->GetCellValue(a_row, a_col).Mid(1); // remove first char: space or '*'
    return names::PairnrSessionText2SessionPairnr(pair);
}   // GetSessionPairnrFromGrid()

void ScoreEntry::SaveRowData()
{
    m_bDataChanged = true;      // save changes lateron

    UINT game   = wxAtoi(m_theGrid->GetCellValue(m_iRowToSave, COL_GAME));
    UINT ns     = GetSessionPairnrFromGrid(m_theGrid, m_iRowToSave, COL_NS);
    int scoreNS = score::ScoreFromString(m_theGrid->GetCellValue(m_iRowToSave, COL_SCORE_NS));
    std::vector<score::GameSetData>& scores = svGameSetData[game];
    score::GameSetData setData;
    auto it = std::find_if(scores.begin(), scores.end(), [ns](const score::GameSetData& lhs){return lhs.pairNS == ns || lhs.pairEW == ns;});

    if ( (scoreNS == SCORE_NONE) || (scoreNS == SCORE_NP) ) // remove NP for compatibility with old programm
    {   // empty score or not played, remove from data
            if (it != scores.end() )
            {
                scores.erase(it);
            }

        return;
    }

    // add/change score to/in list
    UINT ew     = GetSessionPairnrFromGrid(m_theGrid, m_iRowToSave, COL_EW);
    int scoreEW = score::IsReal(scoreNS) ? -scoreNS : score::ScoreFromString(m_theGrid->GetCellValue(m_iRowToSave, COL_SCORE_EW));
    setData.pairNS = ns;
    setData.pairEW = ew;
    setData.scoreNS= scoreNS;
    setData.scoreEW= scoreEW;
    if (it != scores.end() )
    {
        *it = setData;
    }
    else
    {
        scores.push_back(setData);
    }
}   // SaveRowData()

void ScoreEntry::OnSelectRound(wxCommandEvent& evt)
{
    AUTOTEST_BUSY("selectRound");
    m_uActiveRound = 1 + evt.GetInt(); //m_pChoiceRound->GetSelection();
    RefreshInfo();
}   // OnSelectRound()

void ScoreEntry::OnSelectGame(wxCommandEvent&)
{
    AUTOTEST_BUSY("selectGame");
    m_uActiveGame = cfg::GetFirstGame() + m_pChoiceGame->GetSelection();
    RefreshInfo();
}   // OnSelectGame()

bool ScoreEntry::FindEmptyScore()
{
    int maxRow  = m_theGrid->GetNumberRows();
    if (maxRow == 0) return false;    //  nothing to do
    int     row         = m_startrow4EmptyScore;
    bool    bFoundEmpty = false;
    int     count       = 0;
    do
    {
        ++count;
        if (++row >= maxRow) row = 0;
        if ( (m_startrow4EmptyScore == -1) && (row == 0) ) m_startrow4EmptyScore = 0;    // prevent endless loop if no empty scores
        wxString score = m_theGrid->GetCellValue(row, COL_SCORE_NS);
        int col = COL_SCORE_NS;

        if (score.Trim().IsEmpty())
        {
            bFoundEmpty = true;
        }
        else
        {
            wxChar firstChar = score[0];
            if ( (firstChar == '%' || firstChar == 'R') && m_theGrid->GetCellValue(row, COL_SCORE_EW).IsEmpty())
            {
                col = COL_SCORE_EW;
                bFoundEmpty = true;
            }
        }

        if (bFoundEmpty)
        {
            // MyLogDebug("NextEmpty(): van rij %i naar rij %i, colom %u", m_startrow4EmptyScore, row, col);
            m_theGrid->SetFocus();
            m_theGrid->GoToCell(row, col);
            m_theGrid->MakeCellVisible(row, 0);     // probably not needed

            m_startrow4EmptyScore = row;            // next goto should start from here
            break;
        }

    } while (count != maxRow );

//    MyLogInfo(_("NextEmpty(%i,%i): %s"), row, col, bFoundEmpty ? _("ja") : _("nee"));
     return bFoundEmpty;
}   // FindEmptyScore()

void ScoreEntry::GotoNextEmptyScore()
{   // use CallAfter() because sometimes in autotest, things go wrong:
    // ensure that ENTER has positioned first before we go to the cell WE want.
    CallAfter([this]{(void)FindEmptyScore();});
}   // GotoNextEmptyScore()

void ScoreEntry::OnNextEmptyScore(wxCommandEvent&)
{
    CallAfter(&ScoreEntry::GotoNextEmptyScore);
}   // OnNextEmptyScore()

void ScoreEntry::OnSelectCell(wxGridEvent& a_event)
{
    m_startrow4EmptyScore = a_event.GetRow();   // start next search for empty score from here
    a_event.Skip();
}   // OnSelectCell()

void ScoreEntry::RefreshInfo()
{   // update grid with actual info
    //LogMessage("ScoreEntry::RefreshInfo()");
    auto&               pSessionInfo = *cfg::GetSessionInfo();
    int                 currentRow  = 0;    // first row  to fill
    UINT                setSize     = pSessionInfo.setSize;
    auto                pGrp        = &pSessionInfo.groupData;
    UINT                firstGame   = pSessionInfo.firstGame;
    UINT                nrOfGames   = pSessionInfo.nrOfGames;
    wxColour            myLightGrey = {230,230,230,wxALPHA_OPAQUE };
    schema::vGameInfo   info;

    names::InitializePairNames();
    InitializeScores();             // get uptodate scores
    m_theGrid->BeginBatch();        // no screenupdates for a while
    m_theGrid->EmptyGrid();         // start with a fresh grid

    if ( (m_uActiveGame < firstGame) || (m_uActiveGame >= nrOfGames + firstGame) )
        m_uActiveGame = firstGame;  // force inrange if firstgame was changed inbetween...

    for (auto itGrp = pGrp->begin(); itGrp != pGrp->end(); ++itGrp)
    {   //GameInfo{UINT round=0; UINT set=0; NS_EW pairs;};
        UINT pairBase   = itGrp->groupOffset;
        UINT pairAbsent = itGrp->absent+pairBase;
        if (m_bSlipOrder)
        {
            schema::GetRoundInfo(itGrp->schemaId, m_uActiveRound, m_bGameOrder, info);
        }
        else
        {
            schema::GetSetInfo(itGrp->schemaId, 1+(m_uActiveGame-firstGame)/setSize, info);
        }
        for ( auto itSet = info.begin(); itSet != info.end(); ++itSet)
        {
            UINT        nsPair      = itSet->pairs.ns+pairBase;
            UINT        ewPair      = itSet->pairs.ew+pairBase;
            if (pairAbsent == nsPair || pairAbsent == ewPair) continue; // don't show entry for absent pair
            UINT        gameBase    = (itSet->set - 1)*setSize + firstGame - 1;
            wxString    nsName      = names::PairnrSession2SessionText(nsPair);
            wxString    ewName      = names::PairnrSession2SessionText(ewPair);

            UINT        setSizeTmp = m_bSlipOrder ? setSize : 1;
            for (UINT game = 1; game <= setSizeTmp; ++game,++currentRow)
            {
                UINT theGame = m_bSlipOrder ? gameBase+game : m_uActiveGame;
                m_theGrid->AppendRows(1);
                m_theGrid->SetCellValue(currentRow, COL_GAME    , FMT("%3u", theGame)  );

                score::GameSetData data;
                bool bReversed;
                wxString scoreNS, scoreEW;
                if (GetScore(theGame, nsPair, data, bReversed))
                {
                    scoreNS = score::ScoreToString(data.scoreNS);
                    scoreEW = score::ScoreToString(data.scoreEW);
                }

                if (bReversed)
                {   // temporary swap NS-EW values
                    std::swap(nsName,ewName);
                    std::swap(nsPair,ewPair);
                }

                #define IS_NS true
                #define IS_EW false
                m_theGrid->SetCellValue(currentRow, COL_NS      , score::VulnerableChar(theGame, IS_NS) + nsName );
                m_theGrid->SetCellValue(currentRow, COL_EW      , score::VulnerableChar(theGame, IS_EW) + ewName );
                if (game == 1)
                {   // if scoreslips, only display playernames for first game on the slip
                    m_theGrid->SetCellValue(currentRow, COL_NAME_NS , names::PairnrSession2GlobalText(nsPair) );
                    m_theGrid->SetCellValue(currentRow, COL_NAME_EW , names::PairnrSession2GlobalText(ewPair) );
                    if (setSizeTmp != 1)
                        m_theGrid->SetRowBackground(currentRow, myLightGrey /* *wxLIGHT_GREY*/);
                }

                if (bReversed)
                {   // swap back....
                    std::swap(nsName,ewName);
                    std::swap(nsPair,ewPair);
                }
                m_theGrid->SetCellValue(currentRow, COL_SCORE_NS, scoreNS);
                if ( data.scoreNS != -data.scoreEW) // only display EW-score if arbitrary
                    m_theGrid->SetCellValue(currentRow, COL_SCORE_EW, scoreEW);

                m_theGrid->SetReadOnly(currentRow, COL_GAME     );
                m_theGrid->SetReadOnly(currentRow, COL_NS       );
                m_theGrid->SetReadOnly(currentRow, COL_EW       );
                m_theGrid->SetReadOnly(currentRow, COL_SCORE_EW );  // default: only for arbitrary NS score, set to edit
                m_theGrid->SetReadOnly(currentRow, COL_NAME_NS  );
                m_theGrid->SetReadOnly(currentRow, COL_NAME_EW  );
            }
        }
    }

    m_theGrid->AutoSizeRows();
    m_theGrid->MakeCellVisible(0, 0);       // needed, because GotoNextEmptyScore() would position column 1 at leftside
    m_startrow4EmptyScore = -1;             // fresh start
    GotoNextEmptyScore();                   // position cursor at first empty score, if any

    m_theGrid->EndBatch();                  // now you can show the changes

    // update/show wanted control(s), hide non-needed control(s)
    if (m_bSlipOrder)
    {
        m_pChoiceRound->Init( nrOfGames/setSize, m_uActiveRound - 1 );
        m_pSizerAllChoices->Show((size_t)CHOICE_ID_GAME_NS);
        m_pSizerAllChoices->Show((size_t)CHOICE_ID_ROUND);
        m_pSizerAllChoices->Hide((size_t)CHOICE_ID_GAME);
    }
    else
    {
        m_pChoiceGame->Init( nrOfGames, m_uActiveGame - firstGame, firstGame-1);
        m_pSizerAllChoices->Hide(CHOICE_ID_GAME_NS);
        m_pSizerAllChoices->Hide(CHOICE_ID_ROUND);
        m_pSizerAllChoices->Show((size_t)CHOICE_ID_GAME);
    }

    Layout();
    static wxString explanation;    // MUST be initialized dynamically: translation
    explanation = _("Ingave scores: normaal: <score>, arbitraal: '%'<score> of 'r'<score> of 'ng'=niet gespeeld");
    SendEvent2Mainframe(this, ID_STATUSBAR_SETTEXT, &explanation);
}   // RefreshInfo()

void ScoreEntry::OnOk()
{
    WriteScoresToDisk();
    cfg::FLushConfigs();    // update diskfiles
}   // OnOk()

void ScoreEntry::OnCancel()
{
    m_bCancelInProgress = true;
    RefreshInfo();  // just repopulate the grid, only local changes
}   // OnCancel()

void ScoreEntry::OnRbSlipGame(wxCommandEvent& a_event)
{
    AUTOTEST_BUSY("slipGame");
    m_bSlipOrder =  (0 == a_event.GetSelection());
    RefreshInfo();
    LogMessage("ScoreEntry::OnRbSlipGame(%i)", m_bSlipOrder ? 0 : 1);
}   // OnRbSlipGame()

void ScoreEntry::OnRbGameNS(wxCommandEvent& a_event)
{
    AUTOTEST_BUSY("rbGameNs");
    m_bGameOrder = ( 0 == a_event.GetSelection() );
    RefreshInfo();
    LogMessage("ScoreEntry::OnRbGameNS(%i)", m_bGameOrder ? 0 : 1);
}   // OnRbGameNS()

void ScoreEntry::UpdateCell(wxCommandEvent& a_event)
{
    const CellData* pData = reinterpret_cast<CellData*>(a_event.GetClientData());
    m_theGrid->SetCellValue(pData->row, pData->column, pData->newData);
}   // UpdateCell()

void ScoreEntry::DoSearch(wxString& a_string)
{
    (void) m_theGrid->Search(a_string);
}   // DoSearch()

void ScoreEntry::OnSwitchNsEw(wxCommandEvent& )
{
    // switch the content of ns-ew pair numbers and names. Please note the vulnerability char
    int      row    = m_theGrid->GetGridCursorRow();
    wxString ns     = m_theGrid->GetCellValue(row, COL_NS);
    wxString ew     = m_theGrid->GetCellValue(row, COL_EW);
    wxString nsLeft = ns.Left(1);
    wxString ewLeft = ew.Left(1);

    wxString nsNew = nsLeft + ew.Mid(1);
    wxString ewNew = ewLeft + ns.Mid(1);

    wxColour org = m_theGrid->GetCellBackgroundColour(row, COL_NS);
    m_theGrid->SetCellBackgroundColour(row, COL_NS, *wxRED );
    m_theGrid->SetCellBackgroundColour(row, COL_EW, *wxRED );
    m_theGrid->Refresh();
    wxString msg = FMT(_("'%s-%s' wisselen met '%s-%s', zeker weten?\n\n "), ns, ew, nsNew, ewNew);
    bool bNo = (wxNO == wxMessageBox( msg, _("draaihand"), wxYES_NO |wxICON_QUESTION ));// wxICON_INFORMATION));
    m_theGrid->SetCellBackgroundColour(row, COL_NS, org);
    m_theGrid->SetCellBackgroundColour(row, COL_EW, org);
    m_theGrid->Refresh();
    if (bNo) return;

    m_theGrid->SetCellValue(row, COL_NS, nsNew);
    m_theGrid->SetCellValue(row, COL_EW, ewNew);
    // now session names are switched

    ns = m_theGrid->GetCellValue(row, COL_NAME_NS); // may be empty, don't care
    ew = m_theGrid->GetCellValue(row, COL_NAME_EW);
    m_theGrid->SetCellValue(row, COL_NAME_NS, ew);
    m_theGrid->SetCellValue(row, COL_NAME_EW, ns);
    // now pair names are switched, update score list
    m_iRowToSave = row;
    ScoreEntry::SaveRowData();
}   // OnSwitchNsEw()

void ScoreEntry::WriteScoresToDisk()
{
    if (!m_bDataChanged) return;
    score::SetScoreData(svGameSetData);
    m_bDataChanged = false;
}   // WriteScoresToDisk()

void ScoreEntry::InitializeScores()
{
    if (ConfigChanged() || m_bCancelInProgress)
    {
        svGameSetData       = *score::GetScoreData();
        m_bDataChanged      = false;
        m_bCancelInProgress = false;
    }
}   // InitializeScores()

void ScoreEntry::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    // position/size of "ChoiceRound" and "ChoiceGame" are only actual if they were setup before,
    // so init both situations to be sure!
    for ([[maybe_unused]] auto ii : {1,2} )
    {
        m_bSlipOrder = !m_bSlipOrder;
        RefreshInfo();
//        Refresh();
        Update();   // force direct update of this page
    }
    // now, original situation is active again!
    AutoTestAddWindowsNames(a_pFile  , m_description);
    AutoTestAddGridInfo    (a_pFile  , m_description, m_theGrid->GetGridInfo());
}   // AutotestRequestMousePositions()

void ScoreEntry::PrintPage()
{
    UINT session = cfg::GetActiveSession();
    wxString sSession = session == 0 ? "" : FMT(_(" van zitting %u"), session);
    wxString sRoundGame = m_bSlipOrder ? FMT(_("ronde %u"), m_uActiveRound) : FMT(_("spel %u"), m_uActiveGame);
    wxString title = FMT(_("Overzicht van de scores%s voor '%s', %s"), sSession, cfg::GetDescription(), sRoundGame);
    m_theGrid->PrintGrid(title, m_theGrid->GetNumberCols());
}   // PrintPage()

static bool GetScore(UINT a_theGame, UINT a_nsPair, score::GameSetData& a_setData, bool& a_bReversed)
{
    assert (a_theGame <= cfg::MAX_GAMES);
    if ( a_theGame > cfg::MAX_GAMES) return false;
    bool bScoreFound        = false;
    const auto& gameData    = svGameSetData[a_theGame];

    a_bReversed = false;    // assume ok
    a_setData={0};
    for (auto setData : gameData)
    {
        if (setData.pairNS == a_nsPair)
        {
            a_setData = setData;
            bScoreFound = true;
            break;
        }
        if (setData.pairEW == a_nsPair)
        {
            a_setData = setData;
            a_bReversed = true;
            bScoreFound = true;
            break;
        }
    }

    return bScoreFound;
}   // GetScore()
