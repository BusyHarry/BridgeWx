// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/msgdlg.h>

#include "cfg.h"
#include "score.h"
#include "names.h"
#include "fileIo.h"

static vvScoreData svGameSetData;

namespace score
{

static constexpr int    CURRENT_TYPE    = 1;        // version of datastorage

UINT GetNumberOfGamesPlayedByGlobalPair(UINT a_globalPairnr)
{
    ReadScoresFromDisk();
    UINT sessionPair = names::PairnrGlobal2SessionPairnr(a_globalPairnr);
    UINT count = 0;
    for (const auto& itGame : svGameSetData)
    {
        for (const auto& itSet : itGame)
        {
            if ( itSet.pairNS == sessionPair || itSet.pairEW == sessionPair)
            {
                ++count;
                break;
            }
        }
    }
    return count;
}   // GetNumberOfGamesPlayedByGlobalPair()

bool WriteSessionRank(const std::vector<unsigned int>& a_vSessionRank)
{
    return io::SessionRankWrite(a_vSessionRank, cfg::GetActiveSession());
}   // WriteSessionRank()

bool GetSessionRankPrevious(std::vector<unsigned int>& a_vPreviousRank)
{
    return io::SessionRankRead(a_vPreviousRank, cfg::GetActiveSession()-1);
}   // GetSessionRankPrevious()

bool WriteTotalRank(const std::vector<unsigned int>& a_vTotalRank)
{
    return io::TotalRankWrite(a_vTotalRank, cfg::GetActiveSession());
}   // WriteTotalRank()

bool GetSessionRankTotalPrevious(std::vector<unsigned int>& a_vPreviousRank)
{
    return io::TotalRankRead(a_vPreviousRank, cfg::GetActiveSession()-1);
}   // GetSessionRankTotalPrevious()

const vvScoreData* GetScoreData()
{
    ReadScoresFromDisk();
    return &svGameSetData;
}   // GetScoreData()

void SetScoreData(const vvScoreData& newData )
{
    if (newData != svGameSetData)
    {
        svGameSetData = newData;
        WriteScoresToDisk();
        (void)ConfigChanged(true);
    }
}   // SetScoreData()

void ReadScoresFromDisk()
{
    if (!ConfigChanged()) return;
    io::ScoresRead(svGameSetData, cfg::GetActiveSession());
}   // ReadScoresFromDisk()

void WriteScoresToDisk()
{   // for compatability with the original programm:
    // sort all sets for each game on increasing NS player
    for (auto& gamesets : svGameSetData)
    {   // we need a reference, else sorting is done on a copy of the data
        // which is thrown away after sorting...
        std::sort(gamesets.begin(), gamesets.end(), [](const auto& left, const auto& right)
                    {
                        return left.pairNS < right.pairNS;
                    }
                 );
    }
    io::ScoresWrite(svGameSetData, cfg::GetActiveSession());
}   // WriteScoresToDisk()

UINT GetNumberOfGames(const vvScoreData* a_scoreData)
{
    const vvScoreData& scores = a_scoreData == nullptr ? svGameSetData : *a_scoreData;
    UINT count = scores.size();
    if (count == 0) return 0;
    for (--count; count; --count)
    {
        if (scores[count].size()) break;
    }

    return count;
}   // GetNumberOfGames()

bool IsReal(int a_score)
{   // remark: this is NOT a test if its a valid score, only the possible catagory is determined!
    return (a_score < MAX_REAL) && (a_score != SCORE_NP);  // above MAXREAL we have % scores or 'normal' adjusted scores
}   // IsReal()

bool IsProcent(int a_score)
{
    return (a_score >= OFFSET_PROCENT) &&  (a_score <= OFFSET_PROCENT + 100);
}   // IsProcent()

wxString ScoreToString(int a_score)
{
    wxString result;

    if (a_score == SCORE_NONE)
    {                 
        return result;              // empty if no score
    }

    if (a_score == SCORE_NP)
    {                 
        return _("NP");                // not played
    }

    if (abs(a_score) > MAX_REAL)
    {   // filter special cases
        if ((a_score >= OFFSET_REAL - MAX_REAL) && (a_score <= OFFSET_REAL + MAX_REAL))
        {
            a_score -= OFFSET_REAL;     // a real adjusted score
            result = 'R';
        }
        else if (score::IsProcent(a_score))
        {
            a_score -= OFFSET_PROCENT;  // an adjusted % score
            result = '%';
        }
        else
            result = "???";
    }

    return result + FMT("%i", a_score);
}   // ScoreToString()

#include <wx/wxcrtvararg.h>
int ScoreFromString(const wxString& a_score)
{
    wxString tmp = a_score;
    tmp.Trim(TRIM_RIGHT);
    tmp.Trim(TRIM_LEFT);
    tmp.MakeUpper();

    if (tmp.IsEmpty()) return SCORE_NONE;
    int score = 0;
    UINT uScore;
    wxChar chr;
#define BAD_SCORE 1 /* returned value, if input somehow not ok*/

#define FORCE_INRANGE(x) std::clamp((x), -MAX_REAL+1, MAX_REAL-1) /* force 'real' score to be (absolute) less then MAX_REAL*/
    if ( 1 == wxSscanf(tmp," %i %c"  , &score , &chr)) return FORCE_INRANGE(score);                     // 'normal' score
    if ( 1 == wxSscanf(tmp," %%%u %c", &uScore, &chr)) return std::min(uScore,101U)+OFFSET_PROCENT;     // % score
    if ( 1 == wxSscanf(tmp," R%i %c" , &score , &chr)) return FORCE_INRANGE(score)+OFFSET_REAL;         // 'real' aribitrary score
    if ( (tmp == _("NP")) || (tmp == ("NP")) )  // remark: one fixed "NP" and one translatable
        return SCORE_NP;
    return BAD_SCORE;   // bad score, rangecheck will get it
}   // ScoreFromString()

char VulnerableChar(UINT a_game, bool a_bNS)
{
    static const char nsInfo[] = " * ** *  * ** * ";    /* 2,4,5,7,10,12,13,15 */
    static const char ewInfo[] = "  ** ** **  *  *";    /* 3,4,6,7,9,10,13,16  */

    a_game -= 1;        // games from 1 to N
    a_game &= 0x0f;     // vulnerability is equal each 16 games
    return a_bNS ? nsInfo[a_game] : ewInfo[a_game];
}   // VulnerableChar()

bool IsVulnerable(UINT a_game, bool a_bNS)
{
    return '*' == VulnerableChar(a_game, a_bNS) ;
}   // IsVulnerable()


static const int scoresNormalVulnerableNo[]=
{   // generated by maakscr3.bas, LevelsHjj, 03-26-2000, 14:45:29

    // data for non-vulnerable non-doubled results, max 3 down
    //->special    -650, -600, -550, -500, -450, -400, -350, -300, -250, -200,
    -150, -100,  -50,    0,   70,   80,   90,  110,  120,  130,
    140,  150,  170,  180,  190,  200,  210,  230,  240,  260,
    270,  400,  420,  430,  440,  450,  460,  480,  490,  510,
    520,  920,  940,  980,  990, 1010, 1020, 1440, 1510, 1520
};

static const int scoresNormalVulnerableYes[]=
{   // , max 3 down
    // ->special    -1300,-1200,-1100,-1000, -900, -800, -700, -600, -500, -400,
    -300, -200, -100,    0,   70,   80,   90,  110,  120,  130,
    140,  150,  170,  180,  190,  200,  210,  230,  240,  260,
    270,  600,  620,  630,  640,  650,  660,  680,  690,  710,
    720, 1370, 1390, 1430, 1440, 1460, 1470, 2140, 2210, 2220
};

static const int scoresSpecialVulnerableNo[]=
{
    // data for non-vulnerable (re)doubled results, or >= 4 down
    -7000,-6400,-5800,-5200,-4600,-4000,-3500,-3400,-3200,-2900,
    -2800,-2600,-2300,-2200,-2000,-1700,-1600,-1400,-1100,-1000,
    -800, -650, -600, -550, -500, -450, -400, -350, -300, -250,
    -200, -100,    0,  140,  160,  180,
    230,  240,  260,  280,  340,  360,  380,  430,  440,  460,
    470,  480,  490,  510,  520,  530,  540,  550,  560,  570,
    580,  590,  610,  630,  640,  650,  660,  670,  680,  690,
    710,  720,  730,  740,  750,  760,  770,  780,  790,  800,
    810,  830,  840,  850,  870,  880,  890,  910,  920,  930,
    950,  960,  970,  990, 1000, 1030, 1040, 1080, 1090, 1120,
    1160, 1190, 1200, 1210, 1230, 1240, 1280, 1310, 1320, 1330,
    1360, 1380, 1400, 1430, 1440, 1480, 1520, 1560, 1580, 1600,
    1620, 1630, 1640, 1660, 1680, 1720, 1760, 1770, 1790, 1820,
    1860, 1960, 2240, 2280
};

static const int scoresSpecialVulnerableYes[]=
{
    // data for vulnerable (re)doubled results, or >= 4 down
    -7600,-7000,-6400,-5800,-5200,-4600,-4000,-3800,-3500,-3400,
    -3200,-2900,-2800,-2600,-2300,-2200,-2000,-1700,-1600,-1400,
    -1300,-1200,-1100,-1000, -900, -800, -700, -600, -500, -400,
    -200,    0,  140,  160,  180,
    230,  340,  360,  380,  540,  560,  580,  630,  670,  690,
    710,  720,  730,  740,  750,  760,  780,  790,  810,  840,
    850,  870,  880,  890,  910,  920,  930,  940,  950,  960,
    980,  990, 1000, 1010, 1030, 1050, 1070, 1080, 1090, 1110,
    1120, 1130, 1140, 1150, 1160, 1180, 1190, 1200, 1210, 1240,
    1250, 1270, 1280, 1290, 1310, 1320, 1330, 1340, 1350, 1360,
    1380, 1390, 1400, 1410, 1430, 1470, 1480, 1490, 1520, 1530,
    1540, 1550, 1560, 1600, 1640, 1660, 1670, 1680, 1690, 1720,
    1740, 1760, 1800, 1830, 1860, 1880, 1920, 1960, 2000, 2040,
    2070, 2080, 2110, 2120, 2160, 2200, 2230, 2280, 2320, 2330,
    2360, 2440, 2470, 2480, 2490, 2510, 2560, 2600, 2630, 2660,
    2720, 2760, 2840, 2880, 2940, 2980, 3120, 3160
};

static ScoreValidation FindScore(int a_score, bool a_bVulnerable, bool a_bSpecial)
{
#define elements(x) sizeof(x)/sizeof(x[0])
    static struct {const int* table; const size_t count;} const scoreInfo[4]=
    {
          {scoresNormalVulnerableNo,   elements(scoresNormalVulnerableNo  )}
        , {scoresNormalVulnerableYes,  elements(scoresNormalVulnerableYes )}
        , {scoresSpecialVulnerableNo,  elements(scoresSpecialVulnerableNo )}
        , {scoresSpecialVulnerableYes, elements(scoresSpecialVulnerableYes)}
    };

    UINT index = 0;
    if (a_bSpecial)    index += 2;
    if (a_bVulnerable) index += 1;

    bool bFound = std::binary_search(scoreInfo[index].table, scoreInfo[index].table + scoreInfo[index].count, a_score);
    if (bFound)
    {
        if (a_bSpecial) return ScoreSpecial;
        return ScoreValid;
    }
    return ScoreInvalid;
}   // FindScore()

ScoreValidation IsScoreValid(int a_score, UINT a_game, bool a_bNS)
{
    if (cfg::IsScriptTesting()) return ScoreValid;    // always ok!
    if (!score::IsReal(a_score))
    {   // not a 'normal' score
        if (score::IsProcent(a_score)) return IsInRange(a_score, OFFSET_PROCENT, OFFSET_PROCENT+100) ? ScoreValid : ScoreInvalid;
        a_score -= OFFSET_REAL; // now we have a 'normal' score, check it below
    }

    auto const SPECIAL{true};
    auto const NORMAL {false};

    ScoreValidation result;
    result = FindScore( a_score, score::IsVulnerable(a_game, a_bNS), NORMAL );      // check normal NS scores first, should happen more often
    if (result != ScoreInvalid) return result;
    result = FindScore(-a_score, score::IsVulnerable(a_game,!a_bNS), NORMAL );      //  NS <->EW
    if (result != ScoreInvalid) return result;
    result = FindScore( a_score, score::IsVulnerable(a_game, a_bNS), SPECIAL);       // check special scores last, should happen less
    if (result != ScoreInvalid) return result;
    return FindScore(-a_score, score::IsVulnerable  (a_game,!a_bNS), SPECIAL);      //  NS <->EW
}   // IsScoreValid()

int Score2Real(int score)
{
    if ((score >= OFFSET_REAL-MAX_REAL) && (score <= OFFSET_REAL+MAX_REAL))
        score -= OFFSET_REAL;      //separation in % and 'real' scores
    return score;
}   // Score2Real()

int Procentscore2Procent(int score)
{
    return score - OFFSET_PROCENT;
}   // Procentscore2Procent()

bool ExistGameData()
{
    for (const auto& gamesData : svGameSetData)
    {   // for all games
        if (gamesData.size())
            return true;
    }

    return false;
}   // ExistGameData()

bool AdjustPairNrs(UINT a_fromPair, int a_delta)
{
    bool bChanged = false;
    for (auto& gamesData : svGameSetData)
    {   // for all games
        for (auto& score : gamesData)
        {   // for all scores
            if (score.pairNS >= a_fromPair)
            {
                score.pairNS += a_delta;
                bChanged = true;
            }
            if (score.pairEW >= a_fromPair)
            {
                score.pairEW += a_delta;
                bChanged = true;
            }
        }
    }

    return bChanged;
}   // AdjustPairnr()

bool DeleteScoresFromPair(UINT a_pair)
{
    bool bDeleted = false;
    for (auto& gamesData : svGameSetData)
    {   // for all games
        for ( auto score = gamesData.begin(); score!= gamesData.end(); ++score)
        {   // for all scores in this game
            if ((score->pairNS == a_pair) || (score->pairEW == a_pair))
            {
                gamesData.erase(score);
                bDeleted = true;
                break;  // pair can only be found once for a certain game...
            }
        }
    }

    return bDeleted;
}   // DeleteScoresFromPair()

/*
* Following are methods to calculate a score from a contract in text format.
*  "-x[*[*]]" or "y'SUIT'[[+|-]x][*[*]]"
*  where 'x' = the number of down/over tricks.
*  where 'y' = the contract bidden
*  where 'SUIT' is the type of the contract (or, when abbreviated, the first match in the card-names)
*  where '*' is doubled, and '**' is redoubled
*/

