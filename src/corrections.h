// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _CORRECTIONS_H
#define _CORRECTIONS_H
#pragma once
#include <map>
#include "fdp.h"
class wxString;

namespace cor
{

constexpr auto MIN_CORRECTION = (-500);
constexpr auto MAX_CORRECTION = (+500);
constexpr auto BONUS_MINIMUM  = (-9999);
constexpr auto BONUS_MAXIMUM  = (+9999);

    using CORRECTION_SESSION = struct CORRECTION_SESSION
    {
        CORRECTION_SESSION(char a_type, int a_correction, Fdp a_extra, int a_maxExtra, UINT a_games)
          : type        (a_type)
          , correction  (a_correction)
          , extra       (a_extra)
          , maxExtra    (a_maxExtra)
          , games       (a_games)
        {}
        CORRECTION_SESSION() = default;
        bool operator == (const CORRECTION_SESSION& rhs) const = default;
        bool operator != (const CORRECTION_SESSION& rhs) const {return !(*this == rhs);}
        char    type        = '%';
        int     correction  = 0;
        Fdp     extra       = 0;        //  xx.x
        int     maxExtra    = 0;
        UINT    games       = 0;        // only valid for extra/maxExtra (i.e combi-table)
    };

    using CORRECTION_END = struct CORRECTION_END
    {
        bool operator == (const CORRECTION_END& rhs) const = default;
        Fdp             score;      // xx.xx
        Fdp             bonus;      // xx.xx
        unsigned int    games=0;
    };

using mCorrectionsSession = std::map<unsigned int, cor::CORRECTION_SESSION>;
using mCorrectionsEnd     = std::map<unsigned int, cor::CORRECTION_END>;

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
