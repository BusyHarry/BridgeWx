// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/grid.h>

#include "baseframe.h"
#include "utils.h"
#include "cfg.h"
#include "names.h"
#include "printer.h"
#include "mygrid.h"

#include <wx/wxcrtvararg.h>

MyGrid::MyGrid(Baseframe* a_pParent, const wxString& a_ahkLabel) : wxGrid(a_pParent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxWANTS_CHARS | wxBORDER_SIMPLE, a_ahkLabel)
{
    m_pParent    = a_pParent;
    if (a_pParent) SetLabelFont(a_pParent->GetFont());  // just incase the parent font is NOT standard
    SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_CENTER);
    SetTabBehaviour(Tab_Wrap);  // wrap to next/previous row: till end/begin
    UseNativeColHeader(true);   // V3.3.*: disables highlight of columnheader when a cell is selected
    Bind(wxEVT_GRID_CELL_CHANGING,    &MyGrid::OnCellChanging,   this, wxID_ANY);   // check for changes and signal owner
    Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &MyGrid::OnLeftClickLabel, this, wxID_ANY);

#ifdef MY_GRIDSORT
    Bind(wxEVT_GRID_COL_SORT,         &MyGrid::OnSortColumn    , this, wxID_ANY);   // sorting
    m_sortedCol     = wxNOT_FOUND;
    m_sortType      = SORT_NONE;
    m_bEnableSort   = true;
    CallAfter(&MyGrid::BindLate);
    //    SetUseNativeColLabels(true);    // for sorting indication
#endif
}

MyGrid::~MyGrid(){}

void MyGrid::PrintGrid( const wxString& a_title, UINT a_nrOfColumnsToPrint, UINT a_notEmptyfrom)
{
    int rows = GetNumberRows();
    if (rows <= 0) return; //nothing to print

    UINT cols   = (UINT)GetNumberCols();
    int cWidth  = GetCharWidth();
    if (a_nrOfColumnsToPrint > cols) a_nrOfColumnsToPrint = cols;

    prn::BeginPrint(a_title);
    wxString header = "\n   ";  // 3 spaces: 3 digit rownumber
    for (UINT col = 0; col < a_nrOfColumnsToPrint; ++col)
    {
        int size=GetColSize(col);
        if (size <= 0) // ignore 'hidden' columns, they have a size of 0
            continue;
        header += FMT(" %-*s", size/cWidth, GetColLabelValue(col));
    }
    prn::PrintLine(header + "\n");

    for (int row = 0; row < rows; ++row)
    {
        bool bData = false;    // check if wanted columns have data
        wxString info = FMT("%3d", row+1);
        for (UINT col = 0; col < a_nrOfColumnsToPrint; ++col)
        {
            int size=GetColSize(col);
            if (size <= 0)
                continue;
            wxString columnValue = GetCellValue(row, col);
            if ( col >= a_notEmptyfrom && !columnValue.IsEmpty() ) bData = true;
            info += FMT(" %-*s", size/cWidth, columnValue);
        }
        if (bData || a_nrOfColumnsToPrint < a_notEmptyfrom)
        {
            info.Trim(TRIM_RIGHT);
            prn::PrintLine(info + "\n");
        }
    }

    prn::PrintLine(cfg::GetCopyrightDateTime());
    prn::EndPrint();
}   // PrintGrid()

void MyGrid::OnCellChanging(wxGridEvent& a_event)
{
    int row             = a_event.GetRow();
    int col             = a_event.GetCol();
    wxString oldData    = GetCellValue(row, col);
    wxString newData    = a_event.GetString();

    if (oldData != newData)
    {   // let owner check if new data is ok
        CellInfo cellInfo( this, row, col, oldData, newData);
        if ( CELL_CHANGE_REJECTED == m_pParent->OnCellChanging(cellInfo) )
        {
            a_event.Veto(); // don't put 'newData' back in the cell: not ok or owner changed it himself...
        }
        else
            a_event.SetString(cellInfo.newData);
    }
}   // OnCellChanging()

