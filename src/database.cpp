// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/fileconf.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <wx/arrstr.h>
#include <wx/wxcrtvararg.h>
#include <map>

#include "mylog.h"
#include "database.h"
#include "dbglobals.h"

namespace db
{
static const long dbVersion = 100;  // assumed version (or smaller) for the code below
static void InitSdb();

#define TEST 0
#if TEST
    static void Test();
    static void Test2();
#endif

static bool                     sbGlobelNames = false;  // using names/clubs from a global database
static wxFileConfig*            s_pConfig = nullptr;    // current database manager
static wxFileConfig*            s_pConfigGlobalNames = nullptr; // dbase if globalPairNames active
static wxString                 sDbFile;                // current database, empty if database closed
static std::map<enum keyId,wxString>   dbKeys;          // map, translating db_id to key
static wxChar                   theSeparator = '@';     // default separator

static struct { char input; wxString replace; } charEncoding[] = { {'"',  "&quot;"} };
static wxString EncodeString(const wxString& a_string)
{   // encode special char in a string
    wxString result(a_string);

    for (const auto& it : charEncoding)
        result.Replace(it.input, it.replace);
    return result;
}   // EncodeString()

static wxString DecodeString(const wxString& a_string)
{   // decode special char in a string
    wxString result(a_string);

    for (const auto& it : charEncoding)
        result.Replace(it.replace, it.input);
    return result;
}   // DecodeString()

class ReadConfigGroup
{   // support class for easy getting all entries in a group
public:
    ReadConfigGroup(wxFileConfig* a_pConfig, const wxString& a_path)
    {
        m_pConfig   = a_pConfig;
        m_bFirst    = true;
        m_bHasNext  = false;
        m_index     = 0;
        m_pConfig->SetPath(a_path);
    }

    bool GetNextEntry(wxString& a_key, wxString& a_value)
    {
        m_bHasNext = m_bFirst ? m_pConfig->GetFirstEntry(a_key, m_index) : m_pConfig->GetNextEntry(a_key, m_index);
        a_value = m_bHasNext ? m_pConfig->Read(a_key, ES) : ES;
        m_bFirst = false;
        return m_bHasNext;
    }

