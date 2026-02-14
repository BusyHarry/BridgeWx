// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/filedlg.h>

#include "fileIo.h"
#include "database.h"
#include "orgInterface.h"
#include "sqlite.h"

namespace io
{   // all persistent match-io arrives here and is distributed to the wanted data-interface.

    static ActiveDbType seTheType = DB_ORG; // actions (only) applied to active type

    void DatabaseTypeSet(ActiveDbType a_type, bool a_bQuiet)
    {
        if (seTheType == a_type) return;
        DatabaseClose(DB_ALL);  // close all open databases.
        seTheType = a_type;
        if (!a_bQuiet && !cfg::IsScriptTesting()) MyMessageBox(_("Active match closed.\nSetup new match"));
    }   // DatabaseTypeSet()

    ActiveDbType DatabaseTypeGet()
    {
        return seTheType;
    }   // DatabaseTypeGet()

    wxString GetDbFileName(){ return db::GetDbFileName();}


    #define FUNCTION0(name,typeReturn,defaultReturn) \
    typeReturn name() \
    {\
        typeReturn ret(defaultReturn);\
        if (seTheType & DB_ORG)      ret = org::name(); \
        if (seTheType & DB_DATABASE) ret =  db::name(); \
        if (seTheType & DB_SQLITE)   ret = sql::name(); \
        return ret;\
    }

    #define FUNCTION1(name,typeReturn,defaultReturn,typeParam1) \
    typeReturn name(typeParam1 p1) \
    {\
        typeReturn ret(defaultReturn);\
        if (seTheType & DB_ORG)      ret = org::name(p1); \
        if (seTheType & DB_DATABASE) ret =  db::name(p1); \
        if (seTheType & DB_SQLITE)   ret = sql::name(p1); \
        return ret;\
    }

    #define FUNCTION2(name,typeReturn,defaultReturn,typeParam1,typeParam2) \
    typeReturn name(typeParam1 p1, typeParam2 p2) \
    {\
        typeReturn ret(defaultReturn);\
        if (seTheType & DB_ORG)      ret = org::name(p1,p2); \
        if (seTheType & DB_DATABASE) ret =  db::name(p1,p2); \
        if (seTheType & DB_SQLITE)   ret = sql::name(p1,p2); \
        return ret;\
    }

    #define FUNCTION3(name,typeReturn,defaultReturn,typeParam1,typeParam2,typeParam3) \
    typeReturn name(typeParam1 p1, typeParam2 p2, typeParam3 p3) \
    {\
        typeReturn ret(defaultReturn);\
        if (seTheType & DB_ORG)      ret = org::name(p1,p2,p3); \
        if (seTheType & DB_DATABASE) ret =  db::name(p1,p2,p3); \
        if (seTheType & DB_SQLITE)   ret = sql::name(p1,p2,p3); \
        return ret;\
    }

