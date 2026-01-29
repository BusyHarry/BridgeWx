// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>

#include "cfg.h"
#include "names.h"
#include "nameEditor.h"
#include "printer.h"

NameEditor::NameEditor(wxWindow* a_pParent, UINT a_pageId) :Baseframe(a_pParent, a_pageId), m_theGrid(0), m_bDataChanged(false)
{
// create and populate grid
    m_theGrid = new MyGrid(this, "GridNameEditor");
    m_theGrid->CreateGrid(0, COL_NR_OF);                // need 3 columns, rowlabel is used as column 0
    m_theGrid->SetRowLabelSize( 4*GetCharWidth() );     // room for 3 digit numbers

    int charSize = GetCharWidth();
    #define PAIR_SIZE_CHARS (cfg::MAX_NAME_SIZE+1)
    #define CLUB_SIZE_CHARS (cfg::MAX_CLUB_SIZE+1)
    #define ID_SIZE_CHARS   8
    #define PAIR_SIZE_PIX (PAIR_SIZE_CHARS * charSize)
    #define CLUB_SIZE_PIX (CLUB_SIZE_CHARS * charSize)
    #define ID_SIZE_PIX   (ID_SIZE_CHARS   * charSize)
    m_theGrid->SetColSize(COL_PAIRNAME, PAIR_SIZE_PIX); m_theGrid->SetColLabelValue(COL_PAIRNAME, _("pairname"));
    m_theGrid->SetColSize(COL_CLUBNAME, CLUB_SIZE_PIX); m_theGrid->SetColLabelValue(COL_CLUBNAME, _("clubname"));
    m_theGrid->SetColSize(COL_CLUBID  , ID_SIZE_PIX  ); m_theGrid->SetColLabelValue(COL_CLUBID,   _("club id" ));
                                                        m_theGrid->SetColLabelAutoTest(COL_PAIRNAME, "pairname");
                                                        m_theGrid->SetColLabelAutoTest(COL_CLUBNAME, "clubname");
                                                        m_theGrid->SetColLabelAutoTest(COL_CLUBID  , "club id" );
//    m_theGrid->SetColFormatNumber(COL_CLUBID);  // what does this implicate?????

    std::vector<MyGrid::SortMethod> methods;
    methods.push_back(MyGrid::SORT_STRING);             // col 0
    methods.push_back(MyGrid::SORT_STRING_NONEMPTY);    // col 1
    methods.push_back(MyGrid::SORT_INTNUMBER_NONEMPTY); // col 2
    m_theGrid->SetSortMethod(methods);          // set sort hints,  without, plain string compare is done

    // create command buttons
    auto        pButtonAdd  = new wxButton(this, ID_NAMEEDIT_ADD, _("Add name"));
    auto        okCancel    = CreateOkCancelButtons();
    auto        search      = CreateSearchBox();
    wxBoxSizer* hBox        = new wxBoxSizer(wxHORIZONTAL);
    hBox->Add(search    , wxSizerFlags(1).Border(wxALL, MY_BORDERSIZE));//    hBox->AddStretchSpacer(10);
    hBox->Add(pButtonAdd, wxSizerFlags(0).Border(wxALL, MY_BORDERSIZE));    hBox->AddStretchSpacer(10);
    hBox->Add(okCancel  , wxSizerFlags(0).Border(wxALL, MY_BORDERSIZE));

    // add to layout
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Entry/change of pair/clubnames"));

    vBox->Add(m_theGrid, wxSizerFlags(1).Expand().Border(wxALL, MY_BORDERSIZE));
    vBox->Add(hBox,      0);   //no borders/align: already done in hBox!
    SetSizer(vBox);     // add to panel

    RefreshInfo();  // now fill the grid with data
//    Show();
//    Layout();
    Bind(wxEVT_BUTTON,      &NameEditor::OnAddName, this, ID_NAMEEDIT_ADD   );

    AUTOTEST_ADD_WINDOW(pButtonAdd, "AddName");
    AUTOTEST_ADD_WINDOW(this      , "Panel"  );
    m_description = "NameEditor";
}   // NameEditor()

NameEditor::~NameEditor()
{
    // keep changes //names::GetRestorePoint();       // restore data if not exited via the 'normal' way
}   // ~NameEditor()

