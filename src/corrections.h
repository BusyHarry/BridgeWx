// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _CORRECTIONS_H
#define _CORRECTIONS_H
#pragma once
#include <map>

class wxString;

namespace cor
{

#define MIN_CORRECTION      (-500)
#define MAX_CORRECTION      (+500)
#define BONUS_MINIMUM       (-9999)
#define BONUS_MAXIMUM       (+9999)

    typedef struct CORRECTION_SESSION
    {
        CORRECTION_SESSION(char a_type, int a_correction, long a_extra, int a_maxExtra, UINT a_games)
        {type = a_type;correction = a_correction;extra = a_extra; maxExtra = a_maxExtra;games=a_games;}
        CORRECTION_SESSION() {type = '%'; correction = 0; extra = 0; maxExtra = 0; games = 0;}
        bool operator == (const CORRECTION_SESSION& rhs) const
        { return type == rhs.type && correction == rhs.correction && games == rhs.games && maxExtra == rhs.maxExtra && extra == rhs.extra;}
        bool operator != (const CORRECTION_SESSION& rhs) const {return !(*this == rhs);}
        char    type;
        int     correction;
        long    extra;          //  long.1:  xx.x
        int     maxExtra;
        UINT    games;        // only valid for extra/maxExtra (i.e combi-table)
    } CORRECTION_SESSION;

    typedef struct CORRECTION_END
    {
        bool operator == (const CORRECTION_END& rhs) const
        { return score == rhs.score && bonus == rhs.bonus && games == rhs.games;}
        long            score=0;  // long.2:  xx.xx
        long            bonus=0;  // long.2:  xx.xx
        unsigned int    games=0;
    } CORRECTION_END;

typedef std::map<unsigned int, cor::CORRECTION_SESSION> mCorrectionsSession;
typedef std::map<unsigned int, cor::CORRECTION_END> mCorrectionsEnd;

const mCorrectionsSession*  GetCorrectionsSession();
const mCorrectionsEnd*      GetCorrectionsEnd    ();

void SetCorrectionsSession(const mCorrectionsSession* pCorSession);
void SetCorrectionsEnd    (const mCorrectionsEnd*     pCorEnd    );

void InitializeCorrections();   // load all corrections if config changed
void SaveCorrections();         // save changes when leaving corrections-editing

bool GetSessionResult           (UINT session, mCorrectionsEnd& sessionResult);        // for total result
bool IsValidCorrectionSession   (UINT sessionPair, CORRECTION_SESSION& correctionSession, const wxString& input, bool bHasError);
bool IsValidCorrectionEnd       (UINT globalPair , const CORRECTION_END& correctionEnd  , const wxString& input, bool bHasError);

}   // namespace cor
#endif