    size_t GetNumberOfEntries(){return m_pConfig->GetNumberOfEntries();}

private:
    bool            m_bFirst;
    bool            m_bHasNext;
    long            m_index;
    wxFileConfig*   m_pConfig;
};

static wxString MakePath(keyId a_id, UINT a_session = DEFAULT_SESSION )
{
    return a_session == DEFAULT_SESSION ?
        "/main/" + dbKeys[a_id]
      : FMT("/%u/%s", a_session, dbKeys[a_id]);
}   // MakePath()

bool ExistSession(UINT a_session)
{
    return DatabaseIsOpen() && s_pConfig->HasGroup(FMT("/%u", a_session));
}   // ExistSession()

static bool UintVectorWrite(const UINT_VECTOR& vUint, UINT session, keyId id);
static bool UintVectorRead(       UINT_VECTOR& vUint, UINT session, keyId id);

wxString GetDbFileName() {return sDbFile;}

bool DatabaseIsOpen(io::GlbDbType /*dbType*/)
{
    return s_pConfig != nullptr;
}   // DatabaseIsOpen()

bool DatabaseClose(io::GlbDbType /*dbType*/)
{
    delete s_pConfig;   // will flush pending changes
    s_pConfig = nullptr;
    sDbFile.clear();

    delete s_pConfigGlobalNames;
    s_pConfigGlobalNames = nullptr;
    return true;
}   // DatabaseClose()

static wxFileConfig* InitDatabase(const wxString& a_dbFile)
{
    if ( !wxFile::Exists(a_dbFile) )
    {   // create the file with some initial comments, while we can't write comments to a configfile, can we???
        wxString cpy=cfg::GetCopyrightDateTime(); cpy.Replace("\n", ES);
        MyTextFile file;
        file.MyCreate(a_dbFile, MyTextFile::WRITE);
        file.AddLine("[info_begin]");
        file.AddLine(";copyright               : " + cpy);
        file.AddLine(";info database           : 'data saved according definition of version <x>'");
        file.AddLine(";info pair names         : '<global pairnr> = {<name>,<clubId>}, max size=30'");
        file.AddLine(";info club names         : '<club id> = \"club name\", max size=25'");
        file.AddLine(";info game result        : '<game nr> = {<ns>,<ew>,<ns result>,<ew result>[,\"ns contract\",\"ew contract\"]}[@ ...]'");
        file.AddLine(";info schema             : '{<nr of games>,<setsize>,<first game>}@{<nr of pairs>,<absent pair>,\"schema\",\"groupChars\"}[@...]'");
        file.AddLine(";info min/max club       : '{<min pairs needed>,<max pairs used>}'");
        file.AddLine(";info assignments        : '<global pairnr>[@...] -> array[<sessionpair>] = <global pairnr>'");
        file.AddLine(";info assignments names  : '<session name of global pair>[@...] -> array[<globalpair>] = <session name of global pair>'");
        file.AddLine(";info lines per page     : 'lines per page when printertype is <list>'");
        file.AddLine(";info corrections session: '{<session pairnr>,<correction><type>,<extra.1>,<max extra>,<games>}[@...]'");
        file.AddLine(";info corrections end    : '{<global pairnr>,<score.2>,<bonus.2>,<games>}[@...]'");
        file.AddLine(";info session result     : '{<global pairnr>,<score.2>,<games>}[@...]'");
        file.AddLine(";info session rank       : '<globalpair>[@...] array[<rank>]=<globalpair>'");
        file.AddLine(";info total rank         : '<globalpair>[@...] array[<rank>]=<globalpair>'");
        file.AddLine("[info_end]");
        file.Flush();
    }

    wxFileConfig* pCfg = new wxFileConfig(wxEmptyString, wxEmptyString, a_dbFile, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
    pCfg->SetRecordDefaults();   //write all requested entries to the file, if not present
    // set some info-records
    std::swap(s_pConfig, pCfg);       // have the correct value for the next two calls....
    long version = ReadValueLong(KEY_DB_VERSION , dbVersion);
    if ( version > dbVersion )
        MyMessageBox(wxString::Format(_("Database version error, expected <= %ld, found %ld"), dbVersion, version));
    (void) ReadValue    (KEY_PRG_VERSION, cfg::GetVersion());
    std::swap(s_pConfig, pCfg);       // back to how it was
    return pCfg;
}   // InitDatabase()

CfgFileEnum DatabaseOpen(io::GlbDbType /*dbType*/, CfgFileEnum a_how2Open)
{
    theSeparator = glb::GetSeparator();  // defined in the glb namespace
    wxFileName db(cfg::ConstructFilename(cfg::EXT_DATABASE));
    if ( db.GetFullPath() == sDbFile && s_pConfig != nullptr ) return CFG_OK;
    delete s_pConfig;   // will flush pending changes
    s_pConfig = nullptr;
    InitSdb();

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

    sDbFile = db.GetFullPath();
    s_pConfig = InitDatabase(sDbFile);
    return CFG_OK;
} // DatabaseOpen()

bool DatabaseFlush(io::GlbDbType /*dbType*/)
{
    if ( !s_pConfig ) return true;
    if ( s_pConfigGlobalNames ) s_pConfigGlobalNames->Flush();
    return s_pConfig->Flush();
}   // DatabaseFlush()

static void InitGlobalNames()
{   // use a global db for pairnames/clubnames
    if ( !cfg::GetGlobalNameUse() ) return;     // use names from active match db

    // swap db-pointers at begin and end of functions that access pairnames/clubnames
    if ( s_pConfigGlobalNames == nullptr )  // create/open global names db
        s_pConfigGlobalNames = InitDatabase(cfg::GetGlobalNameFile());
    std::swap(s_pConfigGlobalNames, s_pConfig);
}   // InitGlobalNames()

bool PairnamesWrite(const names::PairInfoData& a_pairInfo)
{
    if ( !s_pConfig ) return false;
    InitGlobalNames();  // use global db, if set so
    wxString path = MakePath(KEY_MATCH_PAIRNAMES);
    s_pConfig->DeleteGroup(path);
    s_pConfig->SetPath(path);
    UINT index = 0;
    bool bResult = true;
    for (const auto& it : a_pairInfo)
    {
        if ( index ) if ( !s_pConfig->Write(FMT("%u",index), FMT("{\"%s\",%u}", EncodeString(it.pairName), it.clubIndex)) ) bResult = false;
        ++index;
    }
    InitGlobalNames();  // back to local db
    return bResult;
} // PairnamesWrite()

bool PairnamesRead(names::PairInfoData& a_pairInfo)
{
    a_pairInfo.clear();
    a_pairInfo.resize(cfg::MAX_PAIRS+1);
    if ( !s_pConfig ) return false;

    InitGlobalNames();  // use global db, if set so
    wxString path = MakePath(KEY_MATCH_PAIRNAMES);
    ReadConfigGroup rd(s_pConfig, path);
    size_t maxPair = rd.GetNumberOfEntries();
    a_pairInfo.resize(maxPair+1);
    wxString key,value;
    while (rd.GetNextEntry(key, value))
    {
        UINT pair = wxAtoi(key);
        if ( pair > maxPair ) { MyLogError(_("Reading pairnames: pairnr <%s> too high!"), key); continue; }
        names::PairInfo info;
        char name[2*cfg::MAX_NAME_SIZE+1]={0};
        auto count = wxSscanf(value," { \"%50[^\"]\" , %u }", name, &info.clubIndex);   // hardcoded maxsize == 50!
        if ( count != 2 )
        {
            MyLogError(_("Reading pairnames: <%s = %s> invalid!"), key, value);
            continue;
        }
        info.pairName = DecodeString(name);
        a_pairInfo[pair] = info;
    }
    InitGlobalNames();  // back to local db
    return true;
} // PairnamesRead()

bool ClubnamesRead(std::vector<wxString>& a_clubNames, UINT& a_uMaxId)
{
    a_clubNames.clear();
    a_clubNames.resize(cfg::MAX_CLUBNAMES+1);
    a_uMaxId = cfg::MAX_CLUBID_UNION;    // 'free' added clubnames get an id starting here
    if ( !s_pConfig ) return false;

    InitGlobalNames();  // use global db, if set so
    wxString path = MakePath(KEY_MATCH_CLUBNAMES);
    ReadConfigGroup rd(s_pConfig, path);
    wxString key, value;
    while (rd.GetNextEntry(key, value))
    {
        UINT club = wxAtoi(key);
        if ( club > cfg::MAX_CLUBNAMES ) { MyLogError(_("Reading clubnames: clubnr <%u> too high!"), club ); continue; }
        char name[2*cfg::MAX_CLUB_SIZE+1]={0};
        auto count = wxSscanf(value, " \"%50[^\"]\" ", name);   // hardcoded maxsize == 50!
        if ( count != 1 )
        {
            MyLogError(_("Rading clubnames: <%s = %s> invalid!"), key, value);
            continue;
        }
        a_clubNames[club] = DecodeString(name);
        if ( club > a_uMaxId )
            a_uMaxId = club;
    }
    InitGlobalNames();  // back to local db
    return true;
} // ClubnamesRead()

bool ClubnamesWrite(const std::vector<wxString>& a_clubNames)
{
    if ( !s_pConfig ) return false;
    InitGlobalNames();  // use global db, if set so
    wxString path = MakePath(KEY_MATCH_CLUBNAMES);
    s_pConfig->DeleteGroup(path);
    s_pConfig->SetPath(path);
    UINT index = 0;
    bool bResult = true;
    for (const auto& it : a_clubNames)
    {
        if ( index && !it.IsEmpty() )
            if ( !s_pConfig->Write(FMT("%u",index), FMT("\"%s\"", EncodeString(it))) ) bResult = false;;
        ++index;
    }

    InitGlobalNames();  // back to local db
    return bResult;
} // ClubnamesWrite()

static bool CB_ScoresReadLine(wxString& a_game, wxString& a_gameScores, void* a_pUserData)
{   // read the scores of a single game
    auto pCfg = reinterpret_cast<ReadConfigGroup*>(a_pUserData);
    return pCfg->GetNextEntry(a_game, a_gameScores);
}   // CB_ScoresReadLine()

bool ScoresRead(vvScoreData& a_scoreData, UINT a_session)
{
    if ( !s_pConfig ) return false;
    wxString        path = MakePath(KEY_SESSION_GAMERESULT, a_session);
    ReadConfigGroup configGroup(s_pConfig, path);
    return glb::ScoresRead(a_scoreData, CB_ScoresReadLine, &configGroup);
} // ScoresRead()

static bool CB_ScoresWriteGame(UINT a_game, const wxString& a_gameScores, void* /*a_pUserData*/)
{   // write the scores of a single game

    return s_pConfig->Write(FMT("%u", a_game), a_gameScores);
}   // CB_ScoresWriteGame()

bool ScoresWrite(const vvScoreData& a_scoreData, UINT a_session)
{
    if ( !s_pConfig ) return false;
    wxString path = MakePath(KEY_SESSION_GAMERESULT, a_session);
    s_pConfig->DeleteGroup(path);
    s_pConfig->SetPath(path);
    return glb::ScoresWrite(a_scoreData, CB_ScoresWriteGame );
} // ScoresWrite()

void InitSdb()
{
    static bool bIsInited = false;
    if ( bIsInited ) return;
    bIsInited = true;

    dbKeys[KEY_DB_VERSION]               = "databaseVersion";
    dbKeys[KEY_PRG_VERSION]              = "programVersion";

    dbKeys[KEY_MATCH_MAX_ABSENT]         = "maxAbsent";
    dbKeys[KEY_MATCH_PRNT]               = "printer";
    dbKeys[KEY_MATCH_MAXMEAN]            = "maxMean";
    dbKeys[KEY_MATCH_CLOCK]              = "clock";
    dbKeys[KEY_MATCH_WEIGHTAVG]          = "weightedAverage";
    dbKeys[KEY_MATCH_LINESPP]            = "linesPerPage";
    dbKeys[KEY_MATCH_NEUBERG]            = "neuberg";
    dbKeys[KEY_MATCH_GRPRESULT]          = "groupResult";
    dbKeys[KEY_MATCH_MMCLUB]             = "minMaxClub";
    dbKeys[KEY_MATCH_GLOBALNAMES]        = "centralNames";
    dbKeys[KEY_MATCH_SESSION]            = "session";
    dbKeys[KEY_MATCH_PAIRNAMES]          = "pairNames";
    dbKeys[KEY_MATCH_CLUBNAMES]          = "clubNames";

    dbKeys[KEY_MATCH_CMNT]               = "comment";       // filter out: old
    dbKeys[KEY_MATCH_FF]                 = "form_feed";     // filter out: old
    dbKeys[KEY_MATCH_VIDEO]              = "bios_video";    // filter out: old
    dbKeys[KEY_MATCH_DISCR]              = "description";
    dbKeys[KEY_MATCH_BUTLER]             = "butler";

    dbKeys[KEY_SESSION_DISCR]            = "description";
    dbKeys[KEY_SESSION_SCHEMA]           = "schema";
    dbKeys[KEY_SESSION_GAMERESULT]       = "gameResult";
    dbKeys[KEY_SESSION_ASSIGNMENTS]      = "assignments";        // array of global pairnr's
    dbKeys[KEY_SESSION_ASSIGNMENTS_NAME] = "assignmentsName";    // array for global pairnr's of session names
    dbKeys[KEY_SESSION_CORRECTION]       = "correctionsSession"; // corrections for this session
    dbKeys[KEY_SESSION_CORRECTION_END]   = "correctionsEnd";     // corrections for total/end calculation
    dbKeys[KEY_SESSION_RESULT]           = "sessionResult";      // short session result, used for total/end calculation
    dbKeys[KEY_SESSION_RANK_SESSION]     = "sessionrank";        // rank in the sessionresult
    dbKeys[KEY_SESSION_RANK_TOTAL]       = "totalrank";          // rank in the totalresult for this session

}  // InitSdb()

wxString ReadValue(keyId a_id, const wxString& a_defaultValue, UINT a_session)
{
    if ( !s_pConfig ) return a_defaultValue;
    return DecodeString(s_pConfig->Read(MakePath(a_id, a_session), a_defaultValue));
}   // ReadValue()

bool ReadValueBool(keyId a_id, bool a_defaultValue, UINT a_session)
{
    if ( !s_pConfig ) return a_defaultValue;
    return s_pConfig->ReadBool(MakePath(a_id, a_session), a_defaultValue);
}   // ReadValueBool()

long ReadValueLong(keyId a_id, long a_defaultValue, UINT a_session)
{
    if ( !s_pConfig ) return a_defaultValue;
    return s_pConfig->ReadLong(MakePath(a_id, a_session), a_defaultValue);
}   // ReadValueLong()

UINT ReadValueUINT(keyId a_id, UINT a_defaultValue, UINT a_session)
{
    if ( !s_pConfig ) return a_defaultValue;
    return s_pConfig->ReadLong(MakePath(a_id, a_session), a_defaultValue);
}   // ReadValueUINT()

bool WriteValue(keyId a_id, const wxString& a_value, UINT a_session)
{
    if ( !s_pConfig ) return false;
    return s_pConfig->Write(MakePath(a_id, a_session), EncodeString(a_value));
}   // WriteValue()

bool WriteValue(keyId a_id, bool a_value, UINT a_session)
{
    if ( !s_pConfig ) return false;
    return s_pConfig->Write(MakePath(a_id, a_session), a_value);
}   // WriteValue()

bool WriteValue(keyId a_id, long a_value, UINT a_session)
{
    if ( !s_pConfig ) return false;
    return s_pConfig->Write(MakePath(a_id, a_session), a_value);
}   // WriteValue()

bool WriteValue(keyId a_id, UINT a_value, UINT a_session)
{
    if ( !s_pConfig ) return false;
    return s_pConfig->Write(MakePath(a_id, a_session), a_value);
}   // WriteValue()

bool SchemaRead(cfg::SessionInfo& a_info, UINT a_session)
{
    if ( !s_pConfig ) return false;
    wxString defaultValue = glb::GetDefaultSchema();
    wxString schema = s_pConfig->Read(MakePath(KEY_SESSION_SCHEMA, a_session), defaultValue);
    return glb::SchemaRead(a_info, schema);
}  // SchemaRead()

bool SchemaWrite(const cfg::SessionInfo& a_info, UINT a_session)
{
    if ( !s_pConfig ) return false;
    wxString info = glb::SchemaWrite(a_info);
    return s_pConfig->Write(MakePath(KEY_SESSION_SCHEMA, a_session), info);
}   // SchemaWrite()

bool MaxmeanRead(UINT& a_maxmean)
{
    if ( !s_pConfig ) return false;
    wxString defaultValue = FMT("%u.%02u", a_maxmean / 100, a_maxmean % 100);
    wxString sMaxMean = s_pConfig->Read(MakePath(KEY_MATCH_MAXMEAN), defaultValue);
    a_maxmean = AsciiTolong(sMaxMean, ExpectedDecimalDigits::DIGITS_2);
    return true;
}   // MaxmeanRead()

bool MaxmeanWrite(UINT a_maxmean)
{
    if ( !s_pConfig ) return false;
    wxString sMaxmean = FMT("%u.%02u", a_maxmean / 100, a_maxmean % 100);
    return s_pConfig->Write(MakePath(KEY_MATCH_MAXMEAN), sMaxmean);
} // MaxmeanWrite()

bool MinMaxClubRead(UINT& a_min, UINT& a_max)
{
    if ( !s_pConfig ) return false;
    wxString defaultValue = FMT("{%u,%u}", a_min, a_max);
    wxString sMinMax = s_pConfig->Read(MakePath(KEY_MATCH_MMCLUB), defaultValue);
    auto count = wxSscanf(sMinMax, " {%u ,%u }", &a_min, &a_max);
    return count == 2;
} // MinMaxClubRead()

bool MinMaxClubWrite(UINT a_min, UINT a_max)
{
    if ( !s_pConfig ) return false;
    wxString sMinMax = FMT("{%u,%u}", a_min, a_max);
    return s_pConfig->Write(MakePath(KEY_MATCH_MMCLUB), sMinMax);
} // MinMaxClubWrite()

bool Session2GlobalIdsRead(UINT_VECTOR& a_vuPairnrSession2Global, UINT a_session)
{
    // The file has the pairs based on sessionpairnr order, entry zero = dummy
    // pairIndex[x]: sessionPairNr 'x' is mapped to globalPairNr 'pairIndex[x]'
    // example: pairIndex[0,3,20,5,7] -> sessionpair 1 <==> globalpair 3, sp 4 <==> gp 7

    return UintVectorRead(a_vuPairnrSession2Global, a_session, KEY_SESSION_ASSIGNMENTS);
} // Session2GlobalIdsRead()

bool Session2GlobalIdsWrite(const UINT_VECTOR& a_vuPairnrSession2Global, UINT a_session)
{
    return UintVectorWrite(a_vuPairnrSession2Global, a_session, KEY_SESSION_ASSIGNMENTS);
}   // Session2GlobalIdsWrite()

bool SessionNamesWrite(const wxArrayString& a_names, UINT a_session)
{   // sessionnames for globalpairnrs of a session
    if ( !s_pConfig ) return false;
    wxString info = glb::SessionNamesWrite(a_names);
    return s_pConfig->Write(MakePath(KEY_SESSION_ASSIGNMENTS_NAME, a_session), info);
}   // SessionNamesWrite()

bool SessionNamesRead(wxArrayString& a_assignmentsName, UINT a_session)
{
    if ( !s_pConfig ) return false;
    wxString info = s_pConfig->Read(MakePath(KEY_SESSION_ASSIGNMENTS_NAME, a_session), ES);
    return glb::SessionNamesRead(a_assignmentsName, info);
}   // SessionNamesRead()

bool CorrectionsSessionRead(cor::mCorrectionsSession& a_mCorrectionsSession, UINT a_session)
{
    a_mCorrectionsSession.clear();
    if ( !s_pConfig ) return false;
    wxString corrections = s_pConfig->Read(MakePath(KEY_SESSION_CORRECTION, a_session), ES);
    return glb::CorrectionsSessionRead(a_mCorrectionsSession, corrections);
}   // CorrectionsSessionRead()

bool CorrectionsSessionWrite(const cor::mCorrectionsSession& a_m_correctionsSession, UINT a_session)
{
    if ( !s_pConfig ) return false;
    wxString correction = glb::CorrectionsSessionWrite(a_m_correctionsSession);
    return s_pConfig->Write(MakePath(KEY_SESSION_CORRECTION, a_session), correction);
}   // CorrectionsSessionWrite()

bool CorrectionsEndRead(cor::mCorrectionsEnd& a_mCorrectionsEnd, UINT a_session, bool a_bEdit)
{
    if ( !s_pConfig ) return false;
    wxString data = s_pConfig->Read(MakePath(KEY_SESSION_CORRECTION_END, a_session), ES);
    return glb::CorrectionsEndRead(a_mCorrectionsEnd, a_bEdit, data);
}   // CorrectionsEndRead()

bool CorrectionsEndWrite(const cor::mCorrectionsEnd& a_m_correctionsEnd, UINT a_session)
{
    if ( !s_pConfig ) return false;
    wxString correction = glb::CorrectionsEndWrite(a_m_correctionsEnd);
    return s_pConfig->Write(MakePath(KEY_SESSION_CORRECTION_END, a_session), correction);
}   // CorrectionsEndWrite()

bool SessionResultRead(cor::mCorrectionsEnd& a_mSessionResult, UINT a_session)    // write and read: different params!
{   // NB input MAP is initialized, so don't clear or resize it or add 'new' pairnrs!
    if ( !s_pConfig ) return false;
    wxString info = s_pConfig->Read(MakePath(KEY_SESSION_RESULT, a_session));
    return glb::SessionResultRead(a_mSessionResult, info);
} //SessionResultRead()

bool SessionResultWrite(const cor::mCorrectionsEnd& a_mSessionResult, UINT a_session)
{
    if ( !s_pConfig ) return false;
    wxString result = glb::SessionResultWrite(a_mSessionResult);
    return s_pConfig->Write(MakePath(KEY_SESSION_RESULT, a_session), result);
}   // SessionResultWrite()

bool SessionRankRead(UINT_VECTOR& a_vuRank, UINT a_session)
{
    return UintVectorRead(a_vuRank, a_session, KEY_SESSION_RANK_SESSION);
}   // SessionRankRead()

bool SessionRankWrite(const UINT_VECTOR& a_vuRank, UINT a_session)
{
    return UintVectorWrite(a_vuRank, a_session, KEY_SESSION_RANK_SESSION);
}   // SessionRankWrite()

bool TotalRankRead(UINT_VECTOR& a_vuRank, UINT a_session)
{
    return UintVectorRead(a_vuRank, a_session, KEY_SESSION_RANK_TOTAL);
}   // TotalRankRead()

bool TotalRankWrite(const UINT_VECTOR& a_vuRank, UINT a_session)
{
    return UintVectorWrite(a_vuRank, a_session, KEY_SESSION_RANK_TOTAL);
}   // TotalRankWrite()

bool UintVectorRead(UINT_VECTOR& a_vUint, UINT a_session, keyId a_id)
{   // Resize vector to cfg::MAX_PAIRS and read a set of UINTs and put them in a vector.
    if ( !s_pConfig ) return false;
    wxString key  = MakePath(a_id, a_session);
    wxString info = s_pConfig->Read(key, ES);
    return glb::UintVectorRead(a_vUint, info, sDbFile, key, _("database error"));
}   //UintVectorRead()

bool UintVectorWrite(const UINT_VECTOR& a_vUint, UINT a_session, keyId a_id)
{   // write contents of vector as UINT, ignoring entry 0
    if ( !s_pConfig ) return false;
    wxString  key = MakePath(a_id, a_session);
    wxString info = glb::UintVectorWrite(a_vUint);
    return s_pConfig->Write(key, info);
}   // UintVectorWrite()

#if TEST
static void DoGroup(wxFileConfig& config, wxString group)
{
    long group_index;
    config.SetPath(group);
    wxString base = group;
    wxString key;
    long entry_index;
    for (bool bHasNextEntry = config.GetFirstEntry(key, entry_index);
        bHasNextEntry;
        bHasNextEntry = config.GetNextEntry(key, entry_index))
    {   // show entries in current group
        wxString value = config.Read(key, ES);
        wxMessageOutputDebug d;
        d.Printf("[%s] %s = %s", group, key, value);
    }
    for (bool bHasSubGroup = config.GetFirstGroup(group, group_index);
        bHasSubGroup;
        bHasSubGroup = config.GetNextGroup(group, group_index))
    {   // handle all sub-groups
        DoGroup(config, base + '/' + group); // search for entries/groups within this group
        config.SetPath(base);   // need to restore path!
    }
}

static void Test()
{
    wxString path( "/");
    s_pConfig->Write("/2/testWrite","testWriteäÖöéë");
    s_pConfig->Flush();
    DoGroup(*s_pConfig, path);
    Test2();
}

void Test2()
{
    wxString t1("1 @{1,2,3} @ {3,5,-1,8} @ {1,2,3,\"abc\"}");
    auto split = wxSplit(t1,'@');
    split;
}
#endif // TEST_
} // end namespace db
