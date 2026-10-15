// Copyright(c) 2026-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/*
   I need <wx/wxcrtvararg.h> but from version 3.2.9 to 3.2.11 this gives undifined 'WXDLLIMPEXP_BASE'
   Solution for now is to include 'wx/wx.h' to all (and more?) I need.
*/
#include <wx/wx.h>  // for wx/wxcrtvararg.h and the things this depends on.

#include "sqlite3.h"
#include "sqlite.h"
#include "dbglobals.h"

/*
* tables:
*   - info               -> always present : copyright and description of table-values
*   - main               -> always present : global match settings
*   - pairNames          -> always present : global name + index of clubName for this pair
*   - clubNames          -> always present : index + clubName
*   - session.x.main     -> settings for session <x>, created when read/write requested
*   - session.x.gamedata -> gameresults for session <x>
*
*/
namespace sql
{
    #define MAIN_TABLE  "main"                      /* name of the main table */
    #define GAME_COL    "GAME"                      /* name of the game column for scores*/
    #define DATA_COL    "DATA"                      /* name of the data column for scores*/
    static const long   s_lDbVersion = 100;         // assumed version (or smaller) for the code below
    static wxString     s_sqlDbFilename;            // current database, empty if database closed
    static wxString     s_sqlErrorMsg;              // sql errorstring of last SqlExec()
    static sqlite3*     s_sqlFp           = nullptr;// ptr to database, nullptr if not open
    static sqlite3*     s_sqlFpGlobalNames= nullptr;// ptr to database of global names, nullptr if not open
    static wxChar       s_cTheSeparator   = '@';    // default separator

    enum TableTypes {TEXT_ = 0, INT_ = 1};  // need '_' for INT: stupid ms has defined INT to be int
    static const char* s_acTableTypesText[] = { "TEXT", "INT" };
    static std::map<enum keyId, std::pair<const char*, TableTypes> > s_mDbKeys; // map, translating db_id to tablename and type

    static void     InitSdbMap          ();       // create the map for keyId -> tableName
    static wxString GetTableName        (keyId id, UINT session = DEFAULT_SESSION);
    static wxString GetColumnName       (keyId id);
    static void     CreateSessionTables (UINT session);
    static int      SqlExec             (const char* pSql);         // results, if any, are put in 's_sqlData'
    static int      SqlExec             (const wxString& sql);      // results, if any, are put in 's_sqlData'
    static bool     UintVectorWrite     (const UINT_VECTOR& vUint, UINT session, keyId id);
    static bool     UintVectorRead      (      UINT_VECTOR& vUint, UINT session, keyId id);

    struct ColumnInfo
    {   // query result for one column
        wxString name;
        wxString data;
    };
    static std::vector<std::vector<ColumnInfo>> s_sqlData;  // s_sqlData[row][col].

/************** implementation *************/
    static wxString SqlError4MsgBox(const char* a_msg)
    {
        return FMT(_("SqlExec %s has error(s), see log"), a_msg);
    }   // SqlError4MsgBox()

    wxString GetDbFileName() {return s_sqlDbFilename;}
    bool DatabaseIsOpen(io::GlbDbType /*dbType*/)
    {
        return s_sqlFp != nullptr;
    }   // DatabaseIsOpen()

    bool DatabaseClose(io::GlbDbType /*dbType*/)
    {
        s_sqlDbFilename.clear();
        if ( s_sqlFp )
            sqlite3_close(s_sqlFp);
        s_sqlFp = nullptr;
        if ( s_sqlFpGlobalNames )
            sqlite3_close(s_sqlFpGlobalNames);
        s_sqlFpGlobalNames = nullptr;
        return true;
    }   // DatabaseClose()

