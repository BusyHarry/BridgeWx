// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/msgdlg.h>

#include "validators.h"
#include "cfg.h"
#include "names.h"
#include "correctionsSession.h"
#include "mygrid.h"
#include "printer.h"
#include "score.h"
#include "corrections.h"
#include "main.h"

static constexpr auto COR_MP_MIN        = -25;
static constexpr auto COR_MP_MAX        = +25;
static constexpr auto COR_EXTRA_MAX     = +100; // %: combi for 8 rounds, 4 games: (8-1)*2*4 mp, butler: 4*10 imps
static constexpr auto COR_EXTRA_MIN     = 0;    // %, butler: -COR_EXTRA_MAX
static constexpr auto COR_PERCENT_MIN   = -100;
static constexpr auto COR_PERCENT_MAX   = +100;
/*
* remarks: 'max' and 'extra' are meant for things like a combi-table.
* max  : maximum mp's you can get at that table (for all games you play there)
* extra: the mp's you really  got at that table (for all games....)
* The 'max' is added to the total achievable mp's and the 'extra' is added to your total achieved mp's
*/
#define sizeOne GetCharWidth()
#define SIZE_PAIRNR_SES (4 * sizeOne)
#define SIZE_PAIR_SES   (5 * sizeOne)
#define SIZE_PAIRNAME   ((cfg::MAX_NAME_SIZE+1)* sizeOne)   /* original name        */
#define SIZE_PAIRNR     (6 * sizeOne)                       /* "AB12 *"             */
#define SIZE_PROCENT    (4 * sizeOne)                       /* just numbers 1-120   */
#define SIZE_MP         (6 * sizeOne)                       /* like -2200           */
#define SIZE_GAMES      (8 * sizeOne)

