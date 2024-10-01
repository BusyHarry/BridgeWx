// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _NAMEEDITOR_H_
#define _NAMEEDITOR_H_
#pragma once

#include "baseframe.h"

class MyGrid;
class NameInfo;

class NameEditor : public Baseframe
{
public:
    explicit    NameEditor(wxWindow* pParent, UINT pageId);
               ~NameEditor()  override;
    void        RefreshInfo() override final;   // (re)populate the grid
    void        PrintPage()   override final;   // print the names of pairs
    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

protected:
    void        OnOk            () override final;
    void        OnCancel        () override final;
    virtual void DoSearch       (wxString&            ) override final; // handler for 'any' search in derived class
    void        OnAddName       (wxCommandEvent& event);
    virtual void BackupData     () override final;
    bool        OnCellChanging  (const CellInfo& cellInfo) override final;

private:
    void AddName(const wxString& pairname =wxEmptyString, const wxString& clubname=wxEmptyString, const wxString& clubId=wxEmptyString);
    MyGrid*     m_theGrid;
    wxTextCtrl* m_pTxtCtrlSearchBox;
    bool        m_bDataChanged;

    enum
    {
          COL_ZERO = 0
        , COL_PAIRNAME = COL_ZERO
        , COL_CLUBNAME
        , COL_CLUBID
        , COL_NR_OF                         // nr of columns in this grid
    };
};

#endif
