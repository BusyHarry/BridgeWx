// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>

#include <wx/filename.h>
#include <numeric>

#include "cfg.h"
#include "names.h"
#include "assignNames.h"
#include "mygrid.h"
#include "printer.h"
#include "score.h"

AssignNames::AssignNames(wxWindow* a_pParent, UINT a_pageId) :Baseframe(a_pParent, a_pageId), m_theGrid(0)
{
    // create and populate grid
    m_bDataChanged = false;
    m_theGrid = new MyGrid(this, "GridAssignNames");
    m_theGrid->CreateGrid(0, COL_NR_OF);                // need 2 columns if session <=1, else 5
    m_theGrid->SetRowLabelSize( 4*GetCharWidth() );     // room for 3 digit numbers

    int charSize = GetCharWidth();
    #define SIZE_PAIRNAME_CHARS (cfg::MAX_NAME_SIZE+1)  /* original name*/
    #define SIZE_PAIRNR_CHARS   7                       /* AB12*/
    #define SIZE_ID_CHARS       8                       /* just numbers 1-120, minimum char-count!*/
    #define SIZE_PAIRNAME_PIX (SIZE_PAIRNAME_CHARS* charSize)
    #define SIZE_PAIRNR_PIX   (SIZE_PAIRNR_CHARS  * charSize)
    #define SIZE_ID_PIX       (SIZE_ID_CHARS      * charSize)
    m_theGrid->SetColSize(COL_PAIRNAME             , SIZE_PAIRNAME_PIX); m_theGrid->SetColLabelValue(COL_PAIRNAME           , _("pairname"));
    m_theGrid->SetColSize(COL_PAIRNR_SESSION       , SIZE_PAIRNR_PIX  ); m_theGrid->SetColLabelValue(COL_PAIRNR_SESSION     , _("pair no" ));
    m_theGrid->SetColSize(COL_PAIRNR_SESSION_PREV  , SIZE_ID_PIX      ); m_theGrid->SetColLabelValue(COL_PAIRNR_SESSION_PREV, _("pair S-1"));
    m_theGrid->SetColSize(COL_RANK_TOTAL_PREV      , SIZE_ID_PIX      ); m_theGrid->SetColLabelValue(COL_RANK_TOTAL_PREV    , _("rank"    ));
    m_theGrid->SetColSize(COL_RANK_SESSION_PREV    , SIZE_ID_PIX      ); m_theGrid->SetColLabelValue(COL_RANK_SESSION_PREV  , _("rank S-1"));

    std::vector<MyGrid::SortMethod> methods;
    methods.push_back(MyGrid::SORT_STRING);
    methods.push_back(MyGrid::SORT_SESSIONNAME);
    methods.push_back(MyGrid::SORT_SESSIONNAME);
    methods.push_back(MyGrid::SORT_INTNUMBER);
    methods.push_back(MyGrid::SORT_INTNUMBER);
    m_theGrid->SetSortMethod(methods);          // set sort hints: without this, plain string compare is done

    auto pStaticText    = new wxStaticText(this, wxID_ANY, _("Assign pairnames on base of:  "));
    auto pButtonOrg     = new wxButton    (this, wxID_ANY, _("Original nr"));
    m_pButtonRankTotal  = new wxButton    (this, wxID_ANY, _("Rank total" ));
    m_pButtonRankPrev   = new wxButton    (this, wxID_ANY, _("Rank S-1"   ));
    auto pButtonClear   = new wxButton    (this, wxID_ANY, Unique(_("Clear")));
    wxBoxSizer* hBox    = new wxBoxSizer  (wxHORIZONTAL);
    hBox->Add(pStaticText       , 1, wxBOTH | wxALL | wxALIGN_CENTER_VERTICAL, MY_BORDERSIZE);
    hBox->Add(pButtonOrg        , 0, wxBOTH | wxALL | wxALIGN_CENTER_VERTICAL, MY_BORDERSIZE);
    hBox->Add(m_pButtonRankTotal, 0, wxBOTH | wxALL | wxALIGN_CENTER_VERTICAL, MY_BORDERSIZE);
    hBox->Add(m_pButtonRankPrev , 0, wxBOTH | wxALL | wxALIGN_CENTER_VERTICAL, MY_BORDERSIZE);
    hBox->AddStretchSpacer(100);    // clear button ALWAYS at right end for fixed position!
    hBox->Add(pButtonClear      , 0, wxBOTH | wxALL | wxALIGN_CENTER_VERTICAL, MY_BORDERSIZE);

    auto search   = CreateSearchBox();
    auto okCancel = CreateOkCancelButtons();
    wxBoxSizer* hBoxSearchOk = new wxBoxSizer(wxHORIZONTAL);
    hBoxSearchOk->Add(search   , 1, wxBOTH | wxALL, MY_BORDERSIZE);   hBoxSearchOk->AddStretchSpacer(1000);   // ok/cancel ALWAYS at right side
    hBoxSearchOk->Add(okCancel , 0, wxBOTH | wxALL, MY_BORDERSIZE);

    // add to layout
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Assign pairnames"));
    vBox->Add(m_theGrid         , 1, wxEXPAND | wxALL, MY_BORDERSIZE);
    vBox->Add(hBox              , 0);
    vBox->Add(hBoxSearchOk      , 0);
    SetSizer(vBox);     // add to panel

    pButtonOrg        ->Bind(wxEVT_BUTTON, &AssignNames::OnOriginal    , this);
    m_pButtonRankTotal->Bind(wxEVT_BUTTON, &AssignNames::OnTotalRank   , this);
    m_pButtonRankPrev ->Bind(wxEVT_BUTTON, &AssignNames::OnPreviousRank, this);
    pButtonClear      ->Bind(wxEVT_BUTTON, &AssignNames::OnClear       , this);

    RefreshInfo();  // now fill the grid with data

    AUTOTEST_ADD_WINDOW(pButtonOrg        , "OnOriginal" );
    AUTOTEST_ADD_WINDOW(m_pButtonRankTotal, "OnRankTotal");
    AUTOTEST_ADD_WINDOW(m_pButtonRankPrev , "OnRankPrev" );
    AUTOTEST_ADD_WINDOW(pButtonClear      , "Clear"      );
    m_description = "AssignNames";
    //LogMessage("AssignNames() aangemaakt");
}   // AssignNames()

