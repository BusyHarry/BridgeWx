// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _CALC_SCORE_H_
#define _CALC_SCORE_H_
#pragma once

#include "baseframe.h"

class wxTextCtrl;
class wxStyledTextCtrl;
class wxListView;

struct Total
{
    int maxScore        = 0;
    UINT nrOfGames      = 0;
    long points         = 0;
    long score          = 0;
};

class CalcScore: public Baseframe
{
public:
    explicit    CalcScore(wxWindow* pParent, UINT pageId);
               ~CalcScore() override;
    void        RefreshInfo() override final;   // (re)populate the grid
    void        PrintPage()   override final;   // print grid/listbox
    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

    typedef struct FrequencyState
    {
        int     score           = 0;
        UINT    nrOfEqualScores = 0;
        long    points          = 0;
        long    pointsEW        = 0;
    } FrequencyState;

    typedef std::vector<FrequencyState> FS_INFO;
    typedef std::vector<FS_INFO> FS;

protected:
    virtual void BackupData     () override final;
    virtual void DoSearch       (wxString&) override final;                    // handler for 'any' search in derived class

private:
    void        CalcSession             ();     // results for current session
    void        CalcTotal               ();     // sum of results for all sesssions upto current session
    void        CalcClub                (bool a_bTotal);     // results for clubs, if pairs have assigned clubs(total or session)
    void        InitializeAndCalcScores ();
    void        SaveSessionResults      ();
    void        SaveGroupResult         ();
    void        ApplySessionCorrections (); // apply corrections to the session result
    void        CalcGame                (UINT game, bool bNs, FS_INFO& fsInfo);
    long        NeubergPoints           (long points, UINT gameCount, UINT comparableCount);
    void        MergeFrqTables          (FS_INFO& ns, const FS_INFO& ew);
    int         ScoreEwToNs             (int score);    // convert ew-score to ns-score
    void        SaveFrequencyTable      ();
    void        MakeFrequenceTable      (UINT a_game, std::vector<wxString>& a_stringTable);
    void        SaveSessionResultShort  ();
    void        OnPrint                 (wxCommandEvent& );
    void        OnCalcResultPair        (const wxCommandEvent&);
    void        OnCalcResultGame        (const wxCommandEvent&);
    void        ShowChoice              ();
    long        GetSetResult            (UINT pair, UINT firstGame, UINT nrOfGames);  // get earned match-points for wanted games
    wxString    GetGroupResultString    (UINT pair, const std::vector<UINT>* a_pIndex = nullptr, bool bSession = false);    // get groupstring or rank in group "BLYEGR" / " . 1 ."

//    wxTextCtrl* m_pTextBox;
    wxListView* m_pListBox;
    bool        m_bDataChanged;     // 'something' changed
    MyTextFile  m_txtFileResultSession;
    MyTextFile  m_txtFileResultOnName;
    MyTextFile  m_txtFileFrqTable;
    MyTextFile  m_txtFileResultGroup;
    MyTextFile  m_txtFileResultTotal;
    MyTextFile  m_txtFileResultClubTotal;
    MyTextFile  m_txtFileResultClubSession;
    MyTextFile  m_txtFileResultPair;            // result for a pair
    MyTextFile  m_txtFileResultGame;            // result for a game
    bool        m_bSomeCorrection;
    UINT        m_maxPair;          // highest pairnr played in this session
    UINT        m_maxGame;          // highest gamenr
    long        m_findPos;          // start  searching in listbox from this line
    MyChoiceMC* m_pPairSelect;      // pair-selection for result of a specific pair
    MyChoiceMC* m_pGameSelect;      // game-selection for result of a specific game

    enum Choices
    {
          ResultSession     = 0
        , ResultSessionName = 1
        , ResultTotal       = 2
        , ResultFrqTable    = 3
        , ResultGroup       = 4
        , ResultClubSession = 5
        , ResultClubTotal   = 6
        , ResultPair        = 7 // in separate choicebox
        , ResultGame        = 8 // in separate choicebox
    };
    std::vector<Choices>    m_vChoices;    // items in choice selector
    MY_CHOICE*              m_pChoices;
    int                     m_choiceResult;
//    wxStyledTextCtrl* m_pStyledTextBox;
};

#endif
