// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/msgdlg.h>

#include <wx/filename.h>

#include "cfg.h"
#include "names.h"
#include "correctionsSession.h"
#include "mygrid.h"
#include "printer.h"
#include "score.h"
#include "corrections.h"

static constexpr auto COR_MP_MIN    = -5000;
static constexpr auto COR_MP_MAX    = +5000;
static constexpr auto COR_EXTRA_MAX = +5000;
static constexpr auto COR_EXTRA_MIN = 0;
/*
* remarks: 'max' and 'extra' are meant for things like a combi-table.
* max  : maximum mp's you can get at that table (for all games you play there)
* extra: the mp's you really  got at that table (for all games....)
* The 'max' is added to the total achievable mp's and the 'extra' is added to your total achieved mp's
*/
CorrectionsSession::CorrectionsSession(wxWindow* a_pParent, UINT a_pageId) :Baseframe(a_pParent, a_pageId), m_theGrid(0)
{
    // create and populate grid
    m_theGrid = new MyGrid(this, "GridCorSession");
    m_theGrid->CreateGrid(0, COL_NR_OF);
//    m_theGrid->SetRowLabelSize( 4*GetCharWidth() );   // room for 3 digit numbers
//    m_theGrid->HideRowLabels();                         // don't need 1 to N
    int sizeOne = GetCharWidth();
    #define SIZE_PAIRNR_SES (4 * sizeOne)
    #define SIZE_PAIR_SES   (5 * sizeOne)
    #define SIZE_PAIRNAME   ((cfg::MAX_NAME_SIZE+1)* sizeOne)   /* original name        */
    #define SIZE_PAIRNR     (6 * sizeOne)                       /* "AB12 *"             */
    #define SIZE_PROCENT    (4 * sizeOne)                       /* just numbers 1-120   */
    #define SIZE_MP         (5 * sizeOne)                       /* like -2200           */
    m_theGrid->SetRowLabelSize(SIZE_PAIRNR_SES);
    m_theGrid->SetColSize(COL_PAIRNAME_SESSION, SIZE_PAIR_SES   ); m_theGrid->SetColLabelValue(COL_PAIRNAME_SESSION, _("pair"     ));
    m_theGrid->SetColSize(COL_PAIRNAME_GLOBAL , SIZE_PAIRNAME   ); m_theGrid->SetColLabelValue(COL_PAIRNAME_GLOBAL , _("pairname" ));
    m_theGrid->SetColSize(COL_COR_PROCENT     , SIZE_PROCENT    ); m_theGrid->SetColLabelValue(COL_COR_PROCENT     ,   "%"         );
    m_theGrid->SetColSize(COL_COR_MP          , SIZE_MP         ); m_theGrid->SetColLabelValue(COL_COR_MP          , _("mp"       ));
    m_theGrid->SetColSize(COL_COR_MAX         , SIZE_MP         ); m_theGrid->SetColLabelValue(COL_COR_MAX         , _("max"      ));
    m_theGrid->SetColSize(COL_COR_EXTRA       , SIZE_MP         ); m_theGrid->SetColLabelValue(COL_COR_EXTRA       , _("extra"    ));

    wxGridCellAttr* pAttr = new wxGridCellAttr;
    pAttr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER_VERTICAL);
                      m_theGrid->SetColAttr(COL_PAIRNAME_SESSION, pAttr);
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_PROCENT     , pAttr); 
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_MP          , pAttr); 
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_MAX         , pAttr); 
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_EXTRA       , pAttr); 

    auto search   = CreateSearchBox();
    auto okCancel = CreateOkCancelButtons();

    wxSizerFlags defaultSF1(1); defaultSF1.Border(wxALL, MY_BORDERSIZE);
    wxBoxSizer* hBoxSearchOk = new wxBoxSizer(wxHORIZONTAL);
    hBoxSearchOk->Add(search    , defaultSF1);
    hBoxSearchOk->AddStretchSpacer(1000);
    hBoxSearchOk->Add(okCancel  , defaultSF1);

    // add to layout
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Entry of session corrections"));
    vBox->Add(m_theGrid         , defaultSF1.Expand());
    vBox->Add(hBoxSearchOk      , 0);
    SetSizer(vBox);             // add to panel
    m_bDataChanged  = false;    // no changes yet

    RefreshInfo();              // fill the grid with data
    m_description = "CorSession";
}   // CorrectionsSession()

CorrectionsSession::~CorrectionsSession(){}