AssignNames::~AssignNames()
{
//    names::GetRestorePoint();       // restore data if not exited via the 'normal' way
}

void AssignNames::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    const MyGrid::GridInfo* pGridInfo = nullptr;
    UINT sessions[]={2,cfg::GetActiveSession()};
    if (sessions[1] < 2)    //only needed for sessions < 2
    {
        for (auto session : sessions )
        {   // need session > 1 for getting position of several buttons
            cfg::SetActiveSession(session);
            RefreshInfo();
            Update();
            if (!pGridInfo) pGridInfo = &m_theGrid->GetGridInfo();  // Only visible columns have 'good' values...
        }
    }

    if (!pGridInfo) pGridInfo = &m_theGrid->GetGridInfo();  // get data, if we don't have it yet
    AutoTestAddWindowsNames(a_pFile, m_description);
    AutoTestAddGridInfo    (a_pFile, m_description, *pGridInfo);
}   // AutotestRequestMousePositions()

bool AssignNames::OnCellChanging(const CellInfo& a_cellInfo)
{
    AUTOTEST_BUSY("cellChanging");
    if (a_cellInfo.pList != m_theGrid)
    {
        return CELL_CHANGE_OK;  //hm, its not my grid
    }

    if ( a_cellInfo.column != COL_PAIRNR_SESSION ) return CELL_CHANGE_REJECTED;

    if (m_bIsScriptTesting)
    {
        wxString msg;
        msg.Printf(_("AssignNames:: row %d, column %d changes from <%s> to <%s>")
            , a_cellInfo.row
            , a_cellInfo.column
            , a_cellInfo.oldData.c_str()
            , a_cellInfo.newData.c_str()
        );
        LogMessage("%s", msg);   // there COULD be a "%" sign in the old/new data...
    }
    UINT uSessionPair;
    wxString sSessionPair = a_cellInfo.newData;
    if (!names::ValidateSessionPairName(sSessionPair, uSessionPair) ) return CELL_CHANGE_REJECTED;

    int row = a_cellInfo.row;

    m_theGrid->CallAfter([this,row,sSessionPair](){m_theGrid->SetCellValue(row,COL_PAIRNR_SESSION,sSessionPair);});
    if (sSessionPair == a_cellInfo.oldData)  return CELL_CHANGE_OK; // no change

    int rowCount = m_theGrid->GetNumberRows();
    for (int ii = 0; ii < rowCount; ++ii)
    {   // remove old assign if it was in use before
        if (ii == row) continue;
        if (m_theGrid->GetCellValue(ii, COL_PAIRNR_SESSION).CmpNoCase(sSessionPair) == 0)
            m_theGrid->SetCellValue(ii, COL_PAIRNR_SESSION, names::GetNotSet());
    }
    m_bDataChanged = true;
    return CELL_CHANGE_OK;   // accept change
}   // OnCellChanging()

