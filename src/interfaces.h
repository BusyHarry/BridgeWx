// Copyright(c) 2026-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
/*
*  function prototypes used in the namespace of:
*  - fileio       (io:: ) -> interface for application
*  - orgInterface (org::) -> implementation for 'old' types of storage
*  - database     (db:: ) -> implementation for 'wxConfig' type of storage
*/

    bool        ClubnamesRead           (      std::vector<wxString>&   clubNames, UINT& a_uMaxId);
    bool        ClubnamesWrite          (const std::vector<wxString>&   clubNames   );
    bool        CorrectionsEndRead      (      cor::mCorrectionsEnd& correctionsEnd, UINT session, bool bEdit);
    bool        CorrectionsEndWrite     (const cor::mCorrectionsEnd& correctionsEnd, UINT session);
    bool        CorrectionsSessionRead  (      cor::mCorrectionsSession& correctionsSession, UINT session);
    bool        CorrectionsSessionWrite (const cor::mCorrectionsSession& correctionsSession, UINT session);
    bool        DatabaseClose           (io::GlbDbType dbType = io::DB_ALL);
    bool        DatabaseFlush           (io::GlbDbType dbType = io::DB_ALL);           // flush the database(s)
    bool        DatabaseIsOpen          (io::GlbDbType dbType = io::DB_MAIN);
    CfgFileEnum DatabaseOpen            (io::GlbDbType dbType, CfgFileEnum how2Open);
    wxString    GetDbFileName           ();
    bool        MaxmeanRead             (UINT& maxmean);
    bool        MaxmeanWrite            (UINT  maxmean);
    bool        MinMaxClubRead          (UINT& min, UINT& max);
    bool        MinMaxClubWrite         (UINT  min, UINT max);
    bool        PairnamesRead           (      names::PairInfoData&     pairInfo    );
    bool        PairnamesWrite          (const names::PairInfoData&     pairInfo    );
    wxString    ReadValue               (keyId id, const wxString& defaultValue, UINT session = DEFAULT_SESSION);
    bool        ReadValueBool           (keyId id, bool            defaultValue, UINT session = DEFAULT_SESSION);
    long        ReadValueLong           (keyId id, long            defaultValue, UINT session = DEFAULT_SESSION);
    UINT        ReadValueUINT           (keyId id, UINT            defaultValue, UINT session = DEFAULT_SESSION);
    bool        SchemaRead              (      cfg::SessionInfo&        sessionInfo,UINT session);
    bool        SchemaWrite             (const cfg::SessionInfo&        sessionInfo,UINT session);
    bool        ScoresRead              (      vvScoreData&             scoreData, UINT session);
    bool        ScoresWrite             (const vvScoreData&             scoreData, UINT session);
    bool        Session2GlobalIdsRead   (      UINT_VECTOR& vuPairnrSession2Global, UINT session);
    bool        Session2GlobalIdsWrite  (const UINT_VECTOR& vuPairnrSession2Global, UINT session);
    bool        SessionNamesRead        (      wxArrayString& names, UINT session);
    bool        SessionNamesWrite       (const wxArrayString& names, UINT session);
    bool        SessionRankRead         (      UINT_VECTOR& vuRank, UINT session);
    bool        SessionRankWrite        (const UINT_VECTOR& vuRank, UINT session);
    bool        SessionResultRead       (      cor::mCorrectionsEnd& mSessionResult, UINT session);    // write and read: different params!
    bool        SessionResultWrite      (const cor::mCorrectionsEnd& mSessionResult, UINT session);    // write and read: different params!
    bool        TotalRankRead           (      UINT_VECTOR& vuRank, UINT session);
    bool        TotalRankWrite          (const UINT_VECTOR& vuRank, UINT session);
    bool        WriteValue              (keyId id, bool            value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue              (keyId id, const wxString& value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue              (keyId id, long            value,   UINT session = DEFAULT_SESSION);
    bool        WriteValue              (keyId id, UINT            value,   UINT session = DEFAULT_SESSION);