    struct CardNameAndType
    {
        wxString    name;   // clubs|diamonds|hearts|spades|notrump
        PlayType    type;   // clubdiamonds|heartsSpades|notrump
        CardId      id;     // suit-id of 'name'
    };

    static std::vector<CardNameAndType> svCardNames;            // identification of all suits
    static std::vector<CardNameAndType> svCardNamesLowercase;   // lowercased version of 'svCardNames'
    static std::vector<wxString>        svDoubledTypeNames;     // names of all the 'DoubledType' types
    static std::vector<wxString>        svPlayTypeNames;        // names of all the 'PlayType' types

    void InitTexts4Translation(bool a_bForce)
    {   // needed for text-translations: static text are initialized before the translation is setup
        static bool bInit = false;
        if (bInit && !a_bForce) return;
        bInit = true;

        svCardNames =
        {
              {_("Clubs"   ), PlayType::PtClubsDiamonds, CardId::CiClubs   }
            , {_("Diamonds"), PlayType::PtClubsDiamonds, CardId::CiDiamonds}
            , {_("Hearts"  ), PlayType::PtHeartsSpades , CardId::CiHearts  }
            , {_("Spades"  ), PlayType::PtHeartsSpades , CardId::CiSpades  }
            , {_("NoTrump" ), PlayType::PtNoTrump      , CardId::CiNoTrump }
            , {_("Pass"    ), PlayType::PtPass         , CardId::CiPass    }    // game not played: no player did a bid
            , {  "Z"        , PlayType::PtNoTrump      , CardId::CiNoTrump }    // extra: 'special' shorthand for Dutch language
        };
        svPlayTypeNames      = {_("Clubs/Diamonds"), _("Hearts/Spades"), _("NoTrump")};
        svDoubledTypeNames   = {"", _("doubled"), _("redoubled")};
        svCardNamesLowercase = svCardNames; // need lowercase for matching contracts
        for (auto& str : svCardNamesLowercase) { str.name.MakeLower(); }
    }   // InitTexts4Translation()