bool MyGrid::Search(const wxString& a_s2Search )
{
    int rows = GetNumberRows();
    if ((rows == 0) || a_s2Search.IsEmpty()) return false;      // no data to search for

    wxString search(a_s2Search);
    search.MakeLower();

    int searchCol = wxGrid::GetGridCursorCol(); // we search in the 'real' grid as it is shown
    int searchRow = wxGrid::GetGridCursorRow();

    int     cols        = GetNumberCols();
    int     startRow    = searchRow;     // remember startposition so you can stop searching when arriving here again
    int     startCol    = searchCol;
    bool    bFound      = false;
    bool    bBeginPos;

    do
    {   // loop over columns from left to right, top to bottem
        if (++searchCol >= cols)     // update search row/col, also if found! Next search starts 1 up!
        {
            searchCol = 0;
            if (++searchRow >= rows)
                searchRow = 0;
        }

        wxString cel = wxGrid::GetCellValue(searchRow, searchCol).MakeLower();    // unsorted row needed
        if (wxString::npos != cel.find(search))
        {
            bFound = true;      // got you!
            SetFocus();
            wxGrid::GoToCell(searchRow, searchCol);                               //  untranslated row needed
        }

        bBeginPos = (startRow == searchRow) && (startCol == searchCol);
    } while ( !bFound && !bBeginPos );

    return bFound;  //usefull???
}   // Search()

void MyGrid::EmptyGrid()
{
    int rows = GetNumberRows();
    if (rows) DeleteRows(0, rows);
#ifdef MY_GRIDSORT
    m_sortType = SORT_NONE;
    m_sortedRows.clear();
#endif;
}   // EmptyGrid()

void MyGrid::SetMaxChars(int a_row, int a_col, int a_iMaxCharsInColumn)
{
    wxString tmp = U2String(a_iMaxCharsInColumn);
    SetMaxChars(a_row, a_col, tmp );
}   // SetMaxChars()

void MyGrid::SetMaxChars(int a_row, int a_col, const wxString& a_sMaxCharsInColumn)
{
    auto pEdit = GetCellEditor(a_row, a_col);
    pEdit->SetParameters(a_sMaxCharsInColumn);
    pEdit->DecRef();
}   // SetMaxChars()

void MyGrid::SetRowBackground(int a_row, const wxColour& a_colour)
{
    int cols = GetNumberCols();
    for ( ; cols >= 0; --cols )
        SetCellBackgroundColour(a_row, cols, a_colour);
}   // SetRowBackground()

void MyGrid::OnLeftClickLabel(wxGridEvent& a_event)
{   // eat it, if its a column label: no whole column selection
    int col = a_event.GetCol();
    LogMessage(_("MyGrid::OnLeftClickLabel(column: %i)"), col);
    if ( col == wxNOT_FOUND)
        a_event.Skip(); // passtrough if its not a column header
}   // OnLeftClickLabel()

const MyGrid::GridInfo& MyGrid::GetGridInfo()
{   // info for creating mousepositions
    m_gridInfo.collumnInfo.clear();
    m_gridInfo.pGridWindow  = this;
    m_gridInfo.rowLabelSize = GetRowLabelSize();
    m_gridInfo.colLabelSize = GetColLabelSize();

    AppendRows(1);   // be sure to have atleast one row
    auto nrOfCols = GetNumberCols();
    for (int col = 0; col < nrOfCols; ++col)
    {
        m_gridInfo.collumnInfo.push_back(CellToRect(0, col));
    }
    DeleteRows(GetNumberRows()-1, 1); // remove added row
    return m_gridInfo;
}   // GetGridInfo()

#ifdef MY_GRIDSORT

void MyGrid::SetSortMethod(const std::vector<SortMethod>& a_sortTypes)
{
    m_vSortMethod = a_sortTypes;
}   // SetSortMethod()

void MyGrid::BindLate()
{   // (try to) be the latest 'binder': you will get the events first!
    Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &MyGrid::SortOnLeftClickLabel, this, wxID_ANY);
    Bind(wxEVT_GRID_CELL_CHANGING   , &MyGrid::SortOnCellChanging  , this, wxID_ANY);
    Bind(wxEVT_GRID_SELECT_CELL     , &MyGrid::SortOnSelectCell    , this, wxID_ANY);
}   // BindLate()

static void GridEventSetRow(wxGridEvent& a_evt, int a_row)
{   // Will change the protected 'm_row' variable in 'a_evt' to 'a_row'
    // Now we don't need the change in 'include/wx/grid.h' which added a public method SetRow() to access the 'm_row' variable
    class MywxGridEvent : public wxGridEvent
    {   // dummy class to be able to access protected members of the base class
        public:
        MywxGridEvent() {}; // not used...
        void SetRow(int a_row){wxGridEvent::m_row = a_row;}
    };

    auto pMy = reinterpret_cast<MywxGridEvent*>(&a_evt);
    pMy->SetRow(a_row);
}   // GridEventSetRow()

