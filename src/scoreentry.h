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
               ~ScoreEntry () override = default;
    void        RefreshInfo() final;    // (re)populate the grid
    void        PrintPage()   final;    // print grid
    void        AutotestRequestMousePositions(MyTextFile* a_pFile) final;  // create needed mousepositions

protected:
    void        OnOk            () final;
    void        OnCancel        () final;
    void        DoSearch        (wxString&      string) final;

    bool        OnCellChanging  (const CellInfo& cellInfo) final;
    void        BackupData      () final;   // called if active panel is about to be hidden. You may save changed data!

private:
    void        OnRbSlipGame        (const wxCommandEvent& event);  // radio box: order = slip or game
    void        OnRbGameNS          (const wxCommandEvent& event);  // radio box: if slip, then order is gamenr or ns
    void        UpdateCell          (const wxCommandEvent& event);
    void        InitializeScores    ();
    void        WriteScoresToDisk   ();
    void        GotoNextEmptyScore  ();
    bool        FindEmptyScore      ();
    void        SaveRowData         ();                  // store grid-data in score-table
    void        OnNextEmptyScore    (const wxCommandEvent&);
    void        OnSelectCell        (wxGridEvent& event);
    void        OnSelectRound       (const wxCommandEvent&);
    void        OnSelectGame        (const wxCommandEvent&);
    void        OnSwitchNsEw        (const wxCommandEvent&);
    void        OnCheckboxContract  (const wxCommandEvent&);

    static constexpr auto INIT_ROW = -1;            // initial row value for searching empty scores

    MyGrid*     m_theGrid;
    wxRadioBox* m_pRadioBoxSlipGame;                // select input order based on slip (sets) or game number
    wxRadioBox* m_pRadioBoxGameNs;                  // if slip order: select input order based on game number or pair number
    MY_CHOICE*  m_pChoiceRound;                     // choice box for selecting the round for score-entry
    MyChoiceMC* m_pChoiceGame;                      // choice box for selecting the game for score-entry
    wxBoxSizer* m_pSizerAllChoices;                 // for choices: slip/ns/round/game
    wxCheckBox* m_pCheckboxContract;                // if set, columns 'COL_CONTRACT_NS'and 'COL_CONTRACT_EW' are shown
    bool        m_bSlipOrder            = true;     // score entry display based on slips (false: base on gamenr)
    bool        m_bGameOrder            = true;     // if slips, then order is on gamenr, else on NS-pairnr
    bool        m_bDataChanged          = false;    // 'something' changed in grid
    bool        m_bCancelInProgress     = false;    // true, if cancel is wanted
    int         m_startrow4EmptyScore   = INIT_ROW; // start from this row to find next empty score
    UINT        m_uActiveRound          = 1;        // the choosen round
    UINT        m_uActiveGame           = 0;        // the choosen game nr
    int         m_iRowToSave            = 0;        // this row will have its score saved
};

#endif