    static const int siDoubleFactor [] = {  1,  2,   4 }; // multiplication for specific 'doubledType', indexed with 'DoubledType'
    static const int siInsultBonus  [] = {  0, 50, 100 }; // bonus when doubled, indexed with 'DoubledType'
    static const int siPlayTypeValue[] = { 20, 30,  30 }; // trick-value, indexed with 'PlayType'
    static const int siPlayTypeExtra[] = {  0,  0,  10 }; // extra score for first trick of specific 'PlayType', indexed with 'PlayType'
    static const int siDoubledNotVulnerable[14] =
    {   // scores for down, doubled, not-vulnerable, indexed with '-overTricks'
        0, -100, -300, -500, -800,  -1100, -1400, -1700, -2000, -2300, -2600, -2900, -3200, -3500
    };
    static const int siDoubledVulnerable[14] =
    {   // scores for down, doubled, vulnerable, indexed with '-overTricks'
        0, -200, -500, -800, -1100, -1400, -1700, -2000, -2300, -2600, -2900, -3200, -3500, -3800
    };
    static const wxString sQuestionMarks("???");

    wxString GetCardName(CardId id)
    {
        InitTexts4Translation();
        if (id >= svCardNames.size()) return sQuestionMarks;
        return svCardNames[id].name;
    }   // GetCardName()

