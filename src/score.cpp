// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/msgdlg.h>

#include "cfg.h"
#include "score.h"
#include "names.h"
#include "database.h"
#include "fileIo.h"

static vvScoreData svGameSetData;

namespace score
{

static constexpr int    CURRENT_TYPE    = 1;        // version of datastorage

static void ReadScoresFromDisk();
static void WriteScoresToDisk();

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
    return (a_score < MAX_REAL) && (a_score != SCORE_NP);  // above MAXREAL we have % scores or 'normal' arbitrary scores
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
        return _("NG");                // not played
    }

    if (abs(a_score) > MAX_REAL)
    {   // filter special cases
        if ((a_score >= OFFSET_REAL - MAX_REAL) && (a_score <= OFFSET_REAL + MAX_REAL))
        {
            a_score -= OFFSET_REAL;     // a real arbitrary score
            result = 'R';
        }
        else if (score::IsProcent(a_score))
        {
            a_score -= OFFSET_PROCENT;  // an arbitrary % score
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
    if ( (tmp == _("NG")) || (tmp == _("NP")) )
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

static bool FindScore(int a_score, bool a_bVulnerable, bool a_bSpecial)
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
    if (bFound && a_bSpecial)
    {
        wxString score = FMT(_("onverwachte/bijzondere score: %i\n\ntoch accepteren?"), a_score);
        // ^== first line 'blue' and fontsize 2* larger as second line????
        if ( wxNO == MyMessageBox( score, "???", wxYES_NO | wxICON_INFORMATION))
        {
            bFound = false;
        }
    }

    return bFound;
}   // FindScore()

bool IsScoreValid(int a_score, UINT a_game, bool a_bNS)
{
    if (cfg::IsScriptTesting()) return true;    // always ok!
    if (!score::IsReal(a_score))
    {   // not a 'normal' score
        if (score::IsProcent(a_score)) return true;
        a_score -= OFFSET_REAL; // now we have a 'normal' score, check it below
    }

    if (FindScore( a_score, score::IsVulnerable(a_game, a_bNS), false)) return true;   // check normal scores first, should happen more often
    if (FindScore(-a_score, score::IsVulnerable(a_game,!a_bNS), false)) return true;   //  NS <->EW
    if (FindScore( a_score, score::IsVulnerable(a_game, a_bNS), true )) return true;   // check special scores last, should happen less
    if (FindScore(-a_score, score::IsVulnerable(a_game,!a_bNS), true )) return true;   //  NS <->EW

    return false;
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
}   // end namespace score