CorrectionsSession::CorrectionsSession(wxWindow* a_pParent, UINT a_pageId) :Baseframe(a_pParent, a_pageId), m_theGrid(0)
{
    // create and populate grid
    m_theGrid = new MyGrid(this, "GridCorSession");
    m_theGrid->CreateGrid(0, COL_NR_OF);
//    m_theGrid->SetRowLabelSize( 4*GetCharWidth() );   // room for 3 digit numbers
//    m_theGrid->HideRowLabels();                         // don't need 1 to N
    m_theGrid->SetRowLabelSize(SIZE_PAIRNR_SES);
    m_theGrid->SetColSize(COL_PAIRNAME_SESSION, SIZE_PAIR_SES   ); m_theGrid->SetColLabelValue(COL_PAIRNAME_SESSION, _("pair"     ));
    m_theGrid->SetColSize(COL_PAIRNAME_GLOBAL , SIZE_PAIRNAME   ); m_theGrid->SetColLabelValue(COL_PAIRNAME_GLOBAL , _("pairname" ));
    m_theGrid->SetColSize(COL_COR_PROCENT     , SIZE_PROCENT    ); m_theGrid->SetColLabelValue(COL_COR_PROCENT     ,   "%"         );   // TRANSLATORS: 'mp' == MatchPoints
    m_theGrid->SetColSize(COL_COR_MP          , SIZE_MP         ); m_theGrid->SetColLabelValue(COL_COR_MP          , _("mp"       ));
    m_theGrid->SetColSize(COL_COR_MAX         , SIZE_MP         ); m_theGrid->SetColLabelValue(COL_COR_MAX         , _("max"      ));
    m_theGrid->SetColSize(COL_COR_EXTRA       , SIZE_MP         ); m_theGrid->SetColLabelValue(COL_COR_EXTRA       , _("extra"    ));
    m_theGrid->SetColSize(COL_COR_GAMES       , SIZE_GAMES      ); m_theGrid->SetColLabelValue(COL_COR_GAMES       , _("games"    ));
                                                                   m_theGrid->SetColLabelAutoTest(COL_PAIRNAME_SESSION, "pair");
                                                                   m_theGrid->SetColLabelAutoTest(COL_PAIRNAME_GLOBAL , "pairname");
                                                                   m_theGrid->SetColLabelAutoTest(COL_COR_PROCENT     ,   "%"     );
                                                                   m_theGrid->SetColLabelAutoTest(COL_COR_MP          , "mp"      );
                                                                   m_theGrid->SetColLabelAutoTest(COL_COR_MAX         , "max"     );
                                                                   m_theGrid->SetColLabelAutoTest(COL_COR_EXTRA       , "extra"   );
                                                                   m_theGrid->SetColLabelAutoTest(COL_COR_GAMES       , "games"   );
    wxGridCellAttr* pAttr = new wxGridCellAttr;
    pAttr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER_VERTICAL);
                      m_theGrid->SetColAttr(COL_PAIRNAME_SESSION, pAttr); pAttr->SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTER_VERTICAL);
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_PROCENT     , pAttr);
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_MP          , pAttr);
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_MAX         , pAttr);
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_EXTRA       , pAttr);
    pAttr->IncRef();  m_theGrid->SetColAttr(COL_COR_GAMES       , pAttr);

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
    m_bButler = !cfg::GetButler();  // force first setup in RefreshInfo()

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
        //        CORRECTION_SESSION() {type = '%'; correction = 0; extra = 0; maxExtra = 0; games = 0;}
        //         char type; int correction; long extra; int maxExtra; UINT games
        wxString procent = m_theGrid->GetCellValue(row, COL_COR_PROCENT);
        wxString mp      = m_theGrid->GetCellValue(row, COL_COR_MP     );
        wxString maxe    = m_theGrid->GetCellValue(row, COL_COR_MAX    );
        wxString extra   = m_theGrid->GetCellValue(row, COL_COR_EXTRA  );
        wxString games   = m_theGrid->GetCellValue(row, COL_COR_GAMES  );
        if ( procent.IsEmpty() && mp.IsEmpty() && maxe.IsEmpty() && extra.IsEmpty() ) continue;

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

        cor.extra       = AsciiTolong(extra);
        cor.maxExtra    = wxAtoi(maxe);
        cor.games       = wxAtoi(games);
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
        return FMT("%ld.%01ld", a_value/ 10, std::abs(a_value) %  10L);
    }
    return     FMT("%ld.%02ld", a_value/100, std::abs(a_value) % 100L);
}   // Long12ToString()

static wxString ForceInRange( const wxString& val2Check, int a_minVal, int a_maxVal, bool a_bIntType = false)
{
    int ival2Check = a_bIntType ? wxAtoi(val2Check) : AsciiTolong(val2Check);
    ival2Check = std::clamp(ival2Check, a_minVal, a_maxVal);
    return a_bIntType ? I2String(ival2Check) : Long12ToString(ival2Check);
}   // ForceInRange()

void CorrectionsSession::ClearCombiData(int a_row)
{
    m_theGrid->SetCellValue(a_row, COL_COR_MAX  , ES);
    m_theGrid->SetCellValue(a_row, COL_COR_EXTRA, ES);
    m_theGrid->SetCellValue(a_row, COL_COR_GAMES, ES);
}   // ClearCombiData()

