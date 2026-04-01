// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _CORRECTIONSEND_H_
#define _CORRECTIONSEND_H_
#pragma once

#include "baseframe.h"

class MyGrid;
class wxWindow;

class CorrectionsEnd : public Baseframe
{
public:
    explicit    CorrectionsEnd (wxWindow* pParent, UINT pageId);
               ~CorrectionsEnd () override = default;
    void        RefreshInfo() final;    // (re)populate the grid
    void        PrintPage()   final;

    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

protected:
    void        OnOk            () final;
    void        OnCancel        () final;
    void        DoSearch        (wxString&       string  ) final;
    bool        OnCellChanging  (const CellInfo& cellInfo) final;
    void        BackupData      () final;   // called if active panel is about to be hidden. You may save changed data!

private:
    MyGrid*     m_theGrid;
    bool        m_bDataChanged = false;     // 'something' changed in grid

//    ;score.2 pair bonus.2 games   pairname
//    100.00     1  11.00   s16     xxx-xxx
};

#endif