    static sqlite3* InitDatabase(const wxString& a_dbFile)
    {
        sqlite3* fp;    // local database ptr
        if ( !wxFile::Exists(a_dbFile) )
        {   // create the file with some initial comments, while we can't write comments to a configfile, can we???
            #define CHECK(x) if ( (x) != SQLITE_OK ) ++errorCount
            int errorCount = 0;
            int result     = sqlite3_open(a_dbFile.ToUTF8(), &fp);
            if ( result != SQLITE_OK )
            {
                MyLogError("%s", FMT(_("Can't open sql db '%s', error: '%s'"), a_dbFile, sqlite3_errmsg(fp)));
                sqlite3_free(fp);
                return nullptr;
            }
            std::swap(fp, s_sqlFp);   // set current db-ptr to global value
            const char* infoCreateTable = "CREATE TABLE INFO (EXPLANATION TEXT);";
            const char* infoInsertData =
                "INSERT INTO INFO VALUES"
                " (';info database           : data saved according definition of version <x>')"
                ",(';info pair names         : <global pairnr> = {<name>,<clubId>}, max size=30')"
                ",(';info club names         : <club id> = \"club name\", max size=25')"
                ",(';info game result        : <game nr> = {<ns>,<ew>,<ns result>,<ew result>[,\"ns contract\",\"ew contract\"]}[@ ...]')"
                ",(';info schema             : {<nr of games>,<setsize>,<first game>}@{<nr of pairs>,<absent pair>,\"schema\",\"groupChars\"}[@...]')"
                ",(';info min/max club       : {<min pairs needed>,<max pairs used>}')"
                ",(';info assignments        : <global pairnr>[@...] -> array[<sessionpair>] = <global pairnr>')"
                ",(';info assignments names  : <session name of global pair>[@...] -> array[<globalpair>] = <session name of global pair>')"
                ",(';info lines per page     : lines per page when printertype is <list>')"
                ",(';info corrections session: {<session pairnr>,<correction><type>,<extra.1>,<max extra>,<games>}[@...]')"
                ",(';info corrections end    : {<global pairnr>,<score.2>,<bonus.2>,<games>}[@...]')"
                ",(';info session result     : {<global pairnr>,<score.2>,<games>}[@...]')"
                ",(';info session rank       : <globalpair>[@...] array[<rank>]=<globalpair>')"
                ",(';info total rank         : <globalpair>[@...] array[<rank>]=<globalpair>')"
                ";";
            wxString mainCreateTable = "CREATE TABLE " MAIN_TABLE "( ID INT";
            #define ADD_COLUMN(key) \
                mainCreateTable += FMT(",%s %s", s_mDbKeys[key].first, s_acTableTypesText[s_mDbKeys[key].second])
            ADD_COLUMN(KEY_DB_VERSION);
            ADD_COLUMN(KEY_PRG_VERSION);
            ADD_COLUMN(KEY_MATCH_CMNT);
            ADD_COLUMN(KEY_MATCH_DISCR);
            ADD_COLUMN(KEY_MATCH_SESSION);
            ADD_COLUMN(KEY_MATCH_PRNT);
            ADD_COLUMN(KEY_MATCH_MAX_ABSENT);
            ADD_COLUMN(KEY_MATCH_MAXMEAN);
            ADD_COLUMN(KEY_MATCH_CLOCK);
            ADD_COLUMN(KEY_MATCH_WEIGHTAVG);
            ADD_COLUMN(KEY_MATCH_VIDEO);
            ADD_COLUMN(KEY_MATCH_LINESPP);
            ADD_COLUMN(KEY_MATCH_NEUBERG);
            ADD_COLUMN(KEY_MATCH_GRPRESULT);
            ADD_COLUMN(KEY_MATCH_MMCLUB);
            ADD_COLUMN(KEY_MATCH_FF);
            ADD_COLUMN(KEY_MATCH_GLOBALNAMES);
            ADD_COLUMN(KEY_MATCH_BUTLER);
            #undef ADD_COLUMN
            mainCreateTable += ");";   // close command
            CHECK(SqlExec("BEGIN;"));
              // create table 'info'
              CHECK(SqlExec(infoCreateTable));
              wxString InfoInsertCpy="INSERT INTO INFO VALUES(';copyright               : " + cfg::GetCopyrightDateTime()+"');";
              InfoInsertCpy.Replace("\n", ES);
              CHECK(SqlExec(InfoInsertCpy));
              CHECK(SqlExec(infoInsertData));

              // create table 'main'
              CHECK(SqlExec(mainCreateTable));
              const char* initId = "INSERT INTO " MAIN_TABLE " (ID) VALUES(1);";    // need to identify first row
              CHECK(SqlExec(initId));

              // create table pairnames
              wxString pairnamesTable = FMT("CREATE TABLE %s (ID INT, NAME TEXT, CLUBID INT);", GetColumnName(KEY_MATCH_PAIRNAMES) );
              CHECK(SqlExec(pairnamesTable));

              // create table clubnames
              wxString clubnamesTable = FMT("CREATE TABLE %s (ID INT, NAME TEXT);", GetColumnName(KEY_MATCH_CLUBNAMES) );
              CHECK(SqlExec(clubnamesTable));

              CHECK(SqlExec("COMMIT;"));    // execute all cmds
            std::swap(fp, s_sqlFp);           // back to original situation
            CHECK(sqlite3_close(fp));
            if ( errorCount )
                MyMessageBox(SqlError4MsgBox("InitDatabase()"));
            #undef CHECK
        }

        auto rc = sqlite3_open(a_dbFile.ToUTF8(), &fp); (void)rc;
        std::swap(fp, s_sqlFp);       // have the correct global fp for the next two calls....
        long version = ReadValueLong(KEY_DB_VERSION , s_lDbVersion);
        if ( version > s_lDbVersion )
        {
            wxString msg = FMT(_("Database version error, expected <= %ld, found %ld"), s_lDbVersion, version);
            MyLogError("%s", msg);
            MyMessageBox(msg);
        }
        (void) ReadValue(KEY_PRG_VERSION, cfg::GetVersion());
        std::swap(fp, s_sqlFp);     // back to original value for global fp
        return fp;
    }   // InitDatabase()

    CfgFileEnum DatabaseOpen(io::GlbDbType /*dbType*/, CfgFileEnum a_how2Open)
    {
        s_cTheSeparator = glb::GetSeparator();
        wxFileName db(cfg::ConstructFilename(cfg::EXT_SQLITE));
        if ( db.GetFullPath() == s_sqlDbFilename && s_sqlFp != nullptr )
            return CFG_OK;
        sqlite3_close(s_sqlFp); // will flush pending changes
        s_sqlFp = nullptr;
        InitSdbMap();

        bool bFileExist = wxFile::Exists(db.GetFullPath());
        if ( (a_how2Open == CFG_ONLY_READ) && !bFileExist )
        {   // only handle file if it already exists, so don't create it yet
            return CFG_ERROR;
        }

        if ( !db.DirExists() )
            (void)db.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);     // create full path, no error if allready there
        if ( !db.IsDirWritable() )
        {
            // if not writable, select "<documents_dir>/BridgeWx" folder
            wxString err = db.GetFullPath() + _(": is not accessable!\nPath set to: ") + cfg::GetBaseFolder();
            MyLogError( err );      // wxLogError will stop the programm with a pop-up...
            MyMessageBox(err);
            wxString dir = cfg::GetBaseFolder();
            cfg::SetActiveMatch(db.GetName(), dir); //NB, no extension!
            db = wxFileName(dir, db.GetFullName());
        }