bool CorrectionsSession::OnCellChanging(const CellInfo& a_cellInfo)
{
    AUTOTEST_BUSY("cellChanging");
    if ( a_cellInfo.pList != m_theGrid )
    {
        return CELL_CHANGE_OK;  //hm, its not my grid
    }

    if ( m_bIsScriptTesting )
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
            if ( !newData.IsEmpty() )
            {
                m_theGrid->SetCellValue(row, COL_COR_MP, ES);   // only one can have a value
                newData = ForceInRange(newData, COR_PERCENT_MIN, COR_PERCENT_MAX, true);
                value = wxAtoi(newData);
                if ( value == 0 || value == COR_PERCENT_MIN )
                    newData.clear();
            }
            break;
        case COL_COR_MP:
            if ( !newData.IsEmpty() )
            {
                m_theGrid->SetCellValue(row, COL_COR_PROCENT, ES);  // only one can have a value
                newData = ForceInRange(newData, COR_MP_MIN, COR_MP_MAX, true);
                value = wxAtoi(newData);
                if ( value == 0 || value == COR_MP_MIN )
                    newData.clear();
            }
            break;
        case COL_COR_MAX:
            if ( newData.IsEmpty() || value == 0 )
            {
                ClearCombiData(row);
                newData.clear();
                break;
            }

            newData = ForceInRange(newData, COR_EXTRA_MIN, COR_EXTRA_MAX, true);
            value = wxAtoi(newData);
            m_theGrid->UpdateLimitMax(row, COL_COR_EXTRA, value);
            m_theGrid->SetCellValue(row, COL_COR_EXTRA, ForceInRange( m_theGrid->GetCellValue(row,COL_COR_EXTRA), COR_EXTRA_MIN, value*10));
            if ( m_theGrid->GetCellValue(row, COL_COR_GAMES).IsEmpty() )
                m_theGrid->SetCellValue(row, COL_COR_GAMES, U2String(cfg::GetSetSize()));
            break;
        case COL_COR_EXTRA:
            if ( newData.IsEmpty() )
            {
                ClearCombiData(row);
                break;
            }

            if ( m_bButler )
            {
                newData = ForceInRange(newData, -COR_EXTRA_MAX*10, COR_EXTRA_MAX*10);
                if ( m_theGrid->GetCellValue(row, COL_COR_GAMES).IsEmpty() )    // set default set-size
                    m_theGrid->SetCellValue(row, COL_COR_GAMES, U2String(cfg::GetSetSize()));
            }
            else
            {   // percent scoring
                if ( m_theGrid->GetCellValue(row,COL_COR_MAX).IsEmpty() )
                    return CELL_CHANGE_REJECTED;        // only extra-data if we have a maximum inserted
                newData = ForceInRange(newData, COR_EXTRA_MIN, 10*wxAtoi(m_theGrid->GetCellValue(row,COL_COR_MAX)));
            }
            break;
        case COL_COR_GAMES:
            if ( newData.IsEmpty() )
                ClearCombiData(row);
            else
            {
                if ( m_theGrid->GetCellValue(row, COL_COR_MAX).IsEmpty() && m_theGrid->GetCellValue(row, COL_COR_EXTRA).IsEmpty() )
                    return CELL_CHANGE_REJECTED;        // only games if extra/max data
                else
                    newData = ForceInRange(newData, 1, cfg::GetSetSize(), true);
            }
            break;
        default:
            break;
    }

    m_theGrid->CallAfter([this, row, col, newData](){this->m_theGrid->SetCellValue(row, col, newData); });
    m_bDataChanged = true;
    return CELL_CHANGE_OK;   // accept change
}   // OnCellChanging()

void CorrectionsSession::InitButlerProcent()
{
    static wxString explanation;    // MUST be initialized dynamically: translation, static: must survive this function!
    SendEvent2Mainframe(this, ID_STATUSBAR_SETTEXT, &explanation);
    if (m_bButler == cfg::GetButler()) return;  // no change
    m_bButler = !m_bButler;

    wxString toolTip;
    if (m_bButler)
    {
        toolTip = _(
            "combi-table:"
            "\nextra: the imp's earned"
            "\ngames: the nr of games at that combi-table"
        );
        m_theGrid->SetColSize      (COL_COR_PROCENT, 0);
        m_theGrid->SetColSize      (COL_COR_MAX    , 0);
        m_theGrid->SetColLabelValue(COL_COR_MP     , _("imps"));
        explanation = _("SESSION CORRECTIONS in imps and results from a combi-table");
    }
    else
    {
        toolTip = _(
            "combi-table:"
            "\nmax  : maximum mp's to get"
            "\nextra: the mp's earned"
            "\ngames: the nr of games at that combi-table"
        );
        m_theGrid->SetColSize      (COL_COR_PROCENT, SIZE_PROCENT);
        m_theGrid->SetColSize      (COL_COR_MAX    , SIZE_MP     );
        m_theGrid->SetColLabelValue(COL_COR_MP     , _("mp")     );
        explanation = _("SESSION CORRECTIONS in full % or mp (matchpoints) and results from a combi-table");
    }

    m_theGrid->GetGridColLabelWindow()->SetToolTip(toolTip);
}   // InitButlerProcent()

