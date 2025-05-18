// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _CALC_SCORE_H_
#define _CALC_SCORE_H_
#pragma once

#include "baseframe.h"

class wxTextCtrl;
class wxStyledTextCtrl;
class wxListView;

struct Total                    // session-results for a player
{
    int  maxScore       = 0;    // in MP for % calculation
    UINT nrOfGames      = 0;    // total played games
    long points         = 0;    // earned MP*10    (x.y)
    long procentScore   = 0;    // score in %*100  (x.yy)
    long butlerMp       = 0;    // butler: total of imps voor all games
    long mpPerGame      = 0;    // butler: imps/game, used for session result (*100, to simulate 2 digits after decimal point)
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
    void        SaveSessionResultsProcent();
    void        SaveSessionResultsButler();
    void        SaveGroupResult         ();
    void        ApplySessionCorrections (); // apply corrections to the session result
    void        CalcGamePercent         (UINT game, bool bNs, FS_INFO& fsInfo);
    void        CalcGameButler          (UINT game, bool bNs);
    void        CalcButlerFkw           (UINT game);
    long        NeubergPoints           (long points, UINT gameCount, UINT comparableCount);
    void        MergeFrqTables          (FS_INFO& ns, const FS_INFO& ew);
    void        SaveFrequencyTable      ();
    void        MakeFrequenceTable      (UINT a_game, std::vector<wxString>& a_stringTable);
    void        SaveSessionResultShort  ();
    void        OnPrint                 (wxCommandEvent& );
    void        OnCalcResultPair        (const wxCommandEvent&);
    void        CalcResultPairHelper    (long& sumPoints, UINT& sumTops, UINT& gamesPlayed, wxString& tmp);

    void        OnCalcResultGame        (const wxCommandEvent&);
    void        ShowChoice              ();
    long        GetSetResult            (UINT pair, UINT firstGame, UINT nrOfGames, UINT* pGamesPlayed = nullptr);  // get earned match-points for wanted games
    static const bool GROUPRESULT_SESSION = true;
    static const bool GROUPRESULT_FINAL   = false;
    wxString    GetGroupResultString    (UINT pair, const std::vector<UINT>* a_pIndex = nullptr, bool bSession = GROUPRESULT_FINAL);    // get groupstring or rank in group "BLYEGR" / " . 1 ."
    bool        FindBadGameData         (); // return true if invalid pairnrs found in gamedata

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
    wxString    m_txtBadGameData;               // info on bad gamedata
    UINT        m_numberOfSessionPairs;         // sum of pairs in all groups

    bool        m_bSomeCorrection;
    bool        m_bButler;
    bool        m_bBadGameData;     // some bad pairnrs in gamedata
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

class FormBuilder
{   // class to create a table with a header and one or more rows of data
public:
    enum class Align{ // alignment in the columns of the header/rows
          LEFT          // fillup with spaces after till wanted size
        , CENTER        // as much spaces before and after till wanted size
        , RIGHT         // as much spaces before till wanted size
        , LEFT_SPACE1   // add one space in front, then LEFT
        , LEFT_SPACE2   // add two spaces in front, then LEFT
        , RIGHT_SPACE1  // add one space after, then RIGHT
        , RIGHT_SPACE2  // add two spaces after, then RIGHT
    };
    static const size_t NO_LIMIT = ((size_t)-1);    // don't apply alignment for this column (mostly the last column)
    static const char SEPERATOR = ' ';              // separator between columns
    struct ColumnInfoHeader
    {
        Align       align;  // Can be different from row-alignment
        wxString    extra;  // append this string to the constructed string
        wxString    header; // column label
    };

    struct ColumnInfoRow
    {
        size_t      size;   // size of this column            , also used for the header
        Align       align;  // alignment for the data-columns
        wxString    extra;  // append this string to the constructed string
        bool        active; // if not set, ignore this column , also used for the header
    };

    explicit FormBuilder(const std::vector<ColumnInfoRow>& a_rowInfo) : m_rowInfo(a_rowInfo) {}
    ~FormBuilder() {}

    wxString        CreateHeader(const std::vector<ColumnInfoHeader>& headerInfo);
    wxString        CreateRow   (const std::vector<wxString>& columsContent);
    static wxString CreateColumn(const wxString& input, size_t len, Align align);

private:
    const std::vector<ColumnInfoRow>& m_rowInfo;  // reference, so one can apply runtime updates
};  // FormBuilder


#endif