void NameEditor::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    AutoTestAddWindowsNames(a_pFile, m_description);
    AutoTestAddGridInfo    (a_pFile, m_description, m_theGrid->GetGridInfo());
}   // AutotestRequestMousePositions()

bool NameEditor::OnCellChanging(const CellInfo& a_cellInfo)
{
    AUTOTEST_BUSY("cellChanging");
    if (a_cellInfo.pList != m_theGrid)
    {
        return CELL_CHANGE_OK;  //hm, its not my grid
    }

    if (m_bIsScriptTesting)
    {
        wxString msg;
        msg.Printf(_("NameEditor: row %d, column %d changes from <%s> to <%s>")
            , a_cellInfo.row
            , a_cellInfo.column
            , a_cellInfo.oldData.c_str()
            , a_cellInfo.newData.c_str()
        );
        LogMessage("%s", msg);   // there COULD be a "%" sign in the data
    }

    using namespace names;

    m_bDataChanged = true;              // ALWAYS set to true: the CELL_CHANGE_REJECTED is (mis-)used to change the changing column!
    int col = a_cellInfo.column;
    if (col == COL_PAIRNAME)            // pairnames may change, we handle them on Ok/Cancel
    {
        return CELL_CHANGE_OK;          // accept change
    }

    int row = a_cellInfo.row;
    if (col == COL_CLUBNAME)
    {
        wxString    newName = a_cellInfo.newData;
        int         clubId  = wxAtoi(m_theGrid->GetCellValue (row, COL_CLUBID));
        if (newName.IsEmpty())
        {   // remove club info for this pair
            m_theGrid->SetCellValue (row, COL_CLUBID, ES);
        }
        else
        {   // non empty name
            if (clubId == 0)
            {   // no clubname assigned yet: check for exsting clubname
                clubId = GetClubIndex(newName);
                if (clubId == 0)
                {   // new clubname, add to database and update pairinfo
                    clubId = AddClubName(newName);
                    if (clubId == 0)
                        return CELL_CHANGE_REJECTED; // no more room in club table
                    m_theGrid->SetCellValue (row, COL_CLUBID, U2String(clubId));
                }
                else
                {   // get registered name (could be different case)
                    m_theGrid->SetCellValue (row, COL_CLUBNAME, GetClubName(clubId));
                    m_theGrid->SetCellValue (row, COL_CLUBID, U2String( clubId));
                    return CELL_CHANGE_REJECTED;        // now my new value is in the cell!
                }
            }
            else
            {   // non empty name AND and existing club id
                int tmpIndex;
                if (ExistClub(newName, &tmpIndex) && (tmpIndex != clubId))
                {   // user wants to change to other clubname
                    m_theGrid->SetCellValue (row, COL_CLUBNAME, GetClubName(tmpIndex));
                    m_theGrid->SetCellValue (row, COL_CLUBID, U2String( tmpIndex));
                    return CELL_CHANGE_REJECTED;        // now my new value is in the cell!
                }
                // change clubname in database and for other pairs
                bool bResult = SetClubName( newName, clubId); MY_UNUSED(bResult);
                //  RefreshInfo()?????
                for ( int checkRow = 0; checkRow < m_theGrid->GetNumberRows(); ++checkRow)
                {
                    if (checkRow == row) continue;
                    if (clubId == wxAtoi(m_theGrid->GetCellValue (checkRow, COL_CLUBID)))
                    {
                        m_theGrid->SetCellValue (checkRow, COL_CLUBNAME, newName);
                    }
                }
            }
        }
    }
    else
    {   // change in clubId
        int newId = wxAtoi(a_cellInfo.newData);
        if (newId == 0)
        {   // remove clubinfo for this pair
            m_theGrid->SetCellValue (row, COL_CLUBNAME, ES);
            m_theGrid->SetCellValue (row, COL_CLUBID  , ES);
            return CELL_CHANGE_REJECTED;        // now my new value is in the cell!
        }
        else
        {
            if (ExistClub(newId))
                m_theGrid->SetCellValue ( row, COL_CLUBNAME, GetClubName(newId) );
            else
                return CELL_CHANGE_REJECTED;    // not allowed to create clubid
        }
    }

    m_bDataChanged = true;
    return CELL_CHANGE_OK;   // accept change
}   // OnCellChanging()