static void RankIndexToPairIndex(UINT_VECTOR& a_uiV)
{   // a_uiv[rank] = globalPairNr, change to: a_uiv[globalPairNr] = rank
    UINT_VECTOR dst;
    dst.resize(a_uiV.size(),0);
    for ( UINT rank = 0; rank < a_uiV.size(); ++rank)
        dst[a_uiV[rank]] = rank;
    if (dst.size()) dst[0] = 0;
    a_uiV = dst;
}   // RankIndexToPairIndex()

#define TO_STRING(ii) FMT("%3u", (unsigned int)ii)

void AssignNames::RefreshInfo()
{   // update grid with actual info

    wxArrayString             prevAssign;
    std::vector<unsigned int> prevRankTotal;
    std::vector<unsigned int> prevRankSession;

    m_bDataChanged = false;
    names::InitializePairNames();
    UINT activeSession = cfg::GetActiveSession();

    if (activeSession <= 1)
    {   //no extra colums needed
        m_theGrid->HideCol(COL_PAIRNR_SESSION_PREV);
        m_theGrid->HideCol(COL_RANK_TOTAL_PREV);
        m_theGrid->HideCol(COL_RANK_SESSION_PREV);
        m_pButtonRankTotal->Hide();
        m_pButtonRankPrev ->Hide();
    }
    else
    {   // extra colums needed
        m_theGrid->ShowCol(COL_PAIRNR_SESSION_PREV);
        m_theGrid->ShowCol(COL_RANK_TOTAL_PREV);
        m_theGrid->ShowCol(COL_RANK_SESSION_PREV);
        m_pButtonRankTotal->Show();
        m_pButtonRankPrev ->Show();
        names::GetSessionAssignmentsPrevious(prevAssign);
        score::GetSessionRankTotalPrevious  (prevRankTotal);    RankIndexToPairIndex(prevRankTotal);
        score::GetSessionRankPrevious       (prevRankSession);  RankIndexToPairIndex(prevRankSession);
    }

    const names::PairInfoData& pairInfo = names::GetGlobalPairInfo();

    m_theGrid->BeginBatch();    // no screenupdates for a while
    m_theGrid->EmptyGrid();     // start with a fresh grid

    int pairs = pairInfo.size() - 1;
    m_theGrid->AppendRows(pairs);

    for (long long ii = 0; ii < pairs; ++ii)
    {
        m_theGrid->SetCellValue(ii, COL_PAIRNAME      , pairInfo[ii+1].pairName );
        m_theGrid->SetCellValue(ii, COL_PAIRNR_SESSION, names::PairnrGlobal2SessionText(ii+1));

        m_theGrid->SetReadOnly(ii, COL_PAIRNAME             );
        m_theGrid->SetReadOnly(ii, COL_PAIRNR_SESSION_PREV  );
        m_theGrid->SetReadOnly(ii, COL_RANK_TOTAL_PREV      );
        m_theGrid->SetReadOnly(ii, COL_RANK_SESSION_PREV    );
        if (activeSession > 1)
        {
            m_theGrid->SetCellValue(ii, COL_PAIRNR_SESSION_PREV , prevAssign               [ii+1]);
            m_theGrid->SetCellValue(ii, COL_RANK_TOTAL_PREV     , TO_STRING(prevRankTotal  [ii+1]));
            m_theGrid->SetCellValue(ii, COL_RANK_SESSION_PREV   , TO_STRING(prevRankSession[ii+1]));
        }
    }

    m_theGrid->MakeCellVisible(0, 0);       // probably not needed
    m_theGrid->AutoSizeRows();

    m_theGrid->EndBatch();                  // now you can show the changes
    Layout();
}   // RefreshInfo()

