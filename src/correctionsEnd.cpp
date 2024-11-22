// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/grid.h>

#include <wx/filename.h>

#include "cfg.h"
#include "names.h"
#include "correctionsEnd.h"
#include "mygrid.h"
#include "printer.h"
#include "score.h"
#include "corrections.h"
#include "database.h"

CorrectionsEnd::CorrectionsEnd(wxWindow* a_pParent, UINT a_pageId) :Baseframe(a_pParent, a_pageId), m_theGrid(0)
{
    // create and populate grid
    m_theGrid = new MyGrid(this, "GridCorEnd");
    m_theGrid->CreateGrid(0, COL_NR_OF);

//    ;score.2 paar bonus.2 spellen paarnaam
//    100.00     1  11.00   s16     xxx-xxx
    int sizeOne = GetCharWidth();
    #define SIZE_PAIRNR_SES   (4 * sizeOne)
    #define SIZE_PAIRNAME_SES (5 * sizeOne)
    #define SIZE_PAIRNAME     ((cfg::MAX_NAME_SIZE+1)* sizeOne)   /* original name        */
    #define SIZE_PROCENT      (7 * sizeOne)                       /* -100.00 - 100.00  */
    #define SIZE_GAMES        (6 * sizeOne)                       /* like 32             */
    m_theGrid->SetRowLabelSize(SIZE_PAIRNR_SES);
    m_theGrid->SetColSize(COL_PAIRNAME_SESSION, SIZE_PAIRNAME_SES ); m_theGrid->SetColLabelValue(COL_PAIRNAME_SESSION, _("pair"    ));
    m_theGrid->SetColSize(COL_PAIRNAME_GLOBAL , SIZE_PAIRNAME     ); m_theGrid->SetColLabelValue(COL_PAIRNAME_GLOBAL , _("pairname"));
    m_theGrid->SetColSize(COL_COR_SCORE       , SIZE_PROCENT      ); m_theGrid->SetColLabelValue(COL_COR_SCORE       , _("score"   ));
    m_theGrid->SetColSize(COL_COR_BONUS       , SIZE_PROCENT      ); m_theGrid->SetColLabelValue(COL_COR_BONUS       , _("bonus"   ));
    m_theGrid->SetColSize(COL_COR_GAMES       , SIZE_GAMES        ); m_theGrid->SetColLabelValue(COL_COR_GAMES       , _("games"   ));

    wxGridCellAttr* pAttr = new wxGridCellAttr; pAttr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER_VERTICAL);
                      m_theGrid->SetColAttr(COL_PAIRNAME_SESSION, pAttr);
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_SCORE       , pAttr); 
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_BONUS       , pAttr); 
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_GAMES       , pAttr); 

    auto search   = CreateSearchBox();
    auto okCancel = CreateOkCancelButtons();

    wxSizerFlags defaultSF1(1); defaultSF1.Border(wxALL, MY_BORDERSIZE);

    wxBoxSizer* hBoxSearchOk = new wxBoxSizer(wxHORIZONTAL);
    hBoxSearchOk->Add(search    , defaultSF1);
    hBoxSearchOk->AddStretchSpacer(1000);
    hBoxSearchOk->Add(okCancel  , defaultSF1);

    // add to layout
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Entry of sessioncorrections for endresult"));
    vBox->Add(m_theGrid         , defaultSF1.Expand());
    vBox->Add(hBoxSearchOk      , 0);
    SetSizer(vBox);             // add to panel

    m_bDataChanged  = false;    // no changes yet

    RefreshInfo();              // fill the grid with data
    m_description = "CorEnd";
}   // CorrectionsEnd

CorrectionsEnd::~CorrectionsEnd(){}

void CorrectionsEnd::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    AutoTestAddWindowsNames(a_pFile, m_description);
    AutoTestAddGridInfo    (a_pFile, m_description, m_theGrid->GetGridInfo());
}   // AutotestRequestMousePositions()

/*virtual*/ void CorrectionsEnd::BackupData()
{
    if (!m_bDataChanged) return;            // nothing to do...
    m_bDataChanged = false;
    cor::mCorrectionsEnd corrections;       // map of end corrections
    int rows = m_theGrid->GetNumberRows();

    for (int row = 0; row < rows; ++row)
    { 
        wxString score  = m_theGrid->GetCellValue(row, COL_COR_SCORE);
        wxString bonus  = m_theGrid->GetCellValue(row, COL_COR_BONUS);
        wxString games  = m_theGrid->GetCellValue(row, COL_COR_GAMES);
        if (score.IsEmpty() && bonus.IsEmpty()) continue;
            
        cor::CORRECTION_END cor;
        cor.score = score.empty() ? SCORE_IGNORE : AsciiTolong(score, ExpectedDecimalDigits::DIGITS_2);
        cor.bonus = AsciiTolong(bonus, ExpectedDecimalDigits::DIGITS_2);
        cor.games = wxAtoi(games);
        corrections[(UINT)row+1] = cor;     // row+1 equals global pairnr
    }

    /*
    * WHY does next call work without namespace cor::  ???????
    */
    SetCorrectionsEnd(&corrections);
    cor::SaveCorrections();
}   // BackupData()

