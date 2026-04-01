// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _ASSIGNNAMES_H_
#define _ASSIGNNAMES_H_
#pragma once

#include "baseframe.h"

class MyGrid;
class NameInfo;

class AssignNames : public Baseframe
{
public:
    explicit    AssignNames(wxWindow* pParent, UINT pageId);
               ~AssignNames() override;
    void        RefreshInfo() final;   // (re)populate the grid
    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

protected:
    void        OnOk            () final;
    void        OnCancel        () final;
    void        DoSearch        (wxString&         string) final;
    bool        OnCellChanging  (const CellInfo& cellInfo) final;
    void        BackupData      () final;
private:
    void        UpdateColumnAssign (std::vector<unsigned int>& newAssign);
    void        OnOriginal      (const wxCommandEvent& event);
    void        OnTotalRank     (const wxCommandEvent& event);
    void        OnPreviousRank  (const wxCommandEvent& event);
    void        OnClear         (const wxCommandEvent& event);
    void        PrintPage       () final;


    MyGrid*     m_theGrid;
    wxButton*   m_pButtonRankTotal;
    wxButton*   m_pButtonRankPrev;
    bool        m_bDataChanged = false;
};

#endif
