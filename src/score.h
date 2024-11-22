// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SCORE_H_
#define _SCORE_H_
#pragma once

namespace score
{
    #define MAX_REAL        8000    /* for 'normal' score                                       */
    #define OFFSET_PROCENT  9000    /* arbitrary score in %                                     */
    #define OFFSET_REAL     20000   /* arbitrary score asif it was 'normal'                     */
    #define NS              0       /* index in data for N/S pair                               */
    #define EW              1       /* index in data for E/W pair                               */
    #define SCORE_NONE      -1      /* empty score                                              */
    #define SCORE_NP        -2      /* N(ot)P(layed) score: game not played, perhaps no time... */
    #define SCORE_IGNORE    1000000L    /* this value in endcorrrentions (10000.00%) means just ignore*/

    #ifdef _WIN32           // for vs 32/64bit we need 1 byte packing to read/write 'old' data
    #pragma pack(1)
    #endif

    /*
    Layout of score-file:
    Scores[cfg::MAX_PAIRS/2+1]  -> entry[0] = DESCRIPTION, rest is zero
    N*Scores[x]                 -> N=games with data, entry[0]=SetCount, rest: Scores[nrOfSets] of GameSetData
    */

    struct GameSetData
    {
        UINT8   pairNS;
        UINT8   pairEW;
        INT16   scoreNS;
        INT16   scoreEW;
        bool operator < (const GameSetData &rhs) const
        {   /* sorting on NS */ return pairNS < rhs.pairNS; }
        bool operator == (const GameSetData &rhs) const
        {return rhs.pairNS == pairNS && rhs.pairEW == pairEW && rhs.scoreNS == scoreNS && rhs.scoreEW == scoreEW;}
    };

    struct DESCRIPTION
    {
        INT16   dataType;
        UINT16  nrOfGamesWithData;
        UINT16  restSize; //rest of data AFTER row 0 (== Scores[cfg::MAX_PAIRS/2+1]  )
    };

    struct SetCount
    {
        UINT8   dummy[2];
        UINT8   nrOfSets;
    };

    union Scores
    {
        GameSetData setData;
        DESCRIPTION description;
        SetCount    setCount;
    };

    #ifdef _WIN32
    #pragma pack()
    #endif

    bool WriteSessionRank           (const std::vector<unsigned int>& a_vSessionRank);
    bool WriteTotalRank             (const std::vector<unsigned int>& a_vTotalRank);
    bool GetSessionRankTotalPrevious(std::vector<unsigned int>& a_vPreviousTotalRank);
    bool GetSessionRankPrevious     (std::vector<unsigned int>& a_vPreviousRank);
}   // end of namespace score

//now all used identifiers are known: set a typedef
typedef std::vector< std::vector<score::GameSetData> > vvScoreData; // vv=vector[games] of vector[sets]

namespace score
{
    enum ScoreValidation
    {
          ScoreValid    // a valid possible score
        , ScoreInvalid  // incorrect score
        , ScoreSpecial  // possible score, but highly unexpected like 6 down
    };
    const vvScoreData*  GetScoreData        ();                     // get a ptr to the current scores
    void                SetScoreData        (const vvScoreData&);   // update the scores (write to disk)
    UINT                GetNumberOfGames    (const vvScoreData* a_scoreData = nullptr);                     // highest gamenr that is played in a session
    bool                IsReal              (int score);            // checks if score is real (not arbitrairy)
    bool                IsProcent           (int score);            // checks if its a '%' score
    wxString            ScoreToString       (int score);            // get string representation of score
    int                 ScoreFromString     (const wxString& score);// convert string to (possible) score
    bool                IsVulnerable        (UINT game, bool bNS);  // checks for vulnerability
    char                VulnerableChar      (UINT game, bool bNS);  // returns '*' if vulnerable else ' '
    ScoreValidation     IsScoreValid        (int score, UINT game, bool bNS);   // checks if a score is valid
    int                 Score2Real          (int score);            // make arbitrary real score into real score
    int                 Procentscore2Procent(int score);            // convert aribitrairy score to 0<=value<=100
    UINT                GetNumberOfGamesPlayedByGlobalPair(UINT globalPairnr);

}      // end of namespace score
#endif