#define ROW2EXTERN_SKIP(event)  do {event.Skip(); if (m_sortType != SORT_NONE) GridEventSetRow(event, GridRowToExtern(event.GetRow()));} while (0)  /* let the real method do its work*/

void MyGrid::SortOnCellChanging(wxGridEvent& a_event)
{
    ROW2EXTERN_SKIP(a_event);
}   // SortOnCellChanging()

void MyGrid::SortOnSelectCell(wxGridEvent& a_event)
{
    ROW2EXTERN_SKIP(a_event);
}   // SortOnSelectCell()

int MyGrid::GridRowToExtern(int a_row)
{
    if ( (m_sortType != SORT_NONE) && (static_cast<size_t>(abs(a_row)) < m_sortedRows.size()))
    {
        a_row = m_sortedRows[a_row];
    }

    return a_row;
}   // GridRowToExtern()

int MyGrid::GridRowToIntern(int a_row) const
{
    if (m_sortType != SORT_NONE)
    {
        auto it = std::find(m_sortedRows.begin(), m_sortedRows.end(), a_row);
        if (it != m_sortedRows.end())
        {
            a_row = it - m_sortedRows.begin();  // = index in table
        }
    }

    return a_row;
}   // GridRowToIntern()

void MyGrid::SetCellValue(int a_row, int col, const wxString& str)
{
    wxGrid::SetCellValue( GridRowToIntern(a_row), col, str);
}   // SetCellValue()

wxString MyGrid::GetCellValue(int a_row, int a_col) const
{
    return wxGrid::GetCellValue( GridRowToIntern(a_row), a_col);
}   // GetCellValue()

#include <numeric>

bool MyGrid::AppendRows(int a_numRows, bool a_updateLabels)
{
    if ( (m_sortType != SORT_NONE) && (a_numRows > 0) )
    {   // extend the sort array and map new rows to there external row nr
        size_t size = m_sortedRows.size();
        m_sortedRows.resize(size + a_numRows);
        std::iota(m_sortedRows.begin()+size, m_sortedRows.end(), size);
    }

    return wxGrid::AppendRows(a_numRows, a_updateLabels);
}   // AppendRows()

[[maybe_unused]] bool MyGrid::GridSortEnable(bool a_bEnable)
{
    if ( !a_bEnable && (m_sortType != SORT_NONE) )
    {   // undo sorting
        m_sortType = SORT_MAX;
        wxGridEvent dummy;
        OnSortColumn(dummy);
    }

    bool bOldSort = m_bEnableSort;
    m_bEnableSort = a_bEnable;

    return bOldSort;
}   // GridSortEnable()

void MyGrid::SortOnLeftClickLabel(wxGridEvent& a_event)
{
    LogMessage(_("MyGrid::SortOnLeftClickLabel(column: %i)"), a_event.GetCol());
    a_event.Skip(); // passthrough
}   // SortOnLeftClickLabel()

static wxString NormaliseSessionName(wxString str)
{
    const wxChar* ps = str.c_str();
    auto len = str.Len();
    size_t ii;
    for (ii=0; ii < len-1; ++ii)
    {
        if (std::isdigit(ps[ii])) break;
    }
    if (ii == len-1 ) str.insert(ii, ' ');  // convert single digit to 'double' digit
    return str;
}   // NormaliseSessionName()

bool CompareSessionNames(const wxString& s1, const wxString& s2)
{   //  cmp case sensitive, because session names SHOULD be all uppercase!
    wxChar c1 = *s1.c_str();    // *(const wxChar*)s1;
    wxChar c2 = *s2.c_str();    // *(const wxChar*)s2;
    if ( c1 != c2 )
        return c1 < c2;
    return NormaliseSessionName(s1) < NormaliseSessionName(s2);
}   // CompareSessionNames()