        s_sqlDbFilename = db.GetFullPath();
        s_sqlFp = InitDatabase(s_sqlDbFilename);
        return CFG_OK;
    } // DatabaseOpen()

    bool DatabaseFlush(io::GlbDbType /*dbType*/)
    {   // sqlite: always flushed
        return true;
    }   // DatabaseFlush()

    template <typename T>
    int UpdateSingleItem(const wxString& a_table, const wxString& a_column, const T& a_value)
    {
        if ( !s_sqlFp )
            return SQLITE_CANTOPEN;
        wxString cmd;
        if constexpr ( std::is_same_v<T, wxString> )
            cmd = FMT("UPDATE %s SET %s = '%s' WHERE ID = 1;", a_table, a_column, a_value);
        if constexpr ( !std::is_same_v<T, wxString> )
            cmd = FMT("UPDATE %s SET %s = %ld WHERE ID = 1;" , a_table, a_column, static_cast<long>(a_value));
        int rc = SqlExec(cmd);
        return rc;
    }   // UpdateSingleItem<>

    template <typename T>
    bool UpdateSingleItem(keyId a_id, const T& a_value, UINT a_session = DEFAULT_SESSION)
    {
        if ( !s_sqlFp )
            return false;
        wxString column = GetColumnName(a_id);
        wxString table  = GetTableName(a_id, a_session);
        return SQLITE_OK == UpdateSingleItem<T>(table, column, a_value);
    }   // UpdateSingleItem<>

    template <typename T>
    T ReadSingleItem(keyId a_id, const T& a_defaultValue, UINT a_session = DEFAULT_SESSION)
    {   // T must be of (unsigned) type long, bool, int or wxString
        if ( !s_sqlFp )
            return a_defaultValue;
        wxString column = GetColumnName(a_id);
        wxString table  = GetTableName(a_id, a_session);
        wxString cmd    = FMT("SELECT %s FROM %s WHERE %s NOT NULL;", column, table, column);
        auto     rc     = SqlExec(cmd);
        if ( rc == SQLITE_OK )
        {   // table/column present, return its value or store default value
            if ( s_sqlData.size() && s_sqlData[0].size() )
            {
                wxString res = s_sqlData[0][0].data;
                if constexpr ( std::is_same_v<T, wxString> )
                    return res;
                if constexpr ( !std::is_same_v<T, wxString> )
                    return static_cast<T>(wxAtol(res));
            }
            else
            {   // no value present, so set default
               (void)UpdateSingleItem<T>(table, column, a_defaultValue);
            }
        }

        return a_defaultValue;
    }   // end ReadSingleItem<>

    long ReadValueLong(keyId a_id, long a_defaultValue, UINT a_session /* = DEFAULT_SESSION*/)
    {
        return ReadSingleItem<long>(a_id, a_defaultValue, a_session);
    }   // end ReadValueLong()

    wxString ReadValue(keyId a_id, const wxString& a_defaultValue, UINT a_session /* = DEFAULT_SESSION*/)
    {
        return ReadSingleItem<wxString>(a_id, a_defaultValue, a_session);
    }   // end ReadValue()

    bool ReadValueBool(keyId a_id, bool a_defaultValue, UINT a_session)
    {
        return ReadSingleItem<bool>(a_id, a_defaultValue, a_session);
    }   // ReadValueBool()

    UINT ReadValueUINT(keyId a_id, UINT a_defaultValue, UINT a_session)
    {
        return ReadSingleItem<UINT>(a_id, a_defaultValue, a_session);
    }   // ReadValueUINT()

    bool WriteValue(keyId a_id, const wxString& a_value, UINT a_session)
    {
        return UpdateSingleItem<wxString>(a_id, a_value, a_session);
    }   // WriteValue()

    bool WriteValue(keyId a_id, bool a_value, UINT a_session)
    {
        return UpdateSingleItem<bool>(a_id, a_value, a_session);
    }   // WriteValue()

    bool WriteValue(keyId a_id, long a_value, UINT a_session)
    {
        return UpdateSingleItem<long>(a_id, a_value, a_session);
    }   // WriteValue()

    bool WriteValue(keyId a_id, UINT a_value, UINT a_session)
    {
        return UpdateSingleItem<UINT>(a_id, a_value, a_session);
    }   // WriteValue()

    static void InitGlobalNames()
    {   // use a global db for pairnames/clubnames
        if ( !cfg::GetGlobalNameUse() )
            return; // use names from active match db

        // swap db-pointers at begin and end of functions that access pairnames/clubnames
        if ( s_sqlFpGlobalNames == nullptr )      // create/open global names db
            s_sqlFpGlobalNames = InitDatabase(cfg::GetGlobalNameFile());
        std::swap(s_sqlFpGlobalNames, s_sqlFp);
    }   // InitGlobalNames()

    bool PairnamesWrite(const names::PairInfoData& a_pairInfo)
    {
        if ( !s_sqlFp )
            return false;
        InitGlobalNames();  // use global db, if set so
        wxString    table   = GetTableName(KEY_MATCH_PAIRNAMES);

        (void)SqlExec("BEGIN;");
        wxString sql = FMT("DELETE FROM %s", table);
        bool bResult = SQLITE_OK == SqlExec(sql);       // remove content of pairnames
        wxString fmt = FMT("INSERT INTO %s VALUES(%%u,'%%s',%%u);", table);
        UINT pair = 0;
        for ( const auto& it : a_pairInfo )
        {
            if ( pair )
            {   // entry 0 is dummy
                sql = FMT(fmt, pair, it.pairName, it.clubIndex);
                if ( SQLITE_OK != SqlExec(sql) )
                    bResult = false;    // mark error, but continue: log will show errors
            }
            ++pair;
        }
        (void)SqlExec("COMMIT;");
        InitGlobalNames();  // back to local db
        return bResult;
    } // PairnamesWrite()

    bool PairnamesRead(names::PairInfoData& a_pairInfo)
    {
        a_pairInfo.clear();
        a_pairInfo.resize(1);
        if ( !s_sqlFp )
            return false;

        InitGlobalNames();  // use global db, if set so
        bool bResult = true;
        wxString sql = FMT("SELECT * FROM %s;", GetTableName(KEY_MATCH_PAIRNAMES) );
        (void)SqlExec(sql);
        size_t maxPair = s_sqlData.size();
        if ( maxPair > cfg::MAX_PAIRS )
            maxPair = cfg::MAX_PAIRS;   // ignore unwanted entries
        a_pairInfo.resize(maxPair+1);
        for ( const auto& pi : s_sqlData )
        {   // [0]=id, [1]=name, [2]=clubId
            UINT pair = wxAtoi(pi[0].data);
            if ( pair > maxPair )
            {
                MyLogError(_("Reading pairnames: pairnr <%s> too high!"), pi[0].data);
                bResult = false;
                continue;
            }
            names::PairInfo info(pi[1].data, (UINT)wxAtoi(pi[2].data));
            a_pairInfo[pair] = info;
        }
        InitGlobalNames();  // back to local db
        return bResult;
    } // PairnamesRead()

    bool ClubnamesRead(std::vector<wxString>& a_clubNames, UINT& a_uMaxId)
    {
        a_clubNames.clear();
        a_clubNames.resize(cfg::MAX_CLUBNAMES+1);
        a_uMaxId = cfg::MAX_CLUBID_UNION;   // 'free' added clubnames get an id starting here
        if ( !s_sqlFp )
            return false;

        InitGlobalNames();  // use global db, if set so
        bool bResult = true;
        wxString sql = FMT("SELECT * FROM %s;", GetTableName(KEY_MATCH_CLUBNAMES) );
        (void)SqlExec(sql);
        for ( const auto& ci : s_sqlData )
        {   // [0]=clubId, [1]=clubName
            auto clubId = (UINT)wxAtoi(ci[0].data);
            if ( clubId > cfg::MAX_CLUBNAMES )
            {
                MyLogError(_("Reading clubnames: clubnr <%u> too high!"), clubId);
                bResult = false;
                continue;
            }
            a_clubNames[clubId] = ci[1].data;
            if ( clubId > a_uMaxId )
                a_uMaxId = clubId;
        }

        InitGlobalNames();  // back to local db
        return bResult;
    } // ClubnamesRead()

    bool ClubnamesWrite(const std::vector<wxString>& a_clubNames)
    {
        if ( !s_sqlFp )
            return false;
        InitGlobalNames();  // use global db, if set so
        wxString table = GetTableName(KEY_MATCH_CLUBNAMES);
        (void)SqlExec("BEGIN;");
        wxString sql = FMT("DELETE FROM %s", table);
        bool bResult = SQLITE_OK == SqlExec(sql);       // remove content of clubnames
        wxString fmt = FMT("INSERT INTO %s VALUES(%%u,'%%s');", table);
        UINT clubId  = 0;

        for ( const auto& ci : a_clubNames )
        {
            if ( clubId && !ci.IsEmpty() )
            {   // entry 0 is dummy
                sql = FMT(fmt, clubId, ci);
                if ( SQLITE_OK != SqlExec(sql) )
                    bResult = false;
            }
            ++clubId;
        }
        (void)SqlExec("COMMIT;");
        InitGlobalNames();  // back to local db
        return bResult;
    } // ClubnamesWrite()

    bool ExistSession(UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        // const char* e1 = "SELECT EXISTS(SELECT 1 FROM sqlite_master WHERE type="table" AND name ="table_name");";
        // const char* e2 = "select name from sqlite_master where tbl_name like 'm%';";
        wxString sql = FMT("select tbl_name from sqlite_master where tbl_name like 'session.%u." MAIN_TABLE "';", a_session);
        int rc    = SqlExec(sql);
        auto size = s_sqlData.size();
        return rc == SQLITE_OK && size > 0;
    }   // end ExistSession()

    static void InitSdbMap()
    {
        static bool s_bIsInited = false;
        if ( s_bIsInited )
            return;
        s_bIsInited = true;
        s_mDbKeys[KEY_DB_VERSION]               = { "databaseVersion"  , INT_ };
        s_mDbKeys[KEY_PRG_VERSION]              = { "programVersion"   , TEXT_};

        s_mDbKeys[KEY_MATCH_MAX_ABSENT]         = { "maxAbsent"        , INT_ };
        s_mDbKeys[KEY_MATCH_PRNT]               = { "printer"          , TEXT_};
        s_mDbKeys[KEY_MATCH_MAXMEAN]            = { "maxMean"          , TEXT_};
        s_mDbKeys[KEY_MATCH_CLOCK]              = { "clock"            , INT_ };
        s_mDbKeys[KEY_MATCH_WEIGHTAVG]          = { "weightedAverage"  , INT_ };
        s_mDbKeys[KEY_MATCH_LINESPP]            = { "linesPerPage"     , INT_ };
        s_mDbKeys[KEY_MATCH_NEUBERG]            = { "neuberg"          , INT_ };
        s_mDbKeys[KEY_MATCH_GRPRESULT]          = { "groupResult"      , INT_ };
        s_mDbKeys[KEY_MATCH_MMCLUB]             = { "minMaxClub"       , TEXT_};
        s_mDbKeys[KEY_MATCH_GLOBALNAMES]        = { "centralNames"     , INT_ };
        s_mDbKeys[KEY_MATCH_SESSION]            = { "session"          , INT_ };
        s_mDbKeys[KEY_MATCH_PAIRNAMES]          = { "pairNames"        , TEXT_};
        s_mDbKeys[KEY_MATCH_CLUBNAMES]          = { "clubNames"        , TEXT_};

        s_mDbKeys[KEY_MATCH_CMNT]               = { "comment"          , TEXT_};   // filter out: old
        s_mDbKeys[KEY_MATCH_FF]                 = { "form_feed"        , INT_ };   // filter out: old
        s_mDbKeys[KEY_MATCH_VIDEO]              = { "bios_video"       , INT_ };   // filter out: old
        s_mDbKeys[KEY_MATCH_DISCR]              = { "description"      , TEXT_};
        s_mDbKeys[KEY_MATCH_BUTLER]             = { "butler"           , INT_ };

        s_mDbKeys[KEY_SESSION_DISCR]            = { "description"      , TEXT_};
        s_mDbKeys[KEY_SESSION_SCHEMA]           = { "schema"           , TEXT_};
        s_mDbKeys[KEY_SESSION_GAMERESULT]       = { "gameResult"       , TEXT_};
        s_mDbKeys[KEY_SESSION_ASSIGNMENTS]      = { "assignments"      , TEXT_};   // array of global pairnr's
        s_mDbKeys[KEY_SESSION_ASSIGNMENTS_NAME] = { "assignmentsName"  , TEXT_};   // array for global pairnr's of session names
        s_mDbKeys[KEY_SESSION_CORRECTION]       = { "correctionsSession",TEXT_};   // corrections for this session
        s_mDbKeys[KEY_SESSION_CORRECTION_END]   = { "correctionsEnd"   , TEXT_};   // corrections for total/end calculation
        s_mDbKeys[KEY_SESSION_RESULT]           = { "sessionResult"    , TEXT_};   // short session result, used for total/end calculation
        s_mDbKeys[KEY_SESSION_RANK_SESSION]     = { "sessionrank"      , TEXT_};   // rank in the sessionresult
        s_mDbKeys[KEY_SESSION_RANK_TOTAL]       = { "totalrank"        , TEXT_};   // rank in the totalresult for this session

    }  // InitSdbMap()

    static wxString GetTableName(keyId a_id, UINT a_session /* = DEFAULT_SESSION */)
    {
        static std::map<UINT, bool> s_mSessionExist;
        if ( (a_session != DEFAULT_SESSION) && (!s_mSessionExist.contains(a_session)) )
        {
            s_mSessionExist[a_session] = true;  // must come first, else recursion till death follows...
            CreateSessionTables( a_session );
        }

        wxString table;
        switch ( a_id )
        {
            case KEY_SESSION_GAMERESULT:
                table = FMT("'session.%u.%s'", a_session, s_mDbKeys[a_id].first);
                break;
            case KEY_MATCH_CLUBNAMES:
            case KEY_MATCH_PAIRNAMES:
                table = s_mDbKeys[a_id].first;
                break;
            default:
                if ( a_session == DEFAULT_SESSION )
                    table = MAIN_TABLE;
                else
                    table = FMT("'session.%u.%s'", a_session, MAIN_TABLE);
                break;
        }
        return table;
    }   // GetTableName()

    static wxString GetColumnName(keyId a_id)
    {
        return wxString(s_mDbKeys[a_id].first);
    }   // GetColumnName()

    static int SqlExec(const char* a_pSql)
    {   // this will call the sql3 execution unit
        // results: s_sqlData and s_sqlErrorMsg
        s_sqlErrorMsg.clear();
        s_sqlData.clear();    // clear data before exec(): callback will be called for each row with data
        auto CallBack = [] (void* /*pUserData*/, int count, char* columnData[], char* columnNames[])  -> int
        {   // called for each row result
            std::vector<ColumnInfo> rowData;

            for ( int index = 0; index < count; ++index )
            {
                rowData.emplace_back(columnNames[index], columnData[index] ? columnData[index] : "");
            }
            s_sqlData.emplace_back(rowData);
            return 0;   // non-zero will force the exec function to abort
        };

        auto rc = sqlite3_exec(s_sqlFp, a_pSql, CallBack, nullptr, nullptr);
        if ( rc != SQLITE_OK )
        {
            // if sqlite3_exec is called with non-zero errorptr, then
            // you need to call sqlite3_free() when you don't need
            // the errortext anymore to prevent memory leaks
            const char* perr = sqlite3_errmsg(s_sqlFp);
            s_sqlErrorMsg      = perr;
            MyLogError("sqlite3_exec(%s) returned error %d (%s)", a_pSql, rc, perr);
        }

        return rc;
    }   // SqlExec()

    static int SqlExec(const wxString& a_sql)
    {
        auto        buf = a_sql.ToUTF8();
        const char* sql = buf;
        return SqlExec(sql);
    }   // SqlExec()

    static void CreateSessionTables(UINT a_session)
    {   // create the MAIN table and the gameresult table for this session
        if ( s_sqlFp == nullptr || ExistSession( a_session) )
            return;
        int         errorCount = 0;
        wxString    table      = GetTableName(KEY_SESSION_GAMERESULT, a_session);
        wxString    sql        = FMT("CREATE TABLE %s (" GAME_COL " INT," DATA_COL " TEXT); ", table);
        #define CHECK(x) if ( (x) != SQLITE_OK ) ++errorCount
        CHECK(SqlExec("BEGIN"));
        CHECK(SqlExec(sql));

        table = GetTableName(KEY_SESSION_DISCR, a_session);
        sql = FMT("CREATE TABLE %s ( ID INT", table);
        #define ADD_COLUMN(key) \
                sql += FMT(",%s %s", s_mDbKeys[key].first, s_acTableTypesText[s_mDbKeys[key].second])
        ADD_COLUMN(KEY_SESSION_DISCR);
        ADD_COLUMN(KEY_SESSION_SCHEMA);
        ADD_COLUMN(KEY_SESSION_ASSIGNMENTS);
        ADD_COLUMN(KEY_SESSION_ASSIGNMENTS_NAME);
        ADD_COLUMN(KEY_SESSION_CORRECTION);
        ADD_COLUMN(KEY_SESSION_CORRECTION_END);
        ADD_COLUMN(KEY_SESSION_RANK_SESSION);
        ADD_COLUMN(KEY_SESSION_RESULT);
        ADD_COLUMN(KEY_SESSION_RANK_TOTAL);
        #undef ADD_COLUMN
        sql += ");";   // close sql command
        CHECK(SqlExec(sql));
        sql = FMT("INSERT INTO %s (ID) VALUES(1);", table);    // need to identify first row
        CHECK(SqlExec(sql));
        CHECK(SqlExec("COMMIT;"));
        if ( errorCount )
        {
            MyMessageBox(SqlError4MsgBox("CreateSessionTables()"));
        }
        #undef CHECK
    }   // CreateSessionTables()

    static bool UintVectorRead(UINT_VECTOR& a_vUint, UINT a_session, keyId a_id)
    {   // Resize vector to cfg::MAX_PAIRS and read a set of UINTs and put them in a vector.
        if ( !s_sqlFp )
            return false;
        wxString key  = FMT("%s:%s", GetTableName(a_id, a_session), GetColumnName(a_id));
        wxString info = ReadValue(a_id, ES, a_session);
        return glb::UintVectorRead(a_vUint, info, s_sqlDbFilename, key, SqlError4MsgBox("UintVectorRead()"));
    }   //UintVectorRead()

    static bool UintVectorWrite(const UINT_VECTOR& a_vUint, UINT a_session, keyId a_id)
    {   // write contents of vector as UINT, ignoring entry 0
        if ( !s_sqlFp )
            return false;
        wxString info = glb::UintVectorWrite(a_vUint);
        return WriteValue(a_id, info, a_session);
    }   // UintVectorWrite()

    bool Session2GlobalIdsWrite(const UINT_VECTOR& a_vuPairnrSession2Global, UINT a_session)
    {
        return UintVectorWrite(a_vuPairnrSession2Global, a_session, KEY_SESSION_ASSIGNMENTS);
    }   // Session2GlobalIdsWrite()

    bool Session2GlobalIdsRead(UINT_VECTOR& a_vuPairnrSession2Global, UINT a_session)
    {
        // The file has the pairs based on sessionpairnr order, entry zero = dummy
        // pairIndex[x]: sessionPairNr 'x' is mapped to globalPairNr 'pairIndex[x]'
        // example: pairIndex[0,3,20,5,7] -> sessionpair 1 <==> globalpair 3, sp 4 <==> gp 7

        return UintVectorRead(a_vuPairnrSession2Global, a_session, KEY_SESSION_ASSIGNMENTS);
    } // Session2GlobalIdsRead()

    bool SessionRankWrite(const UINT_VECTOR& a_vuRank, UINT a_session)
    {
        return UintVectorWrite(a_vuRank, a_session, KEY_SESSION_RANK_SESSION);
    }   // SessionRankWrite()

    bool SessionRankRead(UINT_VECTOR& a_vuRank, UINT a_session)
    {
        return UintVectorRead(a_vuRank, a_session, KEY_SESSION_RANK_SESSION);
    }   // SessionRankRead()

    bool TotalRankWrite(const UINT_VECTOR& a_vuRank, UINT a_session)
    {
        return UintVectorWrite(a_vuRank, a_session, KEY_SESSION_RANK_TOTAL);
    }   // TotalRankWrite()

    bool TotalRankRead(UINT_VECTOR& a_vuRank, UINT a_session)
    {
        return UintVectorRead(a_vuRank, a_session, KEY_SESSION_RANK_TOTAL);
    }   // TotalRankRead()

    static bool CB_ScoresWriteGame(UINT a_game, const wxString& a_gameScores, void* a_pUserData)
    {   // write the scores of a single game
        wxString sql = FMT(*(reinterpret_cast<wxString*>(a_pUserData)), a_game, a_gameScores);
        return SQLITE_OK == SqlExec(sql);
    }   // CB_ScoreWriteGame()

    bool ScoresWrite(const vvScoreData& a_scoreData, UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        wxString sqlTable = GetTableName(KEY_SESSION_GAMERESULT, a_session);
        wxString sql      = FMT("DELETE FROM %s", sqlTable);
        if ( SQLITE_OK != SqlExec(sql) )
            return false;
        wxString cb_fmtScoresWrite = FMT("INSERT INTO %s (" GAME_COL "," DATA_COL ") VALUES(%%u, '%%s'); ", sqlTable);
        (void)SqlExec("BEGIN;");
        auto result = glb::ScoresWrite(a_scoreData, CB_ScoresWriteGame, &cb_fmtScoresWrite );
        (void)SqlExec("COMMIT;");
        return result;
    }   // ScoresWrite()

    static bool CB_ScoresReadLine(wxString& a_game, wxString& a_gameScores, void* a_pUserData)
    {   // read the scores of a single game at 'index'
        size_t& index = *(reinterpret_cast<size_t*>(a_pUserData));
        if ( index >= s_sqlData.size() )
            return false;   // no more data available
        a_game       = s_sqlData[index][0].data;
        a_gameScores = s_sqlData[index][1].data;
        ++index;   // next line to read
        return true;
    }   // CB_ScoresReadLine()

    bool ScoresRead(vvScoreData& a_scoreData, UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        wxString sqlTable = GetTableName(KEY_SESSION_GAMERESULT, a_session);
        wxString sql = FMT("SELECT * FROM %s;", sqlTable);
        if ( SQLITE_OK != SqlExec(sql) )
            return false;   // no table?
        // now ALL scsores are read into 's_sqlData'
        size_t index = 0;   // read data, starting at index 0
        auto rc = glb::ScoresRead(a_scoreData, CB_ScoresReadLine, &index);
        return rc;
    }   // ScoresRead()

    bool CorrectionsEndRead(cor::mCorrectionsEnd& a_mCorrectionsEnd, UINT a_session, bool a_bEdit)
    {
        if ( !s_sqlFp )
            return false;
        wxString data = ReadSingleItem(KEY_SESSION_CORRECTION_END, ES, a_session);
        return glb::CorrectionsEndRead(a_mCorrectionsEnd, a_bEdit, data);
    }   // CorrectionsEndRead()

    bool CorrectionsEndWrite(const cor::mCorrectionsEnd& a_m_correctionsEnd, UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        wxString correction = glb::CorrectionsEndWrite(a_m_correctionsEnd);
        return UpdateSingleItem(KEY_SESSION_CORRECTION_END, correction, a_session);
    }   // CorrectionsEndWrite()

    bool CorrectionsSessionRead(cor::mCorrectionsSession& a_mCorrectionsSession, UINT a_session)
    {
        a_mCorrectionsSession.clear();
        if ( !s_sqlFp )
            return false;
        wxString corrections = ReadSingleItem(KEY_SESSION_CORRECTION, ES, a_session);
        return glb::CorrectionsSessionRead(a_mCorrectionsSession, corrections);
    }   // CorrectionsSessionRead()

    bool CorrectionsSessionWrite(const cor::mCorrectionsSession& a_m_correctionsSession, UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        wxString correction = glb::CorrectionsSessionWrite(a_m_correctionsSession);
        return UpdateSingleItem(KEY_SESSION_CORRECTION, correction, a_session);
    }   // CorrectionsSessionWrite()

    bool MaxmeanRead(Fdp& a_maxmean)
    {
        if ( !s_sqlFp )
            return false;
        a_maxmean = ReadSingleItem(KEY_MATCH_MAXMEAN, a_maxmean.AsString2F());
        return true;
    }   // MaxmeanRead()

    bool MaxmeanWrite(const Fdp& a_maxmean)
    {
        if ( !s_sqlFp )
            return false;
        return UpdateSingleItem(KEY_MATCH_MAXMEAN, a_maxmean.AsString2F());
    } // MaxmeanWrite()

    bool MinMaxClubRead(UINT& a_min, UINT& a_max)
    {
        if ( !s_sqlFp )
            return false;
        wxString defaultValue = FMT("{%u,%u}", a_min, a_max);
        wxString sMinMax = ReadSingleItem(KEY_MATCH_MMCLUB, defaultValue);
        auto count = wxSscanf(sMinMax, " {%u ,%u }", &a_min, &a_max);
        return count == 2;
    } // MinMaxClubRead()

    bool MinMaxClubWrite(UINT a_min, UINT a_max)
    {
        if ( !s_sqlFp )
            return false;
        wxString minMax = FMT("{%u,%u}", a_min, a_max);
        return UpdateSingleItem(KEY_MATCH_MMCLUB, minMax);
    } // MinMaxClubWrite()

    bool SchemaRead(cfg::SessionInfo& a_info, UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        wxString defaultValue = glb::GetDefaultSchema();
        wxString schema       = ReadSingleItem(KEY_SESSION_SCHEMA, defaultValue, a_session);
        return glb::SchemaRead(a_info, schema);
    }  // SchemaRead()

    bool SchemaWrite(const cfg::SessionInfo& a_info, UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        wxString info = glb::SchemaWrite(a_info);
        return UpdateSingleItem(KEY_SESSION_SCHEMA, info, a_session);
    }   // SchemaWrite()

    bool SessionNamesRead(wxArrayString& a_assignmentsName, UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        wxString info = ReadSingleItem(KEY_SESSION_ASSIGNMENTS_NAME, ES, a_session);
        return glb::SessionNamesRead(a_assignmentsName, info);
    }   // SessionNamesRead()

    bool SessionNamesWrite(const wxArrayString& a_names, UINT a_session)
    {   // sessionnames for globalpairnrs of a session
        if ( !s_sqlFp )
            return false;
        wxString info = glb::SessionNamesWrite(a_names);
        return UpdateSingleItem(KEY_SESSION_ASSIGNMENTS_NAME, info, a_session);
    }   // SessionNamesWrite()

    bool SessionResultWrite(const cor::mCorrectionsEnd& a_mSessionResult, UINT a_session)
    {
        if ( !s_sqlFp )
            return false;
        wxString result = glb::SessionResultWrite(a_mSessionResult);
        return UpdateSingleItem(KEY_SESSION_RESULT, result, a_session);
    }   // SessionResultWrite()

    bool SessionResultRead(cor::mCorrectionsEnd& a_mSessionResult, UINT a_session)    // write and read: different params!
    {   // NB input MAP is initialized, so don't clear or resize it or add 'new' pairnrs!
        if ( !s_sqlFp )
            return false;
        wxString info = ReadSingleItem(KEY_SESSION_RESULT, ES, a_session);
        return glb::SessionResultRead(a_mSessionResult, info);
    } //SessionResultRead()

    int ScoresWriteEx(const wxString& a_dBase, const vvScoreData& a_scoreData, UINT a_session)
    {   // only write to existing files
        if ( !wxFile::Exists(a_dBase) )
            return EX_RESULT_NOT_EXIST;
        if ( a_dBase.Lower() == s_sqlDbFilename.Lower() )
            return EX_RESULT_CURRENT;   // don't open current active system db
        sqlite3* pSqlEx = InitDatabase(a_dBase);
        if ( pSqlEx == nullptr )
            return EX_RESULT_ERROR;

        bool bResult = false;;
        std::swap(pSqlEx, s_sqlFp);       // have the correct value for the next calls....
        bResult = ScoresWrite(a_scoreData, a_session);
        std::swap(pSqlEx, s_sqlFp);       // back to how it was
        sqlite3_close(pSqlEx);          // will also flush pending changes
        return bResult ? EX_RESULT_OK : EX_RESULT_ERROR;
    }   // ScoresWriteEx()

} // namespace sql

