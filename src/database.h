// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _DATABASE_H_
#define  _DATABASE_H_
#include "fileio.h"
#include "names.h"
#include "score.h"
#include "corrections.h"
#include "calcScore.h"
#include "dbkeys.h"

namespace db
{
    CfgFileEnum DatabaseOpen    (io::GlbDbType /*dbType*/, CfgFileEnum how2Open);
    bool        DatabaseClose   (io::GlbDbType dbType = io::DB_ALL);
    bool        DatabaseFlush   (io::GlbDbType dbType = io::DB_ALL);
    bool        DatabaseIsOpen  (io::GlbDbType dbType = io::DB_ALL);
    bool        ExistSession    (UINT session);
    wxString    GetDbFileName   ();


    bool        PairnamesWrite  (const names::PairInfoData&     pairInfo    );
    bool        PairnamesRead   (      names::PairInfoData&     pairInfo    );
    bool        ClubnamesRead   (      std::vector<wxString>&   clubNames, UINT& a_uMaxId);
    bool        ClubnamesWrite  (const std::vector<wxString>&   clubNames   );
    bool        ScoresRead      (      vvScoreData&             scoreData, UINT session);
    bool        ScoresWrite     (const vvScoreData&             scoreData, UINT session);
    bool        SchemaRead      (      cfg::SessionInfo&        sessionInfo,UINT session);
    bool        SchemaWrite     (const cfg::SessionInfo&        sessionInfo,UINT session);
    bool        MaxmeanRead     (UINT& maxmean);
    bool        MaxmeanWrite    (UINT  maxmean);
    bool        MinMaxClubRead  (UINT& min, UINT& max);
    bool        MinMaxClubWrite (UINT  min, UINT max);
    bool        Session2GlobalIdsRead   (      UINT_VECTOR& vuPairnrSession2Global, UINT session);
    bool        Session2GlobalIdsWrite  (const UINT_VECTOR& vuPairnrSession2Global, UINT session);
    bool        SessionNamesWrite       (const wxArrayString& names, UINT session);
    bool        SessionNamesRead        (      wxArrayString& names, UINT session);
    bool        CorrectionsEndWrite     (const cor::mCorrectionsEnd& correctionsEnd, UINT session);
    bool        CorrectionsEndRead      (      cor::mCorrectionsEnd& correctionsEnd, UINT session, bool bEdit);
    bool        CorrectionsSessionWrite (const cor::mCorrectionsSession& correctionsSession, UINT session);
    bool        CorrectionsSessionRead  (      cor::mCorrectionsSession& correctionsSession, UINT session);
    bool        SessionResultWrite      (const cor::mCorrectionsEnd& mSessionResult, UINT session);
    bool        SessionResultRead       (      cor::mCorrectionsEnd& mSessionResult, UINT session);
    bool        SessionRankRead         (      UINT_VECTOR& vuRank, UINT session);
    bool        SessionRankWrite        (const UINT_VECTOR& vuRank, UINT session);
    bool        TotalRankRead           (      UINT_VECTOR& vuRank, UINT session);
    bool        TotalRankWrite          (const UINT_VECTOR& vuRank, UINT session);

    wxString    ReadValue       (keyId id, const wxString& default, UINT session = DEFAULT_SESSION);
    bool        ReadValueBool   (keyId id, bool            default, UINT session = DEFAULT_SESSION);
    long        ReadValueLong   (keyId id, long            default, UINT session = DEFAULT_SESSION);
    UINT        ReadValueUINT   (keyId id, UINT            default, UINT session = DEFAULT_SESSION);

    bool        WriteValue      (keyId id, const wxString& value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue      (keyId id, bool            value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue      (keyId id, long            value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue      (keyId id, UINT            value,   UINT session = DEFAULT_SESSION);
}   // end namespace db

#endif