    //         function name           ret type    def.ret  paramtypes
    FUNCTION1(DatabaseFlush             ,bool       ,false  ,GlbDbType)
    FUNCTION1(DatabaseClose             ,bool       ,false  ,GlbDbType)
    FUNCTION1(DatabaseIsOpen            ,bool       ,false  ,GlbDbType)
    FUNCTION2(DatabaseOpen              ,CfgFileEnum,CFG_OK ,GlbDbType,CfgFileEnum)
    FUNCTION3(ReadValue                 ,wxString   ,ES     ,keyId,const wxString&,UINT)
    FUNCTION3(ReadValueBool             ,bool       ,false  ,keyId,bool,UINT)
    FUNCTION3(ReadValueLong             ,long       ,0L     ,keyId,long,UINT)
    FUNCTION3(ReadValueUINT             ,UINT       ,0U     ,keyId,UINT,UINT)
    FUNCTION3(WriteValue                ,bool       ,false  ,keyId,const wxString&,UINT)
    FUNCTION3(WriteValue                ,bool       ,false  ,keyId,bool,UINT)
    FUNCTION3(WriteValue                ,bool       ,false  ,keyId,long,UINT)
    FUNCTION3(WriteValue                ,bool       ,false  ,keyId,UINT,UINT)
    FUNCTION2(MinMaxClubRead            ,bool       ,false  ,UINT&,UINT&)
    FUNCTION2(MinMaxClubWrite           ,bool       ,false  ,UINT,UINT)
    FUNCTION1(MaxmeanRead               ,bool       ,false  ,Fdp&)
    FUNCTION1(MaxmeanWrite              ,bool       ,false  ,const Fdp&)
    FUNCTION2(SchemaRead                ,bool       ,false  ,      cfg::SessionInfo&,UINT)
    FUNCTION2(SchemaWrite               ,bool       ,false  ,const cfg::SessionInfo&,UINT)
    FUNCTION1(PairnamesWrite            ,bool       ,false  ,const names::PairInfoData&)
    FUNCTION1(PairnamesRead             ,bool       ,false  ,      names::PairInfoData&)
    FUNCTION2(Session2GlobalIdsRead     ,bool       ,false  ,      UINT_VECTOR&,UINT)
    FUNCTION2(Session2GlobalIdsWrite    ,bool       ,false  ,const UINT_VECTOR&,UINT)
    FUNCTION2(ClubnamesRead             ,bool       ,false  ,      std::vector<wxString>&,UINT&)
    FUNCTION1(ClubnamesWrite            ,bool       ,false  ,const std::vector<wxString>&)
    FUNCTION2(SessionNamesWrite         ,bool       ,false  ,const wxArrayString&,UINT)
    FUNCTION2(SessionNamesRead          ,bool       ,false  ,      wxArrayString&,UINT)
    FUNCTION2(ScoresRead                ,bool       ,false  ,      vvScoreData&,UINT)
    FUNCTION2(ScoresWrite               ,bool       ,false  ,const vvScoreData&,UINT)
    FUNCTION2(SessionRankRead           ,bool       ,false  ,      UINT_VECTOR&,UINT)
    FUNCTION2(SessionRankWrite          ,bool       ,false  ,const UINT_VECTOR&,UINT)
    FUNCTION2(TotalRankRead             ,bool       ,false  ,      UINT_VECTOR&,UINT)
    FUNCTION2(TotalRankWrite            ,bool       ,false  ,const UINT_VECTOR&,UINT)
    FUNCTION2(CorrectionsEndWrite       ,bool       ,false  ,const cor::mCorrectionsEnd&,UINT)
    FUNCTION3(CorrectionsEndRead        ,bool       ,false  ,      cor::mCorrectionsEnd&,UINT,bool)
    FUNCTION2(CorrectionsSessionWrite   ,bool       ,false  ,const cor::mCorrectionsSession&,UINT)
    FUNCTION2(CorrectionsSessionRead    ,bool       ,false  ,      cor::mCorrectionsSession&,UINT)
    FUNCTION2(SessionResultWrite        ,bool       ,false  ,const cor::mCorrectionsEnd&,UINT)
    FUNCTION2(SessionResultRead         ,bool       ,false  ,      cor::mCorrectionsEnd&,UINT)

    static bool CanContinue(const wxString& a_targetFile)
    {
        if (!wxFile::Exists(a_targetFile)) return true;
        if (wxCANCEL == MyMessageBox(FMT(_("Target file <%s> exists.\nRemove?"), a_targetFile), _("Check"),  wxYES|wxCANCEL))
            return false;
        wxRemoveFile(a_targetFile);
        return true;
    }   // CanContinue()