bool CorrectionsEnd::OnCellChanging(const CellInfo& a_cellInfo)
{
    AUTOTEST_BUSY("cellChanging");
    if (a_cellInfo.pList != m_theGrid)
    {
        return CELL_CHANGE_OK;  //hm, its not my grid
    }

    if (m_bIsScriptTesting)
    {
        wxString msg;
        msg.Printf(_("CorrectionsEnd:: row %d, column %d changes from <%s> to <%s>")
            , a_cellInfo.row
            , a_cellInfo.column
            , a_cellInfo.oldData.c_str()
            , a_cellInfo.newData.c_str()
        );
        LogMessage("%s", msg); // there COULD be a "%" sign in the message.....
    }

    int         row     = a_cellInfo.row;
    int         col     = a_cellInfo.column;
    wxString    oldData = a_cellInfo.oldData;   (void)oldData;
    wxString    newData = a_cellInfo.newData;
    long        minimum = 0L;

    switch (col)
    {
        case COL_COR_BONUS:     // -99.99 <= bonus/minus <= +99.99
            // to check: if new value == "" or '0', clear cell
            minimum = -10000;   // -100%
            [[fallthrough]];    // fallthrough is explicit
        case COL_COR_SCORE:     // 0 <= score <= 100
            {
                wxString formattedData;
                long value = AsciiTolong( newData, ExpectedDecimalDigits::DIGITS_2);
                if ( (value >= minimum) && (value <= 10000) )
                {   // if out of range --> remove it, a zero in COL_COR_SCORE means: ignore this round for this pair in the end-calculation
                    if ( value != 0 || col != COL_COR_BONUS)
                        formattedData = LongToAscii2(value);
                    if ( col == COL_COR_SCORE) 
                    {
                        if (value == 0)
                            m_theGrid->SetCellValue(row, COL_COR_GAMES, ES);
                        else
                        {
                            if (wxAtoi(m_theGrid->GetCellValue(row, COL_COR_GAMES)) == 0)
                                m_theGrid->SetCellValue(row, COL_COR_GAMES, U2String(score::GetNumberOfGamesPlayedByGlobalPair(row+1)));
                            //if (colgames== 0) colgames = aantalspellengespeelddoorpaar()
                        }
                    }
                }
                m_theGrid->CallAfter([this,row,col, formattedData](){this->m_theGrid->SetCellValue(row, col, formattedData); });
            }
            break;
        default:
            break;
    }

    m_bDataChanged = true;
    return CELL_CHANGE_OK;   // accept change
}   // OnCellChanging()

void CorrectionsEnd::RefreshInfo()
{   // update grid with actual info

    names::InitializePairNames();
    cor::InitializeCorrections();
    m_bDataChanged = false;

    UINT globalPairs = names::GetNumberOfGlobalPairs();
    if (globalPairs == 0)
    {
        return;
    }
    //    ;score.2 global pair bonus.2 games   pairname
    //    100.00     1         11.00   s16     xxx - xxx

    m_theGrid->EmptyGrid();                     // remove all if we have 'old' data
    for (int row = 0; row < (int)globalPairs; ++row)  // first get all pairnames
    {
        m_theGrid->AppendRows(1);
        m_theGrid->SetCellValue (row, COL_PAIRNAME_SESSION,       names::PairnrGlobal2SessionText(row+1));
        m_theGrid->SetCellValue (row, COL_PAIRNAME_GLOBAL ,       names::GetGlobalPairInfo(row+1).pairName);  // small separation with previous column
        m_theGrid->SetReadOnly  (row, COL_PAIRNAME_SESSION);
        m_theGrid->SetReadOnly  (row, COL_PAIRNAME_GLOBAL);

        m_theGrid->SetCellEditor(row, COL_COR_GAMES, new wxGridCellNumberEditor(0, cfg::GetNrOfGames() ));
    }

    auto corrections = cor::GetCorrectionsEnd();
    
    for (const auto& it : *corrections)
    {
        cor::CORRECTION_END cs = it.second;
        bool bIgnore = cs.score == SCORE_IGNORE;
        int row = it.first - 1;
        if (!bIgnore) m_theGrid->SetCellValue(row, COL_COR_SCORE, LongToAscii2(cs.score));
        if (cs.score != 0)  // zero means: this session does not add to the end-result: <games> not important
        {
            if (cs.bonus)   // only show non-zero values
            m_theGrid->SetCellValue(row, COL_COR_BONUS, LongToAscii2(cs.bonus));
            m_theGrid->SetCellValue(row, COL_COR_GAMES, U2String    (cs.games));
        }
    }

    Layout();
    static wxString explanation;    // MUST be initialized dynamically: translation
    explanation = _("END CORRECTIONS: 0%: sessionsscore is ignored for total result, <0 or >100: remove correction");
    SendEvent2Mainframe(this, ID_STATUSBAR_SETTEXT, &explanation);
}   // RefreshInfo()

void CorrectionsEnd::OnOk()
{
    BackupData();     // get and store corrections
    cfg::FLushConfigs();    // update diskfiles
}   // OnOk()

void CorrectionsEnd::OnCancel()
{
    RefreshInfo();  // just repopulate the grid, only local changes
}   // OnCancel()

void CorrectionsEnd::DoSearch(wxString& a_string)
{
    (void) m_theGrid->Search(a_string);
}   // DoSearch()

void CorrectionsEnd::PrintPage()
{
    UINT session = cfg::GetActiveSession();
    wxString sSession = session == 0 ? ES : FMT(_(", session %u"), session);
    wxString title = FMT(_("Overview of endcorrections for '%s'%s"), cfg::GetDescription(), sSession);
    m_theGrid->PrintGrid(title, m_theGrid->GetNumberCols(), COL_COR_SCORE);
}   // PrintPage()