    wxString GetPlayTypeName(PlayType type)
    {
        InitTexts4Translation();
        if (type >= svPlayTypeNames.size()) return sQuestionMarks;
        return svPlayTypeNames[type];
    }   // GetPlayTypeName

    wxString GetDoubledTypeName(int type)
    {
        InitTexts4Translation();
        if ((size_t)type >= svDoubledTypeNames.size()) return sQuestionMarks;
        return svDoubledTypeNames[(size_t)type];
    }   // GetDoubledTypeName

    struct Contract
    {
        PlayType    type       = PlayType::PtClubsDiamonds;
        CardId      id         = CardId  ::CiClubs;
        int         doubled    = 0;
        int         level      = 0;
        int         overTricks = 0;
    };

    /*
    *  fe: 3 NT with 3 overtricks: type = NoTrump, level = 3, overTricks = 3, doubled = 0
    */
    static int CalculateScore4Contract(const Contract& a_contract, bool a_bVulnerable, wxString& a_errorDescription)
    {   // 1 <= level <= 7, -13 <= overTricks <= 6
        int score;
        int vulnerableFactor = a_bVulnerable ? 2 : 1;
        if (a_contract.overTricks < 0)
        {   // contract down
            if (a_contract.level == 0)
            {   // only '-N' was given as contract
                if (a_contract.overTricks < -13)
                {
                    a_errorDescription = _("more down then contract-tricks??\n");
                    return CONTRACT_NOT_CONSISTENT;
                }
            }
            else
            {   // full contract info, now we can be more precise
                if (!IsInRange(a_contract.level, 1, 7))
                {
                    a_errorDescription = _("more tricks then possible in a game??\n");
                    return CONTRACT_NOT_CONSISTENT;
                }
                if (a_contract.overTricks < -(6 + a_contract.level))
                {
                    a_errorDescription = _("more down then contract-tricks??\n");
                    return CONTRACT_NOT_CONSISTENT;
                }
            }

            if (a_contract.doubled == 0)
                return a_contract.overTricks * vulnerableFactor * 50;
            auto& penalty = a_bVulnerable ? siDoubledVulnerable : siDoubledNotVulnerable;
            score  = penalty[-a_contract.overTricks];
            score *= a_contract.doubled;
            return score;
        }   // contract down
        
        if ( !IsInRange(a_contract.level, 1, 7) || !IsInRange(a_contract.overTricks, 0, 7 - a_contract.level) )
        {
            a_errorDescription = _("more tricks then possible in a game??\n");
            return CONTRACT_NOT_CONSISTENT;
        }
        score  = a_contract.level * siPlayTypeValue[a_contract.type];
        score += siPlayTypeExtra[a_contract.type];
        score *= siDoubleFactor [a_contract.doubled];

        // determine bonus
        if (score < 100)
            score += 50;                                // part score bonus
        else
        {
            score += a_bVulnerable ? 500 : 300;         // game bonus
            if (a_contract.level == 6)
                score += a_bVulnerable ? 750 : 500;     // small slam bonus
            else if (a_contract.level == 7)
                score += a_bVulnerable ? 1500 : 1000;   // grand slam bonus
        }

        score += siInsultBonus[a_contract.doubled];     // insult bonus

        if (a_contract.overTricks > 0)
        {                                               // overtrick bonus
            if (a_contract.doubled == 0)
                score += a_contract.overTricks * siPlayTypeValue[a_contract.type];
            else
                score += a_contract.overTricks * a_contract.doubled * vulnerableFactor * 100;
        }   // overtricks
        return score;
    }   // CalculateScore4Contract()