bool MyCompare(const wxString& s1, const wxString& s2, MyGrid::SortMethod a_sortType, bool a_bUp)
{
    bool  bResult(false);
    switch(a_sortType)
    {
        case MyGrid::SORT_SESSIONNAME:
            bResult = CompareSessionNames(s1, s2);
            break;
        case MyGrid::SORT_INTNUMBER:
                bResult = ( wxAtoi(s1) < wxAtoi(s2) );
            break;
        case MyGrid::SORT_INTNUMBER_NONEMPTY:   // number compare: 'empty' numbers ALWAYS displayed after non-empty numbers
            {
                int n1 =  wxAtoi(s1);
                int n2 =  wxAtoi(s2);
                if ( (n1 == 0) && s1.IsEmpty() ) n1 = a_bUp ? INT_MAX-1 : INT_MIN+1;
                if ( (n2 == 0) && s2.IsEmpty() ) n2 = a_bUp ? INT_MAX-1 : INT_MIN+1;
                bResult = n1 < n2;
            }
            break;
        case MyGrid::SORT_STRING_NONEMPTY:    // string compare: empty strings ALWAYS displayed after non-empty strings
            {
                int type=0;
                if (s1.IsEmpty()) type  = 1;
                if (s2.IsEmpty()) type += 2;
                switch (type)
                {
                    case 0: // 2 non-empty strings
                        bResult = (s1.CmpNoCase(s2) < 0);   // "a" < "b"  -> true
                        break;
                    case 1: // 1 empty, 2 non-empty
                        bResult = a_bUp  ? false : true;    // "" < "b"  bUp -> false
                        break;
                    case 2:
                        bResult = a_bUp ? true : false;     // "a" < ""  bUp -> true
                        break;
                    case 3: // both empty
                        bResult = false;
                        break;
                }
            }   // SORT_STRING_NONEMPTY
            break;
        default:    // compare as strings, not case sensitive
            bResult = (s1.CmpNoCase(s2) < 0);
            break;
    }

    return bResult;
}   // MyCompare()

void MyGrid::OnSortColumn(wxGridEvent& a_event)
{
    LogMessage(_("MyGrid::OnSortColumn(column: %i)"), a_event.GetCol());

    int rows    = GetNumberRows();
    int sortCol = a_event.GetCol();

    if ( (sortCol == wxNOT_FOUND) || (rows == 0) || !m_bEnableSort)
    {
        a_event.Skip(); // we don't handle this one
        return;
    }

    int cols = GetNumberCols();
    std::vector< std::vector<wxString> > data;
    data.resize(rows);
    for (int row = 0; row < rows; ++row)
    {   // get data from grid in original order, so we can sort and put it back in the grid in (new) order
        int sortedRow = (m_sortType == SORT_NONE) ? row : m_sortedRows[row];    // if sorted, then this vector should be initialized!
        data[sortedRow].resize(cols);
        for (int col = 0; col < cols; ++col)
        {
            data[sortedRow][col] = wxGrid::GetCellValue(row, col);
        }
    }

    //  now sort it;
    const SORT_DIRECTION next[] = {SORT_UP, SORT_DOWN, SORT_NONE};
    if (sortCol == m_sortedCol)
    {
        m_sortType = next[m_sortType];
    }
    else
    {
        m_sortedCol= sortCol;
        m_sortType = next[0];
    }

    m_sortedRows.resize(rows);
    std::iota (m_sortedRows.begin(), m_sortedRows.end(), 0); // Fill with 0, 1, ..., i.e. non-sorted! 0->0, 1->1 etc
    SortMethod method = (static_cast<size_t>(sortCol) < m_vSortMethod.size()) ? m_vSortMethod[sortCol] : SORT_DISABLED;
    if ( (m_sortType != SORT_NONE) && (method != SORT_DISABLED) )
    {   // we need to sort the data
        bool bUp = (m_sortType == SORT_UP);
        std::sort(m_sortedRows.begin(), m_sortedRows.end(),
                    [&data, sortCol, bUp, method] ( int row1, int row2 ) -> bool
                    {
                        if (!bUp) std::swap(row1, row2);
                        wxString s1 = data[row1][sortCol];
                        wxString s2 = data[row2][sortCol];
                        return MyCompare(s1, s2, method, bUp);
                    }
                );
    }

    BeginBatch();
    // now putback the data in the sorted rows
    for (int row = 0; row < rows; ++row)
    {
        int newRow = m_sortedRows[row];
        for (int col = 0; col < cols; ++col)
        {
            wxGrid::SetCellValue(row, col, data[newRow][col]);
        }
    }
    EndBatch();
    // a_event.Skip(); // passtrough to other users??? no, here it would  select the whole column
}   // OnSortColumn()

#endif
