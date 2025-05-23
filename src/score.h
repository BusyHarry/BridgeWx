﻿// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SCORE_H_
#define _SCORE_H_
#pragma once

namespace score
{
    #define MAX_REAL        8000    /* for 'normal' score                                       */
    #define OFFSET_PROCENT  9000    /* adjusted score in %                                      */
    #define OFFSET_REAL     20000   /* adjusted score asif it was 'normal'                      */
    #define NS              0       /* index in data for N/S pair                               */
    #define EW              1       /* index in data for E/W pair                               */
    #define SCORE_NONE      -1      /* empty score                                              */
    #define SCORE_NP        -2      /* N(ot)P(layed) score: game not played, perhaps no time... */
    #define SCORE_IGNORE    1000000L    /* this value in endcorrrections: just ignore if only bonus present */
    #define SCORE_NO_TOTAL  2000000L    /* this value in endcorrrections: sessionScore NOT added to total score*/

    struct GameSetData
    {
        UINT        pairNS;
        UINT        pairEW;
        int         scoreNS;
        int         scoreEW;
        wxString    contractNS;
        wxString    contractEW;
        bool operator < (const GameSetData &rhs) const
        {   /* sorting on NS */ return pairNS < rhs.pairNS; }
        bool operator == (const GameSetData &rhs) const
        {return rhs.pairNS == pairNS && rhs.pairEW == pairEW && rhs.scoreNS == scoreNS && rhs.scoreEW == scoreEW;}
    };

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
    void                ReadScoresFromDisk();
    void                WriteScoresToDisk();
    const vvScoreData*  GetScoreData();                     // get a ptr to the current scores
    void                SetScoreData(const vvScoreData&);   // update the scores (write to disk)
    UINT                GetNumberOfGames(const vvScoreData* a_scoreData = nullptr);                     // highest gamenr that is played in a session
    bool                IsReal(int score);            // checks if score is real (not adjusted)
    bool                IsProcent(int score);            // checks if its a '%' score
    wxString            ScoreToString(int score);            // get string representation of score
    int                 ScoreFromString(const wxString& score);// convert string to (possible) score
    bool                IsVulnerable(UINT game, bool bNS);  // checks for vulnerability
    char                VulnerableChar(UINT game, bool bNS);  // returns '*' if vulnerable else ' '
    ScoreValidation     IsScoreValid(int score, UINT game, bool bNS);   // checks if a score is valid
    int                 Score2Real(int score);            // make adjusted real score into real score
    int                 Procentscore2Procent(int score);            // convert aribitrairy score to 0<=value<=100
    UINT                GetNumberOfGamesPlayedByGlobalPair(UINT globalPairnr);
    bool                ExistGameData();                     // true, if there is atleast one score entered
    bool                AdjustPairNrs(UINT fromPair, int delta); // adjust all pairnrs in the scoredata starting from 'frompair' with 'delta', return true if one or more changes
    bool                DeleteScoresFromPair(UINT sessionPair);     // delete all scores for 'pair', return true if anything deleted

    // next some enums and methods to handle contracts in text
    enum PlayType :int
    {
          PlayTypeFirst   = 0
        , PtClubsDiamonds = PlayTypeFirst
        , PtHeartsSpades  = 1
        , PtNoTrump       = 2
        , PtPass          = 3
        , PlayTypeLast    = PtPass
    };

    enum CardId :int
    {
          CardIdFirst   = 0
        , CiClubs       = CardIdFirst
        , CiDiamonds    = 1
        , CiHearts      = 2
        , CiSpades      = 3
        , CiNoTrump     = 4
        , CiPass        = 5
        , CardIdLast    = CiPass
    };

    /*
    *  Get the score for the contract in 'input': "-n[*[*]]"  or "n'SUITE'[[+|-]n][*[*]]"
    *  return value:
    *   CONTRACT_NOT_CONSISTENT = error in calculation: not all params are consistent (like 7clubs+1, or 1club-8)
    *   CONTRACT_MALFORMED      = malformed contract: can't interprete the input, show usage
    *  rest = correct score
    */
    #define CONTRACT_MALFORMED         -1
    #define CONTRACT_NOT_CONSISTENT    -2
    int                 GetContractScoreFromString(const wxString& input, bool bVulnerable, wxString& result);
    wxString            GetDoubledTypeName        (int         type           );
    wxString            GetPlayTypeName           (PlayType    type           );
    wxString            GetCardName               (CardId      id             );    // name of the suit
    void                InitTexts4Translation     (bool        bForce = false );    // init static texts for translation
    wxString            GetContractExplanation    ();                               // how to build a contract-string

} // end of namespace score
#endif