    static void SkipDigits(const wxChar* &pBuf)       // skip digits
    {
        while (std::isdigit(*pBuf)) ++pBuf;
    }   // SkipDigits()

    static void GetDoubled(const wxChar* &pBuf, int& doubled)   // get all 'doubled' characters
    {
        while ('*' == *pBuf)
        {
            if (doubled < 2) ++doubled; // max == re-doubled!
            ++pBuf;
        }
    }   // GetDoubled()

    static bool ExtractContract(const wxString& a_input, Contract& a_contract)
    {   // convert string representation of a contract to binairy types
        wxString input = a_input.Lower();   // lowercase for easier compare
        input.Replace(' ', "");             // remove all spaces
        const wxChar* pInput = input.c_str();
        if (*pInput == '-')
        {   // only number of down tricks given
            a_contract.overTricks = wxAtoi(pInput);
            if (a_contract.overTricks == 0) return false;
            SkipDigits(++pInput);
            GetDoubled(pInput, a_contract.doubled);
            return true;
        }   // contract down

        if (svCardNamesLowercase[CiPass].name.StartsWith(input))
        {
            a_contract.id = CiPass;
            return true;
        }
        a_contract.level = wxAtoi(pInput);          // determine the bidlevel
        if (0 == a_contract.level) return false;    // can't have a 0 contract
        SkipDigits(pInput);
        // now we expect the suit. Match all the alpha-characters in the input with pre-defined cardnames
        size_t len = 0; wxString expectedChars = "*-+0123456789";
        while (pInput[len] && -1 == expectedChars.Find(pInput[len])) {++len;}   // find first non-name char
        if (len == 0) return false; // no (abbreviated) suit found
        wxString tmp = wxString(pInput).Mid(0, len);
        // find the first (partial) match of a card-name
        const auto it = std::find_if(svCardNamesLowercase.begin(), svCardNamesLowercase.end(), [&tmp](const auto& it){return it.name.StartsWith(tmp);});
        if (it == svCardNamesLowercase.end()) return false;    // no (partial) match found
        a_contract.type = it->type;
        a_contract.id   = it->id;
        pInput += len;
        //now we expect a (signed) number or '*' for double
        GetDoubled(pInput, a_contract.doubled);
        // now there could be a (signed) number
        if (!*pInput) return true;  // end of string, just level and contract given
        if (-1 == expectedChars.Find(*pInput)) return false;
        a_contract.overTricks = wxAtoi(pInput); SkipDigits(++pInput);
        GetDoubled(pInput, a_contract.doubled);
        return true;    // could give error if not eol, but we just ignore rest, if present.....
    }   // ExtractContract()

