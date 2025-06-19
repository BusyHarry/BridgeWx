// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _MYGRID_H_
#define _MYGRID_H_
#pragma once

#include<wx/grid.h>
class Baseframe;
class wxBoxSizer;

#define MY_GRIDSORT /* sorting of grid rows, based on column content*/

class MyGrid : public wxGrid
{
public:
    explicit MyGrid(Baseframe* pParent, const wxString& ahkLabel = ES);
    virtual ~MyGrid();

    bool        Search      (const wxString& s2Search );     // find a string in the grid, starting in (last) selected cell.using the physical/sorted grid
    void        EmptyGrid   ();                           // clear the grid and set search position to 0,0
    void        SetMaxChars (int row, int col, int iMaxCharsInColumn);              // setmax nr of chars in a (text)column
    void        SetMaxChars (int row, int col, const wxString& sMaxCharsInColumn);  // setmax nr of chars in a (text)column
    void        PrintGrid   (const wxString& title, UINT nrOfColumnsToPrint, UINT notEmptyfrom = MaxRow);   //print grid content upto <x> columns, if columns > <y> not empty 

#ifdef MY_GRIDSORT
    enum SortMethod
    {
          SORT_DISABLED = 0             // no sorting wanted....
        , SORT_STRING                   // 'normal' not case sensitive string compare
        , SORT_STRING_NONEMPTY          // empty strings displayed AFTER non-empty
        , SORT_SESSIONNAME              // assume session names: AA1 AA2 AA10 AA11: retains order i.s.o AA1 AA10 AA11 AA2 etc
        , SORT_INTNUMBER                // assuming integers in string, sort atoi("123")
        , SORT_INTNUMBER_NONEMPTY       // empty 'numbers' displayed AFTER non-empty
    };
    void        SetSortMethod           (const std::vector<MyGrid::SortMethod>& sortTypes);

    // catch grid functions to translate rows->intern->extern
    // some helper funcions
    void        SetCellValue            ( int row, int col, const wxString& s );
    wxString    GetCellValue            ( int row, int col ) const;
    int         GridRowToExtern         ( int row );            // if sorting: translate internal row to external row nr
    int         GridRowToIntern         ( int row ) const;      // if sorting: translate external row to internal row nr
    bool        AppendRows              ( int numRows = 1, bool updateLabels = true);    // if sorting, extend the sort-array
    bool        SetSortEnable           ( bool enable = true);  // enable/disable row-sorting, return previous setting
    void        SetRowBackground        ( int row, const wxColour& colour); //sets the background colour for a whole row
#endif

    struct GridInfo
    {   // for autotest mouse positions/window labels
        wxWindow*   pGridWindow = nullptr;
        int         rowLabelSize= -1;
        int         colLabelSize= -1;
        std::vector<wxRect> collumnInfo;
    };
    const GridInfo& GetGridInfo();
protected:

private:
    void OnCellChanging     (wxGridEvent& event);       // end of editing: old and new data returned, if changed call Baseframe:: OnCellChanging()
    void OnLeftClickLabel   (wxGridEvent& event);       // left click column header, prevent column selection

    GridInfo                        m_gridInfo;
    Baseframe*                      m_pParent;          // needed to call its OnCellChanging()
    std::vector<MyGrid::SortMethod> m_vSortMethod;      // vector of sorttypes for columns
    enum {MaxRow = 9999};
#ifdef MY_GRIDSORT
    // catch grid functions to translate rows->intern->extern
    // some helper funcions
    enum SORT_DIRECTION { SORT_NONE = 0,SORT_UP = 1, SORT_DOWN = 2, SORT_MAX=SORT_DOWN };

    void OnSortColumn           (wxGridEvent& event);   // sorting, based on column
    void SortOnLeftClickLabel   (wxGridEvent& event);   // left click column header
    void SortOnCellChanging     (wxGridEvent& event);   // end of editing: old and new data returned, if changed call Baseframe:: OnCellChanging()
    void SortOnSelectCell       (wxGridEvent& event);   // set start position for search to current selection

    void BindLate();                                // be sure that your are latest binder to get first called on events!
    std::vector<int>    m_sortedRows;               // [n] -> original row 'n' is now shown at row [n]
    SORT_DIRECTION      m_sortType;                 // UP, DOWN, NONE
    int                 m_sortedCol;                // last column that was sorted upon, WXNOT_FOUND if none
    bool                m_bEnableSort;              // true if sorting wanted, default not set
#endif
};

#endif