void CorrectionsSession::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    AutoTestAddWindowsNames(a_pFile, m_description);
    AutoTestAddGridInfo    (a_pFile, m_description, m_theGrid->GetGridInfo());
}   // AutotestRequestMousePositions()

/*virtual*/ void CorrectionsSession::BackupData()
{
    if (!m_bDataChanged) return;    //nothing to do...
    m_bDataChanged = false;

    cor::mCorrectionsSession corrections;    // map of session corrections
    int rows = m_theGrid->GetNumberRows();
    for (int row = 0; row < rows; ++row)
    { 
        //        CORRECTION_SESSION() {type = '%'; correction = 0; extra = 0; maxExtra = 0; }
        //         char type; int correction; long extra; int maxExtra;
        wxString procent = m_theGrid->GetCellValue(row, COL_COR_PROCENT);
        wxString mp      = m_theGrid->GetCellValue(row, COL_COR_MP     );
        wxString maxe    = m_theGrid->GetCellValue(row, COL_COR_MAX    );
        wxString extra   = m_theGrid->GetCellValue(row, COL_COR_EXTRA  );
        if (procent.IsEmpty() && mp.IsEmpty() && maxe.IsEmpty() && extra.IsEmpty()) continue;
            
        cor::CORRECTION_SESSION cor;
        if (mp.IsEmpty())
        {   // if procent AND mp are empty, we use '%' type
            cor.type = '%';
            cor.correction = wxAtoi(procent);
        }
        else
        {
            cor.type = 'm';
            cor.correction = wxAtoi(mp);
        }

        cor.extra = AsciiTolong(extra);
        cor.maxExtra = wxAtoi(maxe);
        corrections[(UINT)row+1] = cor;     // row+1 equals sessionpairnr
    }

    /*
    * WHY does next call work without namespace cor::  ???????
    */
    SetCorrectionsSession(&corrections);
    cor::SaveCorrections();
}   // BackupData()

static wxString Long12ToString(long a_value, int a_nrOfDecimals = 1)
{
    if (a_nrOfDecimals == 1)
    {
        if ((a_value % 10) == 0)
            return FMT("%ld  ", a_value/ 10);
        return FMT("%ld.%01ld", a_value/ 10, std::abs(a_value) %  10);
    }
    return     FMT("%ld.%02ld", a_value/100, std::abs(a_value) % 100);
}   // Long12ToString()

static wxString ForceInRange( const wxString& val2Check, int a_minVal, int a_maxVal, bool a_bIntType = false)
{
    int ival2Check = a_bIntType ? wxAtoi(val2Check) : AsciiTolong(val2Check);
    ival2Check = std::clamp(ival2Check, a_minVal, a_maxVal);
    return a_bIntType ? I2String(ival2Check) : Long12ToString(ival2Check);
}   // ForceInRange()