void CorrectionsSession::RefreshInfo()
{   // update grid with actual info

    InitButlerProcent();                // init columns/tooltip for butler or percentage calculation
    names::InitializePairNames();
    cor::InitializeCorrections();
    m_bDataChanged = false;

    UINT sessionPairs = cfg::GetNrOfSessionPairs();
    if (sessionPairs == 0)
    {
        MyMessageBox(_("No session data yet.."));
        return;
    }

    bool bButler = cfg::GetButler();
    // the minimun value for the COL_COR_EXTRA depends on butler:
    int minVal = bButler ? -COR_EXTRA_MAX : COR_EXTRA_MIN;

    m_theGrid->EmptyGrid();                     // remove all if we have 'old' data
    for (int row = 0; row < (int)sessionPairs; ++row)  // first get all pairnames
    {
        m_theGrid->AppendRows(1);
        m_theGrid->SetCellValue (row, COL_PAIRNAME_SESSION,       names::PairnrSession2SessionText(row+1, true));
        m_theGrid->SetCellValue (row, COL_PAIRNAME_GLOBAL ,       names::PairnrSession2GlobalText (row+1));  // small separation with previous column
        m_theGrid->SetReadOnly  (row, COL_PAIRNAME_SESSION);
        m_theGrid->SetReadOnly  (row, COL_PAIRNAME_GLOBAL);
        m_theGrid->SetCellEditor(row, COL_COR_PROCENT   , new MyGridCellEditorWithValidator(COR_PERCENT_MIN, COR_PERCENT_MAX   ));
        m_theGrid->SetCellEditor(row, COL_COR_MP        , new MyGridCellEditorWithValidator(COR_MP_MIN     , COR_MP_MAX        ));
        m_theGrid->SetCellEditor(row, COL_COR_MAX       , new MyGridCellEditorWithValidator(COR_EXTRA_MIN  , COR_EXTRA_MAX     ));
        m_theGrid->SetCellEditor(row, COL_COR_GAMES     , new MyGridCellEditorWithValidator(1              , cfg::GetSetSize() ));
        m_theGrid->SetCellEditor(row, COL_COR_EXTRA     , new MyGridCellEditorWithValidator(minVal         , COR_EXTRA_MAX , 1 ));
    }

    auto corrections = cor::GetCorrectionsSession();
    for (const auto& [sessionPair, cs] : *corrections)
    {
        int row = sessionPair - 1;
        int col = cs.type == '%' ? COL_COR_PROCENT : COL_COR_MP;
        if (cs.correction)  // zero means: not present
            m_theGrid->SetCellValue(row, col, I2String(cs.correction));
        if ( cs.maxExtra || cs.extra || cs.games )
        {   // some combi result
            if ( !bButler && cs.maxExtra )
            {
                m_theGrid->UpdateLimitMax(row, COL_COR_MAX  , cs.maxExtra);
                m_theGrid->UpdateLimitMax(row, COL_COR_EXTRA, cs.maxExtra);
            }
            m_theGrid->SetCellValue(row, COL_COR_MAX  , I2String      (cs.maxExtra));
            m_theGrid->SetCellValue(row, COL_COR_EXTRA, Long12ToString(cs.extra   ));
            m_theGrid->SetCellValue(row, COL_COR_GAMES, U2String      (cs.games   ));
        }
    }

    Layout();
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
