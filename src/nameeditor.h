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
               ~NameEditor()  override = default;
    void        RefreshInfo() final;    // (re)populate the grid
    void        PrintPage()   final;    // print the names of pairs
    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

protected:
    void        OnOk            () final;
    void        OnCancel        () final;
    void        DoSearch        (wxString&            ) final;  // handler for 'any' search in derived class
    void        OnAddName       (const wxCommandEvent& event);
    void        BackupData      () final;
    bool        OnCellChanging  (const CellInfo& cellInfo) final;

private:
    void AddName(const wxString& pairname =wxEmptyString, const wxString& clubname=wxEmptyString, const wxString& clubId=wxEmptyString);
    MyGrid*     m_theGrid;
    wxTextCtrl* m_pTxtCtrlSearchBox;
    bool        m_bDataChanged;
};

#endif