bool CorrectionsSession::OnCellChanging(const CellInfo& a_cellInfo)
{
    AUTOTEST_BUSY("cellChanging");
    if (a_cellInfo.pList != m_theGrid)
    {
        return CELL_CHANGE_OK;  //hm, its not my grid
    }

    if (m_bIsScriptTesting)
    {
        wxString msg;
        msg.Printf(_("CorrectionsSession:: row %d, column %d changes from <%s> to <%s>")
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
    int         value   = wxAtoi(newData);
    switch (col)
    {
        case COL_COR_PROCENT:
        case COL_COR_MP:
            if (!newData.empty())
            {
                m_theGrid->SetCellValue(row, (col == COL_COR_PROCENT) ? COL_COR_MP : COL_COR_PROCENT , ES);
                if ( value == 0 || value == -100)   // you get -100 (==min) if there is rubbisch in the cell ....
                    m_theGrid->CallAfter([this,row,col](){this->m_theGrid->SetCellValue(row, col, ES); });
            }
            break;
        case COL_COR_MAX:
            if (newData.empty() || 0 == value)
            {
                m_theGrid->SetCellValue(row, COL_COR_EXTRA, ES);
                m_theGrid->CallAfter([this,row,col](){this->m_theGrid->SetCellValue(row, col, ES); });
            }
            else
            {
                if ( value < COR_EXTRA_MIN) value = COR_EXTRA_MIN;
                if ( value > COR_EXTRA_MAX) value = COR_EXTRA_MAX;
                m_theGrid->SetCellValue(row, COL_COR_EXTRA, ForceInRange( m_theGrid->GetCellValue(row,COL_COR_EXTRA), COR_EXTRA_MIN, value*10));
            }
            break;
        case COL_COR_EXTRA:
            if (!newData.empty())
            {
                if (m_theGrid->GetCellValue(row,COL_COR_MAX).empty())
                    return CELL_CHANGE_REJECTED;
                wxString extra = ForceInRange(newData, COR_EXTRA_MIN, 10*wxAtoi(m_theGrid->GetCellValue(row,COL_COR_MAX)));
                m_theGrid->CallAfter([this,row,col,extra](){this->m_theGrid->SetCellValue(row, col, extra);});
            }
            break;
        default:
            break;
    }

    m_bDataChanged = true;
    return CELL_CHANGE_OK;   // accept change
}   // OnCellChanging()

void CorrectionsSession::RefreshInfo()
{   // update grid with actual info

    names::InitializePairNames();
    cor::InitializeCorrections();
    m_bDataChanged = false;

    UINT sessionPairs = cfg::GetNrOfSessionPairs();
    if (sessionPairs == 0)
    {
        MyMessageBox(_("No session data yet.."));
        return;
    }

    m_theGrid->EmptyGrid();                     // remove all if we have 'old' data
    for (int row = 0; row < (int)sessionPairs; ++row)  // first get all pairnames
    {
        m_theGrid->AppendRows(1);
        m_theGrid->SetCellValue (row, COL_PAIRNAME_SESSION,       names::PairnrSession2SessionText(row+1));
        m_theGrid->SetCellValue (row, COL_PAIRNAME_GLOBAL ,       names::PairnrSession2GlobalText (row+1));  // small separation with previous column
        m_theGrid->SetReadOnly  (row, COL_PAIRNAME_SESSION);
        m_theGrid->SetReadOnly  (row, COL_PAIRNAME_GLOBAL);
        m_theGrid->SetCellEditor(row, COL_COR_PROCENT   , new wxGridCellNumberEditor(-100, 100));
        m_theGrid->SetCellEditor(row, COL_COR_MP        , new wxGridCellNumberEditor(COR_MP_MIN    , COR_MP_MAX    ));
        m_theGrid->SetCellEditor(row, COL_COR_MAX       , new wxGridCellNumberEditor(COR_EXTRA_MIN , COR_EXTRA_MAX ));
        // hm, floating point strings with space(s) at the end give debug assertions on editing celvalue....
        // m_theGrid->SetCellEditor(row, COL_COR_EXTRA  , new wxGridCellFloatEditor (6 ,1, wxGRID_FLOAT_FORMAT_FIXED ));
    }

    auto corrections = cor::GetCorrectionsSession();
    for (const auto& it : *corrections)
    {
        UINT sessionPair = it.first;
        cor::CORRECTION_SESSION cs = it.second;

        int row = sessionPair - 1;
        int col = cs.type == '%' ? COL_COR_PROCENT : COL_COR_MP;
        if (cs.correction)  // zero means: not present
            m_theGrid->SetCellValue(row, col, I2String(cs.correction));
        if (cs.extra || cs.maxExtra)
        {
            m_theGrid->SetCellValue(row, COL_COR_MAX  , I2String      (cs.maxExtra));
            m_theGrid->SetCellValue(row, COL_COR_EXTRA, Long12ToString(cs.extra   ));
        }
    }

    Layout();
    static wxString explanation;    // MUST be initialized dynamically: translation
    explanation = _("SESSION CORRECTIONS in full % or mp (matchpoints), 0 or empty: remove correction");
    SendEvent2Mainframe(this, ID_STATUSBAR_SETTEXT, &explanation);
}   // RefreshInfo()

void CorrectionsSession::OnOk()
{
    BackupData();           // get and store corrections
    cfg::FLushConfigs();    // update diskfiles
}   // OnOk()

void CorrectionsSession::OnCancel()
{
    RefreshInfo();  // just repopulate the grid, only local changes
}   // OnCancel()

void CorrectionsSession::DoSearch(wxString& a_string)
{
    (void) m_theGrid->Search(a_string);
}   // DoSearch()

void CorrectionsSession::PrintPage()
{
    UINT session = cfg::GetActiveSession();
    wxString sSession = session == 0 ? ES : FMT(_(", session %u"), session);
    wxString title = FMT(_("Overview of the sessioncorrections for '%s'%s"), cfg::GetDescription(), sSession);
    m_theGrid->PrintGrid(title, m_theGrid->GetNumberCols(), COL_COR_PROCENT);
}   // PrintPage()
