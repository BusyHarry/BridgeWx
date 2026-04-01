// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _CORRECTIONSSESSION_H_
#define _CORRECTIONSSESSION_H_
#pragma once

#include "baseframe.h"

class MyGrid;
class wxWindow;

class CorrectionsSession : public Baseframe
{
public:
    explicit    CorrectionsSession (wxWindow* pParent, UINT pageId);
               ~CorrectionsSession() override = default;
    void        RefreshInfo() final;   // (re)populate the grid
    void        PrintPage()   final;
    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

protected:
    void        OnOk            () final;
    void        OnCancel        () final;
    void        DoSearch        (wxString&       string  ) final;
    bool        OnCellChanging  (const CellInfo& cellInfo) final;
    void        BackupData      () final;   // called if active panel is about to be hidden. You may save changed data!
    void        InitButlerProcent();    // init columns for butler or percentage calculation
    void        ClearCombiData  (int row);
private:

    MyGrid*     m_theGrid;
    bool        m_bButler;              // true, if columns are setup for butler calculation
    bool        m_bDataChanged = false; // 'something' changed in grid
};

#endif