void NameEditor::RefreshInfo()
{   // update grid with actual info

//    LogMessage("NameEditor::RefreshInfo()");
    m_bDataChanged = false;
    names::InitializePairNames();
    using namespace names;      // sorry, but lots of these calls in this function...
    const PairInfoData& pairInfo = GetGlobalPairInfo();
    //NB todo check if size >= 1
    m_theGrid->BeginBatch();    // no screenupdates for a while
    m_theGrid->EmptyGrid();     // just in case...

    for (auto it = pairInfo.begin()+1; it != pairInfo.end(); ++it)
    {
        int index = it->clubIndex;
        if (index)
            AddName(it->pairName, GetClubName(index), FMT("%2d",index));
        else
            AddName(it->pairName);
    }

    m_theGrid->MakeCellVisible(0, 0);       // probably not needed
    m_theGrid->AutoSizeRows();
//    m_theGrid->AutoSize();                // gives layout errors: grid will use all vertical room in window
//    m_theGrid->AutoSizeColumns();

    m_theGrid->EndBatch();                  // now you can show the changes
    SetRestorePoint();  // make a copy of current names/club info to restore if user cancels the editing
    Layout();
}   // RefreshInfo()

void NameEditor::AddName(const wxString& a_pairName, const wxString& a_clubName, const wxString& a_clubId)
{
    int count = m_theGrid->GetNumberRows();
    if (count < cfg::MAX_PAIRS)
    {
        m_theGrid->AppendRows(1);
        if ( a_pairName.IsEmpty() )
        {
            m_theGrid->SetCellValue(count, COL_PAIRNAME, FMT(_("pair %d"), count + 1     ));
        }
        else
        {
            m_theGrid->SetCellValue(count, COL_PAIRNAME, a_pairName);
            m_theGrid->SetCellValue(count, COL_CLUBNAME, a_clubName);
            m_theGrid->SetCellValue(count, COL_CLUBID  , a_clubId);
        }

        m_theGrid->SetCellEditor(count, COL_PAIRNAME, new wxGridCellTextEditor  (   cfg::MAX_NAME_SIZE  ));
        m_theGrid->SetCellEditor(count, COL_CLUBNAME, new wxGridCellTextEditor  (   cfg::MAX_CLUB_SIZE  ));
        m_theGrid->SetCellEditor(count, COL_CLUBID  , new wxGridCellNumberEditor(0, cfg::MAX_CLUBNAMES+1));
    }
}   // AddName()

void NameEditor::OnAddName(wxCommandEvent& /*event*/)
{
    AUTOTEST_BUSY("addName");
    AddName();
    m_theGrid->MakeCellVisible(m_theGrid->GetNumberRows()-1, 0);
    m_bDataChanged = true;
}   // OnAddName()

/*virtual*/ void NameEditor::BackupData()
{
    MyLogMessage("NameEditor::BackupData()");

    if (!m_bDataChanged) return;
    m_bDataChanged = false;
    using namespace names;

    int maxRows = m_theGrid->GetNumberRows();
    for (int row = 0; row < maxRows; ++row)
    {
        wxString    name    =  m_theGrid->GetCellValue(row, COL_PAIRNAME);
        int         clubId  =  wxAtoi( m_theGrid->GetCellValue(row, COL_CLUBID));
        PairInfo    info( name, clubId);
        if (ExistGlobalPairnr(row+1))
            ChangePairInfo(info, row+1);
        else
            AddGlobalPairInfo(info);

        // clubinfo is already updated when changing...
    }

    // write changed data to disk
    WriteClubNames();
    WritePairNames();
}   // BackupData()

void NameEditor::OnOk()
{
    BackupData();
    names::SetRestorePoint();      // just incase one presses 'cancel' ....
    cfg::FLushConfigs();    // update diskfiles
}   // OnOk()

void NameEditor::OnCancel()
{
    names::GetRestorePoint();       // undo (possible) changes
    RefreshInfo();                  // and repopulate the grid
}   // OnCancel()

void NameEditor::DoSearch(wxString& a_theString)
{
    (void) m_theGrid->Search(a_theString);
}   // DoSearch()

void NameEditor::PrintPage()
{   // assume m_theGrid->m_vColumnSizes is initialized in constructor
    wxString    title       = _("playerlist of ") + cfg::GetDescription();
    UINT        nrOfColumns = COL_NR_OF;
    m_theGrid->PrintGrid(title, nrOfColumns);
}   // PrintPage()