    void ConvertDataBase(ConvertFromTo a_how)
    {
        wxString fileTypes = (a_how == FromOldToDb ) ? _("ini files (*.ini)|*.ini") : _("db files (*.db)|*.db");
        wxFileDialog dialog(nullptr, _("'Bridge' Datafiles"), cfg::GetActiveMatchPath(), "", fileTypes, wxFD_OPEN|wxFD_FILE_MUST_EXIST);
        if (dialog.ShowModal() == wxID_CANCEL) return;
        wxFileName fileName = dialog.GetPath();
        wxString folder     = fileName.GetPath();
        wxString match      = fileName.GetName();
        class BackUpCfg
        {
        public:
            BackUpCfg()
            {   // backup essential values: activeMatch, activeMatchPath, session
                cfg::DataConversionBackup();
                //bOldGlobalNames = cfg::GetGlobalNameUse();
                //cfg::SetGlobalNameUse(false);   // we want match specific values for <match>.nm
            }
            ~BackUpCfg()
            {   // restore essential values
                cfg::DataConversionRestore();
                //cfg::SetGlobalNameUse(bOldGlobalNames);
            }
        private:
           // bool bOldGlobalNames;
        };
        BackUpCfg backup;                           // auto backup/restore cfg data
        cfg::DataConversionSetMatch(match);         // and set needed params for cfg::ConstructFilename()
        cfg::DataConversionSetMatchPath(folder);

        wxString tmpS; UINT tmpU, tmpU2; bool tmpB; // all temporary vars for read-old -> write-new
        Fdp tmpFdp;
        names::PairInfoData         pairNames;
        std::vector<wxString>       clubNames;
        std::vector<UINT>           uintV;
        vvScoreData                 scores;
        cfg::SessionInfo            si;
        cor::mCorrectionsEnd        ce;
        cor::mCorrectionsSession    cs;
        wxArrayString               arrayS;
        wxString                    dbFile  = cfg::ConstructFilename(cfg::EXT_DATABASE);
        wxString                    iniFile = cfg::ConstructFilename(cfg::EXT_MAIN_INI);

        if (a_how == FromOldToDb)
        {   // transfer <match>.ini data to <match>.db
            if (!CanContinue(dbFile)) return;  // check destination for unwanted overwrite
            org::DatabaseOpen(DB_MATCH, CFG_ONLY_READ);
             db::DatabaseOpen(DB_MATCH, CFG_WRITE);
            tmpS = org::ReadValue     (KEY_PRG_VERSION      , ES   ); db::WriteValue      (KEY_PRG_VERSION        , tmpS);
            tmpS = org::ReadValue     (KEY_MATCH_CMNT       , ES   ); db::WriteValue      (KEY_MATCH_CMNT         , tmpS);
            tmpS = org::ReadValue     (KEY_MATCH_DISCR      , ES   ); db::WriteValue      (KEY_MATCH_DISCR        , tmpS);
            tmpU = org::ReadValueUINT (KEY_MATCH_SESSION    , 0    ); db::WriteValue      (KEY_MATCH_SESSION      , tmpU);
            tmpS = org::ReadValue     (KEY_MATCH_PRNT       , ES   );
            tmpS.Replace(cfg::GetWinPrintPrefix(), ES       , false); db::WriteValue      (KEY_MATCH_PRNT         , tmpS);
            tmpU = org::ReadValueUINT (KEY_MATCH_MAX_ABSENT , 0    ); db::WriteValue      (KEY_MATCH_MAX_ABSENT   , tmpU);
                   org::MaxmeanRead   (tmpFdp                      ); db::MaxmeanWrite    (tmpFdp);
            tmpB = org::ReadValueBool (KEY_MATCH_CLOCK      , false); db::WriteValue      (KEY_MATCH_CLOCK        , tmpB);
            tmpB = org::ReadValueBool (KEY_MATCH_WEIGHTAVG  , false); db::WriteValue      (KEY_MATCH_WEIGHTAVG    , tmpB);
            tmpB = org::ReadValueBool (KEY_MATCH_VIDEO      , false); db::WriteValue      (KEY_MATCH_VIDEO        , tmpB);
            tmpU = org::ReadValueUINT (KEY_MATCH_LINESPP    , 0    ); db::WriteValue      (KEY_MATCH_LINESPP      , tmpU);
            tmpB = org::ReadValueBool (KEY_MATCH_NEUBERG    , false); db::WriteValue      (KEY_MATCH_NEUBERG      , tmpB);
            tmpB = org::ReadValueBool (KEY_MATCH_GRPRESULT  , false); db::WriteValue      (KEY_MATCH_GRPRESULT    , tmpB);
                   org::MinMaxClubRead(tmpU                 , tmpU2); db::MinMaxClubWrite (tmpU                   , tmpU2);
            tmpB = org::ReadValueBool (KEY_MATCH_FF         , false); db::WriteValue      (KEY_MATCH_FF           , tmpB);
            tmpB = org::ReadValueBool (KEY_MATCH_GLOBALNAMES, false); db::WriteValue      (KEY_MATCH_GLOBALNAMES  , tmpB);
                                                                      db::WriteValue      (KEY_MATCH_BUTLER       , false);
                   org::PairnamesRead(pairNames);                     db::PairnamesWrite  (pairNames);
                   org::ClubnamesRead(clubNames,tmpU);                db::ClubnamesWrite  (clubNames);
            // transfer session data of <match>.<x><session> data to <match>.db
            for (UINT session = 0; session <= cfg::MAX_SESSIONS; ++session)
            {
                        if (!wxFile::Exists(cfg::ConstructFilename(cfg::EXT_SESSION_INI, session))) continue;    // only existing data
                        cfg::DataConversionSetSession(session);     // datafiles use cfg-local sessionId
                        names::InitNames4Conversion(session);       // need actual names/assignments for scores/corrections
                        ce.clear();
                        cs.clear();
                        org::DatabaseOpen           (DB_SESSION, CFG_ONLY_READ);
                tmpS =  org::ReadValue              (KEY_SESSION_DISCR , ES, session ); db::WriteValue              (KEY_SESSION_DISCR , tmpS, session );
                        org::SchemaRead             (si     , session );                db::SchemaWrite             (si    , session);
                        org::ScoresRead             (scores , session);                 db::ScoresWrite             (scores, session);
                        org::Session2GlobalIdsRead  (uintV  , session);                 db::Session2GlobalIdsWrite  (uintV , session);
                        org::SessionNamesRead       (arrayS , session);                 db::SessionNamesWrite       (arrayS, session);
                        org::CorrectionsSessionRead (cs     , session);                 db::CorrectionsSessionWrite (cs    , session);
                        org::CorrectionsEndRead     (ce     , session, true);           db::CorrectionsEndWrite     (ce    , session);
                        org::SessionRankRead        (uintV  , session);                 db::SessionRankWrite        (uintV , session);
                        org::SessionResultRead      (ce     , session);                 db::SessionResultWrite      (ce     , session);
                        org::TotalRankRead          (uintV  , session);                 db::TotalRankWrite          (uintV , session);
            }
        }
        else
        {   // FromDbToOld
            if (!CanContinue(iniFile)) return;  // check destination for unwanted overwrite

             org::DatabaseOpen(DB_MATCH, CFG_WRITE);
              db::DatabaseOpen(DB_MATCH, CFG_ONLY_READ);

             tmpS = db::ReadValue     (KEY_PRG_VERSION      , ES   ); org::WriteValue      (KEY_PRG_VERSION        , tmpS);
             tmpS = db::ReadValue     (KEY_MATCH_CMNT       , ES   ); org::WriteValue      (KEY_MATCH_CMNT         , tmpS);
             tmpS = db::ReadValue     (KEY_MATCH_DISCR      , ES   ); org::WriteValue      (KEY_MATCH_DISCR        , tmpS);
             tmpU = db::ReadValueUINT (KEY_MATCH_SESSION    , 0    ); org::WriteValue      (KEY_MATCH_SESSION      , tmpU);
             tmpS = db::ReadValue     (KEY_MATCH_PRNT       , ES   );
             tmpS.Replace(cfg::GetWinPrintPrefix(), ES      , false); org::WriteValue      (KEY_MATCH_PRNT         , tmpS);
             tmpU = db::ReadValueUINT (KEY_MATCH_MAX_ABSENT , 0    ); org::WriteValue      (KEY_MATCH_MAX_ABSENT   , tmpU);
                    db::MaxmeanRead   (tmpFdp                      ); org::MaxmeanWrite    (tmpFdp);
             tmpB = db::ReadValueBool (KEY_MATCH_CLOCK      , false); org::WriteValue      (KEY_MATCH_CLOCK        , tmpB);
             tmpB = db::ReadValueBool (KEY_MATCH_WEIGHTAVG  , false); org::WriteValue      (KEY_MATCH_WEIGHTAVG    , tmpB);
             tmpB = db::ReadValueBool (KEY_MATCH_VIDEO      , false); org::WriteValue      (KEY_MATCH_VIDEO        , tmpB);
             tmpU = db::ReadValueUINT (KEY_MATCH_LINESPP    , 0    ); org::WriteValue      (KEY_MATCH_LINESPP      , tmpU);
             tmpB = db::ReadValueBool (KEY_MATCH_NEUBERG    , false); org::WriteValue      (KEY_MATCH_NEUBERG      , tmpB);
             tmpB = db::ReadValueBool (KEY_MATCH_GRPRESULT  , false); org::WriteValue      (KEY_MATCH_GRPRESULT    , tmpB);
                    db::MinMaxClubRead(tmpU                 , tmpU2); org::MinMaxClubWrite (tmpU                   , tmpU2);
             tmpB = db::ReadValueBool (KEY_MATCH_FF         , false); org::WriteValue      (KEY_MATCH_FF           , tmpB);
             tmpB = db::ReadValueBool (KEY_MATCH_GLOBALNAMES, false); org::WriteValue      (KEY_MATCH_GLOBALNAMES  , tmpB);
             // pairnames & clubnames
             db::PairnamesRead(pairNames);                     org::PairnamesWrite(pairNames);
             db::ClubnamesRead(clubNames,tmpU);                org::ClubnamesWrite(clubNames);
             // transfer session data of <match>.<x><session> data to <match>.db
             for (UINT session = 0; session <= cfg::MAX_SESSIONS; ++session)
             {
                if (!db::ExistSession(session)) continue;
                names::InitNames4Conversion(session);               // need actual names/assignments for scores/corrections
                cfg::DataConversionSetSession(session);             // datafiles use cfg-local sessionId
                wxString sessionIni = cfg::ConstructFilename(cfg::EXT_SESSION_INI, session);
                wxRemoveFile(sessionIni);   // remove, if  there: we don't want (partially) old data
                ce.clear();
                cs.clear();
                        org::DatabaseOpen           (DB_SESSION, CFG_WRITE);
                 tmpS =  db::ReadValue              (KEY_SESSION_DISCR , ES, session ); org::WriteValue              (KEY_SESSION_DISCR , tmpS, session );
                         db::SchemaRead             (si     , session );                org::SchemaWrite             (si    , session);
                         db::ScoresRead             (scores , session);                 org::ScoresWrite             (scores, session);
                         db::SessionRankRead        (uintV  , session);                 org::SessionRankWrite        (uintV , session);
                         db::Session2GlobalIdsRead  (uintV  , session);                 org::Session2GlobalIdsWrite  (uintV , session);
                         db::TotalRankRead          (uintV  , session);                 org::TotalRankWrite          (uintV , session);
                         db::CorrectionsSessionRead (cs     , session);                 org::CorrectionsSessionWrite (cs    , session);
                         db::CorrectionsEndRead     (ce     , session, true);           org::CorrectionsEndWrite     (ce    , session);
                         db::SessionNamesRead       (arrayS , session);                 org::SessionNamesWrite       (arrayS, session);
                ce.clear();    // preset pairnr's
                for (UINT pair = 1; pair <= names::GetNumberOfGlobalPairs(); ++pair) ce[pair] = cor::CORRECTION_END();  // init ce map
                         db::SessionResultRead      (ce     , session);                 org::SessionResultWrite      (ce     , session);
             }
        }
        org::DatabaseClose(DB_ALL);
         db::DatabaseClose(DB_ALL);
    }   // ConvertDataBase()
}   // end namespace io
