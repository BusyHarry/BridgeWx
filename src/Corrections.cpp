// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "cfg.h"
#include "baseframe.h"
#include "names.h"
#include "utils.h"
#include "score.h"
#include "database.h"
#include "fileIo.h"
#include "corrections.h"

namespace cor
{
    static bool CorrectionsSessionRead();
    static bool CorrectionsSessionWrite();
    static bool CorrectionsEndRead();
    static bool CorrectionsEndWrite();


    static bool bCorrectionsSessionChanged = false;
    static bool bCorrectionsEndChanged     = false;

    static mCorrectionsSession  sm_correctionsSession;
    static mCorrectionsEnd      sm_correctionsEnd;

    void SaveCorrections()
    {
        CorrectionsSessionWrite();
        CorrectionsEndWrite();
    }   // SaveCorrections()

    void InitializeCorrections()
    {
        if (!ConfigChanged()) return;
        (void)CorrectionsSessionRead();
        (void)CorrectionsEndRead();
    }   // InitializeCorrections()

    const mCorrectionsSession* GetCorrectionsSession()
    {
        return &sm_correctionsSession;
    }   // GetCorrectionsSession()

    const mCorrectionsEnd* GetCorrectionsEnd()
    {
        return &sm_correctionsEnd;
    }   // GetCorrectionsEnd()

    void SetCorrectionsSession(const mCorrectionsSession* pCorSession)
    {
        sm_correctionsSession = *pCorSession;
        bCorrectionsSessionChanged = true;
    }   // SetCorrectionsSession()

    void SetCorrectionsEnd(const mCorrectionsEnd* pCorEnd)
    {
        sm_correctionsEnd = *pCorEnd;
        bCorrectionsEndChanged = true;
    }   // SetCorrectionsEnd()

    bool IsValidCorrectionSession(UINT a_sessionPair, CORRECTION_SESSION& a_correctionSession, const wxString& a_input, bool a_bHasError)
    {   // validate data and show error if not ok
        if (    a_bHasError
            || ((a_correctionSession.type != 'm') && (a_correctionSession.type != 'M') && (a_correctionSession.type != '%'))
            || (!cfg::GetButler() && a_correctionSession.extra > a_correctionSession.maxExtra*10)
            || (a_sessionPair > names::GetNumberOfGlobalPairs())
            || (a_sessionPair < 1)
            || (a_correctionSession.correction < MIN_CORRECTION)
            || (a_correctionSession.correction > MAX_CORRECTION)
           )
        {
            wxString msg = FMT(_("Invalid session-correction data <%s>, will be ignored."), a_input);
            MyLogError("%s",a_input);
            MyMessageBox(msg);
            return false;
        }

        if (a_correctionSession.type == '%') a_correctionSession.correction = std::clamp(a_correctionSession.correction, -100, 100);

        return true;
    }   // IsValidCorrectionSession()

    bool CorrectionsSessionRead()   //unconditional read
    {
        return io::CorrectionsSessionRead(sm_correctionsSession, cfg::GetActiveSession());
    }   // CorrectionsSessionRead()

    bool CorrectionsSessionWrite()
    {
        if (!bCorrectionsSessionChanged) return true;
        (void)ConfigChanged(true);  // refresh config
        bCorrectionsSessionChanged = false; // set false, even if errors occur...

        return io::CorrectionsSessionWrite(sm_correctionsSession, cfg::GetActiveSession());
    }   // CorrectionsSessionWrite()

    bool IsValidCorrectionEnd(UINT a_globalPair, const CORRECTION_END& a_ce, const wxString& a_input, bool a_bHasError)
    {
        bool bError = !cfg::GetButler() && (a_ce.score < 0);
        if (   a_bHasError
            || (bError)              || ((a_ce.score > 10000) && (a_ce.score != SCORE_IGNORE && a_ce.score != SCORE_NO_TOTAL))  // between 0 and 100%  :10000 = 100.00%
            || (a_ce.bonus < -9999)  || (a_ce.bonus > 9999 )  // between -99.99% and +99.99%
            || (a_globalPair < 1 )   || (a_globalPair > names::GetNumberOfGlobalPairs())
            || (a_ce.games > cfg:: MAX_GAMES ) //cfg::GetNrOfGames())
           )
        {
            wxString msg  = FMT(_("Invalid total-correction/end data <%s> will be ignored.\n"), a_input);
                     msg += FMT(_(" score  : %ld\n"), a_ce.score);
                     msg += FMT(_(" bonus  : %ld\n"), a_ce.bonus);
                     msg += FMT(_(" GlPair : %u\n" ), a_globalPair);
                     msg += FMT(_(" games: %u, cfg::max: %u"), a_ce.games, cfg::MAX_GAMES); //cfg::GetNrOfGames());
                     MyLogError("%s", msg);
            MyMessageBox(msg);
            return false;
        }

        return true;
    }   // IsValidCorrectionEnd()

    bool CorrectionsEndRead()   //unconditional read for editing
    {
        sm_correctionsEnd.clear();
        return io::CorrectionsEndRead(sm_correctionsEnd, cfg::GetActiveSession(), true);
    }   // CorrectionsEndRead()

    bool CorrectionsEndWrite()
    {
        if (!bCorrectionsEndChanged) return true;
        (void)ConfigChanged(true);      // refresh config
        bCorrectionsEndChanged = false; // set false, even if errors occur...

        return io::CorrectionsEndWrite(sm_correctionsEnd, cfg::GetActiveSession());
    }
}   // CorrectionsEndWrite()
