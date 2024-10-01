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
    void        RefreshInfo() override final;   // (re)populate the grid
    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

protected:
    void        OnOk            () override final;
    void        OnCancel        () override final;
    void        DoSearch        (wxString&      string) override final;

    bool        OnCellChanging  (const CellInfo& cellInfo) override final;
    virtual void BackupData   () override final;
private:
    void        UpdateColumnAssign (std::vector<unsigned int>& newAssign);
    void        OnOriginal      (wxCommandEvent& event);
    void        OnTotalRank     (wxCommandEvent& event);
    void        OnPreviousRank  (wxCommandEvent& event);
    void        OnClear         (wxCommandEvent& event);
    void        PrintPage       () override final;


    MyGrid*     m_theGrid;
    wxButton*   m_pButtonRankTotal;
    wxButton*   m_pButtonRankPrev;
    bool        m_bDataChanged;

    enum
    {
          COL_ZERO = 0                      // label, here just the number of pairname
        , COL_PAIRNAME = COL_ZERO           // the (global) pairname
        , COL_PAIRNR_SESSION                // pairnr for this session
        , COL_PAIRNR_SESSION_PREV           // pairnr of previous session for this pair
        , COL_RANK_TOTAL_PREV               // rank in total result for all sessions up to now
        , COL_RANK_SESSION_PREV             // rank in previous session
        , COL_NR_OF                         // nr of columns in this grid
    };
};

#endif
