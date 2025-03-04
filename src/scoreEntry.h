// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SCOREENTRY_H_
#define _SCOREENTRY_H_
#pragma once

#include "baseframe.h"

class MyGrid;
class wxTextCtrl;
class wxRadioBox;
class ChoiceMC;

class ScoreEntry : public Baseframe
{
public:
    explicit    ScoreEntry (wxWindow* pParent, UINT pageId);
               ~ScoreEntry () override;
    void        RefreshInfo() override final;   // (re)populate the grid
    void        PrintPage()   override final;   // print grid
    void        AutotestRequestMousePositions(MyTextFile* a_pFile) final;  // create needed mousepositions

protected:
    void        OnOk            () override final;
    void        OnCancel        () override final;
    void        DoSearch        (wxString&      string) override final;

    bool        OnCellChanging  (const CellInfo& cellInfo) override final;
    virtual void BackupData     () override final;  // called if active panel is about to be hidden. You may save changed data!

private:
    void        OnRbSlipGame        (wxCommandEvent& event);    // radio box: order = slip or game
    void        OnRbGameNS          (wxCommandEvent& event);    // radio box: if slip, then order is gamenr or ns
    void        UpdateCell          (wxCommandEvent& event);
    void        InitializeScores    ();
    void        WriteScoresToDisk   ();
    void        GotoNextEmptyScore  ();
    bool        FindEmptyScore      ();
    void        SaveRowData         ();                  // store grid-data in score-table
    void        OnNextEmptyScore    (wxCommandEvent&);
    void        OnSelectCell        (wxGridEvent& event);
    void        OnSelectRound       (wxCommandEvent&);
    void        OnSelectGame        (wxCommandEvent&);
    void        OnSwitchNsEw        (wxCommandEvent&);

    MyGrid*     m_theGrid;
    wxRadioBox* m_pRadioBoxSlipGame;// select input order based on slip (sets) or game number
    wxRadioBox* m_pRadioBoxGameNs;  // if slip order: select input order based on game number or pair number
    bool        m_bSlipOrder;       // score entry display based on slips (false: base on gamenr)
    bool        m_bGameOrder;       // if slips, then order is on gamenr, else on NS-pairnr
    bool        m_bDataChanged;     // 'something' changed in grid
    int         m_startrow4EmptyScore;  // start from here to find next empty score
    int         m_iRowToSave;       // this row will have its score saved
    MY_CHOICE*  m_pChoiceRound;     // choice box for selecting the round for score-entry
    MyChoiceMC* m_pChoiceGame;      // choice box for selecting the game for score-entry

    UINT        m_uActiveRound;     // the choosen round
    UINT        m_uActiveGame;      // the choosen game nr
    wxBoxSizer* m_pSizerAllChoices; // for choices: slip/ns/round/game
    bool        m_bCancelInProgress;// true, if cancel is wanted

    enum
    {
          COL_ZERO = 0                      // label, first column
        , COL_GAME = COL_ZERO               // game nr
        , COL_NS                            // NS session pair nr
        , COL_EW                            // EW session pair nr
        , COL_SCORE_NS                      // score NS
        , COL_SCORE_EW                      // score EW (only if adjusted score)
        , COL_NAME_NS                       // pairname NS
        , COL_NAME_EW                       // pairname EW
        , COL_NR_OF                         // nr of columns in this grid
    };
};

#endif
