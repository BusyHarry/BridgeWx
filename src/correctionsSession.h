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
               ~CorrectionsSession() override;
    void        RefreshInfo() override final;   // (re)populate the grid
    void        PrintPage()   override final;
    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

protected:
    void        OnOk            () override final;
    void        OnCancel        () override final;
    void        DoSearch        (wxString&       string  ) override final;
    bool        OnCellChanging  (const CellInfo& cellInfo) override final;
    virtual void BackupData     () override final;  // called if active panel is about to be hidden. You may save changed data!

private:

    MyGrid*     m_theGrid;
    bool        m_bDataChanged;     // 'something' changed in grid

    enum
    {
          COL_ZERO = 0                      // label, first column
        , COL_PAIRNAME_SESSION = COL_ZERO   // session pairname
        , COL_PAIRNAME_GLOBAL               // global pairname
        , COL_COR_PROCENT                   // correction in %
        , COL_COR_MP                        // correctionin MP
        , COL_COR_MAX                       // maximum correction ....
        , COL_COR_EXTRA                     // extra corrrection.....
        , COL_NR_OF                         // nr of columns in this grid
    };
};

#endif
