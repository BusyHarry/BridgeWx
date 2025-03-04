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
               ~CorrectionsEnd () override;
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

//    ;score.2 pair bonus.2 games   pairname
//    100.00     1  11.00   s16     xxx-xxx

    enum
    {
          COL_ZERO = 0                      // label, first column = numerical session pairname
        , COL_PAIRNAME_SESSION = COL_ZERO   // session pairname
        , COL_PAIRNAME_GLOBAL               // global pairname
        , COL_COR_SCORE                     // score in %
        , COL_COR_BONUS                     // bonus in %
        , COL_COR_GAMES                     // nr of games for these corrections
        , COL_NR_OF                         // nr of columns in this grid
    };
};

#endif