    int GetContractScoreFromString(const wxString& a_input, bool a_bVulnerable, wxString& a_sResult)
    {
        // next init is only for translating text when we need them (static texts are NOT translated)
        InitTexts4Translation();

        a_sResult.clear();
        Contract contract;

        if (!ExtractContract(a_input, contract))
            return CONTRACT_MALFORMED;  // error in contract description, show usage
        if (contract.id == CiPass)
        {
            a_sResult = svCardNames[CiPass].name;
            return 0;
        }
        int score = CalculateScore4Contract(contract, a_bVulnerable, a_sResult);
        if (score != CONTRACT_NOT_CONSISTENT) // if so, a_sResult contains an error description
        {
            if (contract.overTricks < 0)
            {
                a_sResult = FMT(_("%i down %s"), -contract.overTricks, GetDoubledTypeName(contract.doubled));
            }
            else
            {
                a_sResult = FMT("%i %s %+i %s"
                    , contract.level
                    , GetCardName(contract.id)
                    , contract.overTricks
                    , GetDoubledTypeName(contract.doubled)
                );
            }
            a_sResult.Trim(true);   // remove last space, if present
        }

        return score;
    }   // GetContractScoreFromString()

    wxString GetContractExplanation()
    {
        wxString result = 
          _("-x[*[*]] or y'SUIT'[[+|-]x][*[*]]   calculate the score for:\n"
            "      'x'    = the number of over/under tricks.\n"
            "      'y'    = the level of the contract.\n"
            "      '*'    = doubled, and '**' is redoubled\n"
            "      'SUIT' = the type of the contract (or, when abbreviated, the first match in the suit-names)\n"
          );
            result += FMT("      %s:", _("SUIT"));
            for (int index = CardId::CardIdFirst; index <= CardId::CardIdLast; ++index)
            {   // can't use 'score::CardId' for index: it doesn't go above last typevalue! (I made it so!)
                result += FMT(" '%s' |",  score::GetCardName(static_cast<CardId>(index)));
            }
            result.RemoveLast();    // == '|'
            result += '\n';
        return result;
    }   // GetContractExplanation
}   // end namespace score
