// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _FILEIO_H_
#define _FILEIO_H_
#include "cfg.h"
#include "dbkeys.h"
#include "names.h"
#include "score.h"
#include "corrections.h"
#include "calcscore.h"

namespace io
{
    enum GlbDbType
    {   //apply actions supplied type of configuration. Only useful for 'original type' datafiles
          DB_MAIN      = 1
        , DB_MATCH     = 2
        , DB_SESSION   = 4
        , DB_ALL       = (DB_MAIN | DB_MATCH | DB_SESSION)
    };

    enum ActiveDbType
    {   // mask
          DB_ORG        = 1     // 'old' interface with many separate files
        , DB_DATABASE   = 2     // new interface with all matchdata in 1 file
        , DB_BOTH       = (DB_ORG | DB_DATABASE)
    };

    enum ConvertFromTo
    {
          FromOldToDb
        , FromDbToOld
    };

    void ConvertDataBase(ConvertFromTo how);

    void DatabaseTypeSet(ActiveDbType type, bool bQuiet = false);
    ActiveDbType DatabaseTypeGet();

    CfgFileEnum DatabaseOpen    (GlbDbType dbType, CfgFileEnum how2Open);
    bool        DatabaseClose   (GlbDbType dbType = DB_ALL);
    bool        DatabaseFlush   (GlbDbType dbType = DB_ALL);           // flush the database(s)
    bool        DatabaseIsOpen  (GlbDbType dbType = DB_MAIN);
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
    bool        SessionResultWrite      (const cor::mCorrectionsEnd& mSessionResult, UINT session);    // write and read: different params!
    bool        SessionResultRead       (      cor::mCorrectionsEnd& mSessionResult, UINT session);    // write and read: different params!
    bool        SessionRankRead         (      UINT_VECTOR& vuRank, UINT session);
    bool        SessionRankWrite        (const UINT_VECTOR& vuRank, UINT session);
    bool        TotalRankRead           (      UINT_VECTOR& vuRank, UINT session);
    bool        TotalRankWrite          (const UINT_VECTOR& vuRank, UINT session);

    wxString    ReadValue       (keyId id, const wxString& defaultValue, UINT session = DEFAULT_SESSION);
    bool        ReadValueBool   (keyId id, bool            defaultValue, UINT session = DEFAULT_SESSION);
    long        ReadValueLong   (keyId id, long            defaultValue, UINT session = DEFAULT_SESSION);
    UINT        ReadValueUINT   (keyId id, UINT            defaultValue, UINT session = DEFAULT_SESSION);

    bool        WriteValue      (keyId id, const wxString& value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue      (keyId id, bool            value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue      (keyId id, long            value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue      (keyId id, UINT            value,   UINT session = DEFAULT_SESSION);


} // namespace io

#endif // _FILEIO_H_