#if 0
void SqlTest()
{
    sql::s_sqlDbFilename = "f:/sqlTest.sqlite";
//    wxRemoveFile(file);
    sql::InitSdbMap();
    sql::s_sqlFp = sql::InitDatabase(sql::s_sqlDbFilename);

    bool result = sql::ExistSession(0); (void)result;

    long version = sql::ReadValueLong(KEY_DB_VERSION , 1234L); (void)version;

    bool bButler = sql::ReadValueBool( KEY_MATCH_BUTLER, false); (void)bButler;
    bButler      = sql::ReadValueBool( KEY_MATCH_BUTLER, true);  (void)bButler;

    auto tblString1 = sql::GetTableName(KEY_DB_VERSION);    (void)tblString1;
    auto tblString2 = sql::GetTableName(KEY_DB_VERSION,3);  (void)tblString2;

    auto res1 = sql::WriteValue(KEY_MATCH_BUTLER        , true); (void)res1;
    auto res2 = sql::WriteValue(KEY_MATCH_SESSION       , 2U); (void)res2;
    auto res3 = sql::WriteValue(KEY_MATCH_CMNT          , wxString("hello me!")); (void)res3;
    auto res6 = sql::WriteValue(KEY_SESSION_ASSIGNMENTS , wxString("test_assign") , 3); (void)res6;

    auto res5 = sql::WriteValue((keyId)1234             , true); (void)res5;  //<-- should give error, see log

    names::PairInfoData pid(1);  // entry 0 is dummy
    pid.emplace_back( names::PairInfo ("harrie"     , 0 ));
    pid.emplace_back( names::PairInfo ("jan en piet", 7 ));
    auto res7 = sql::PairnamesWrite( pid ); (void)res7;
    auto res8 = sql::PairnamesRead( pid ); (void)res8;

    std::vector<wxString> clubNames;
    clubNames.push_back(ES);
    clubNames.push_back("club1");
    clubNames.push_back(ES);
    clubNames.push_back("club3");
    auto res9 = sql::ClubnamesWrite( clubNames ); (void)res9;

    clubNames.clear();
    UINT maxClubId = 0;
    auto res10 = sql::ClubnamesRead( clubNames, maxClubId ); (void)res10;

    UINT_VECTOR rank = {0,1,2,4,3};
    auto res11= sql::TotalRankWrite(rank, 3); (void) res11;

    rank.clear();
    //gives errors because no names are readin yet.// auto res12= sql::TotalRankRead(rank, 3); (void) res12;


    sql::DatabaseClose();
}   // end SqlTest()
#endif