/*virtual*/ void AssignNames::BackupData()
{
    if (!m_bDataChanged) return;
    m_bDataChanged = false;

    std::vector<unsigned int> newAssign;
    newAssign.resize(cfg::MAX_PAIRS+1, 0);

    auto rows = m_theGrid->GetNumberRows();

    for (auto row = 0; row < rows; ++row)
    {
        UINT     sessionNr;
        wxString sessionName = m_theGrid->GetCellValue(row, COL_PAIRNR_SESSION );
        names::ValidateSessionPairName(sessionName, sessionNr); // sessionNr: ALWAYS set: 0 if invalid name
        newAssign[row+1LL] = sessionNr;
    }

    names::WriteAssignmentsToDisk(newAssign);
}   // BackupData()

void AssignNames::OnOk()
{
    BackupData();
    cfg::FLushConfigs();    // update diskfiles
}   // OnOk()

void AssignNames::OnCancel()
{
    RefreshInfo();  // just repopulate the grid, only local changes
}   // OnCancel()

void AssignNames::OnOriginal(wxCommandEvent&)
{
    AUTOTEST_BUSY("original");
    std::vector<unsigned int> newAssign;
    newAssign.resize(cfg::MAX_PAIRS);
    std::iota(newAssign.begin(), newAssign.end(), 0);   // fill with 0,1,2,3,...
    UpdateColumnAssign(newAssign);
    m_bDataChanged = true;
}   // OnOriginal()

void AssignNames::OnTotalRank(wxCommandEvent&)
{
    AUTOTEST_BUSY("rank");
    std::vector<unsigned int> newAssign;
    score::GetSessionRankTotalPrevious(newAssign);
    UpdateColumnAssign(newAssign);
    m_bDataChanged = true;
}   // OnTotalRank()

void AssignNames::OnPreviousRank(wxCommandEvent&)
{
    AUTOTEST_BUSY("previous");
    std::vector<unsigned int> newAssign;
    score::GetSessionRankPrevious(newAssign);
    UpdateColumnAssign(newAssign);
    m_bDataChanged = true;
}   // OnPreviousRank()

void AssignNames::OnClear(wxCommandEvent&)
{
    AUTOTEST_BUSY("clear");
    std::vector<unsigned int> newAssign;
    newAssign.resize(cfg::MAX_PAIRS+1, 0);
    UpdateColumnAssign(newAssign);
    m_bDataChanged = true;
}   // OnClear()

void AssignNames::UpdateColumnAssign(std::vector<unsigned int>& a_newAssign)
{
    AUTOTEST_BUSY("updateAssign");
    RankIndexToPairIndex(a_newAssign);
    auto rows = m_theGrid->GetNumberRows();
    for (auto ii = 0; ii < rows; ++ii)
    {
        m_theGrid->SetCellValue(ii, COL_PAIRNR_SESSION, names::PairnrGlobal2SessionText(ii+1,&a_newAssign));
    }
}   // UpdateColumnAssign()

void AssignNames::DoSearch(wxString& a_string)
{
    (void) m_theGrid->Search(a_string);
}   // DoSearch()

void AssignNames::PrintPage()
{
    UINT nrOfPairs = cfg::GetNrOfSessionPairs();
    UINT session   = cfg::GetActiveSession();
    wxString sessionString = session > 0 ? FMT(_(", session %u"), session) : "";

    wxString title = FMT(_("Pairname assignment for '%s'%s\n\nPair name"), cfg::GetDescription(), sessionString );
    for (UINT index = 1; index <= nrOfPairs; ++index)
    {
        title+=FMT("\n%4s %s", names::PairnrSession2SessionText(index), names::PairnrSession2GlobalText(index));
    }

    UINT nrOfColumns = cfg::GetActiveSession() > 1 ? m_theGrid->GetNumberCols() : 2;
    m_theGrid->PrintGrid(title, nrOfColumns);
}   // PrintPage()
 
