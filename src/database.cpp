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
#include "baseframe.h"
#include "cfg.h"

namespace db
{
static const long dbVersion = 100;
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
static const wxChar             theSeparator = '@';     // default separator


class ReadConfigGroup
{   // support class for easy getting all entries in a group
public:
    ReadConfigGroup(wxFileConfig* pConfig, const wxString& path)
    {
        m_pConfig    = pConfig;
        m_bFirst    = true;
        m_bHasNext  = false;
        m_index     = 0;
        m_pConfig->SetPath(path);
    }

    bool GetNextEntry(wxString& key, wxString& value)
    {
        m_bHasNext = m_bFirst ? m_pConfig->GetFirstEntry(key, m_index) : m_pConfig->GetNextEntry(key, m_index);
        value = m_bHasNext ? m_pConfig->Read(key, ES) : ES;
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

static wxString MakePath(keyId id, UINT session = DEFAULT_SESSION )
{
    return session == DEFAULT_SESSION ? 
        "/main/" + dbKeys[id]
      : FMT("/%u/%s", session, dbKeys[id]);
}   // MakePath()

bool ExistSession(UINT a_session)
{
    return DatabaseIsOpen() && s_pConfig->HasGroup(FMT("/%u", a_session));
}   // ExistSession()

bool UintVectorWrite(const UINT_VECTOR& vUint, UINT session, keyId id);
bool UintVectorRead(       UINT_VECTOR& vUint, UINT session, keyId id);

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
    if (!wxFile::Exists(a_dbFile))
    {   // create the file with some initial comments, while we can't write comments to a configfile, can we???
        wxString cpy=cfg::GetCopyrightDateTime(); cpy.Replace("\n", ES);
        MyTextFile file;
        file.MyCreate(a_dbFile, MyTextFile::WRITE);
        file.AddLine("[info_begin]");
        file.AddLine(";copyright               : " + cpy);
        file.AddLine(";info database           : 'data saved according definition of version <x>'");
        file.AddLine(";info pair names         : '<global pairnr> = {<name>,<clubId>}, max size=30'");
        file.AddLine(";info club names         : '<club id> = \"club name\", max size=25'");
        file.AddLine(";info game result        : '<game nr> = {<ns>,<ew>,<ns result>,<ew result>}[@ ...]'");
        file.AddLine(";info schema             : '{<nr of games>,<setsize>,<first game>}@{<nr of pairs>,<absent pair>,\"schema\",\"groupChars\"}[@...]'");
        file.AddLine(";info min/max club       : '{<min pairs needed>,<max pairs used>}'");
        file.AddLine(";info assignments        : '<global pairnr>[@...] -> array[<sessionpair>] = <global pairnr>'");
        file.AddLine(";info assignments names  : '<session name of global pair>[@...] -> array[<globalpair>] = <session name of global pair>'");
        file.AddLine(";info lines per page     : 'lines per page when printertype is <list>'");
        file.AddLine(";info corrections session: '{<session pairnr>,<correction><type>,<extra.1>,<max extra>}[@...]'");
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
    (void) ReadValueLong(KEY_DB_VERSION , dbVersion);
    (void) ReadValue    (KEY_PRG_VERSION, cfg::GetVersion());
    std::swap(s_pConfig, pCfg);       // back to how it was
    return pCfg;
}   // InitDatabase()

CfgFileEnum DatabaseOpen(io::GlbDbType /*dbType*/, CfgFileEnum a_how2Open)
{
    wxFileName db(cfg::ConstructFilename(cfg::EXT_DATABASE));
    if (db.GetFullPath() == sDbFile && s_pConfig != nullptr) return CFG_OK;
    delete s_pConfig;   // will flush pending changes
    s_pConfig = nullptr;
    InitSdb();

    bool bFileExist = wxFile::Exists(db.GetFullPath());
    if ( (a_how2Open == CFG_ONLY_READ) && !bFileExist)
    {   // only handle file if it already exists, so don't create it yet
        return CFG_ERROR;
    }

    if (!db.DirExists())
        (void)db.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);     // create full path, no error if allready there
    if (!db.IsDirWritable())
    {
        // if not writable, select "C:\\Users\\<me>\\AppData\\Local\\BridgeWx" dir
        wxString err = db.GetFullPath() + _(": is not accessable!");
        MyLogError( err );      // wxLogError will stop the programm with a pop-up...
        MyMessageBox(err);
        wxString dir = cfg::GetBaseFolder();
        cfg::SetActiveMatch(db.GetFullName(), dir);
        db = wxFileName(dir, db.GetFullName());
    }

    sDbFile = db.GetFullPath();
    s_pConfig = InitDatabase(sDbFile);
    return CFG_OK;
} // DatabaseOpen()

bool DatabaseFlush(io::GlbDbType /*dbType*/)
{
    if (!s_pConfig) return true;
    if (s_pConfigGlobalNames) s_pConfigGlobalNames->Flush();
    return s_pConfig->Flush();
}   // DatabaseFlush()

static void InitGlobalNames()
{   // use a global db for pairnames/clubnames
    if (!cfg::GetGlobalNameUse()) return;   // use names from active match db

    // swap db-pointers at begin and end of functions that access pairnames/clubnames
    if (s_pConfigGlobalNames == nullptr)    // create/open global names db
        s_pConfigGlobalNames = InitDatabase(cfg::GetGlobalNameFile());
    std::swap(s_pConfigGlobalNames, s_pConfig);
}   // InitGlobalNames()

bool PairnamesWrite(const names::PairInfoData& pairInfo)
{
    if (!s_pConfig) return false;
    InitGlobalNames();  // use global db, if set so
    wxString path = MakePath(KEY_MATCH_PAIRNAMES);
    s_pConfig->DeleteGroup(path);
    s_pConfig->SetPath(path);
    UINT index = 0;
    bool bResult = true;
    for (const auto& it : pairInfo)
    {
        if (index) if (!s_pConfig->Write(FMT("%u",index), FMT("{\"%s\",%u}", it.pairName, it.clubIndex))) bResult = false;
        ++index;
    }
    InitGlobalNames();  // back to local db
    return bResult;
} // PairnamesWrite()

bool PairnamesRead(names::PairInfoData& pairInfo)
{
    pairInfo.clear();
    pairInfo.resize(cfg::MAX_PAIRS+1);
    if (!s_pConfig) return false;

    InitGlobalNames();  // use global db, if set so
    wxString path = MakePath(KEY_MATCH_PAIRNAMES);
    ReadConfigGroup rd(s_pConfig, path);
    size_t maxPair = rd.GetNumberOfEntries();
    pairInfo.resize(maxPair+1);
    wxString key,value;
    while (rd.GetNextEntry(key, value))
    {
        UINT pair = wxAtoi(key);
        if (pair > maxPair ) { MyLogError(_("Reading pairnames: pairnr <%s> too high!"), key); continue; }
        names::PairInfo info;
        char name[2*cfg::MAX_NAME_SIZE+1]={0};
        auto count = wxSscanf(value," { \"%50[^\"]\" , %u }", name, &info.clubIndex);   // hardcoded maxsize == 50!
        if (count != 2)
        {
            MyLogError(_("Reading pairnames: <%s = %s> invalid!"), key, value);
            continue;
        }
        info.pairName = name;
        pairInfo[pair]=info;
    }
    InitGlobalNames();  // back to local db
    return true;
} // PairnamesRead()

bool ClubnamesRead(std::vector<wxString>& clubNames, UINT& uMaxId)
{
    clubNames.clear();
    clubNames.resize(cfg::MAX_CLUBNAMES+1);
    uMaxId = cfg::MAX_CLUBID_UNION;    // 'free' added clubnames get an id starting here
    if (!s_pConfig) return false;

    InitGlobalNames();  // use global db, if set so
    wxString path = MakePath(KEY_MATCH_CLUBNAMES);
    ReadConfigGroup rd(s_pConfig, path);
    wxString key, value;
    while (rd.GetNextEntry(key, value))
    {
        UINT club = wxAtoi(key);
        if (club > cfg::MAX_CLUBNAMES ) { MyLogError(_("Reading clubnames: clubnr <%u> too high!"), club); continue; }
        char name[2*cfg::MAX_CLUB_SIZE+1]={0};
        auto count = wxSscanf(value, " \"%50[^\"]\" ", name);   // hardcoded maxsize == 50!
        if (count != 1)
        {
            MyLogError(_("Rading clubnames: <%s = %s> invalid!"), key, value);
            continue;
        }
        clubNames[club]=name;
        if (club > uMaxId)
            uMaxId = club;
    }
    InitGlobalNames();  // back to local db
    return true;
} // ClubnamesRead()

bool ClubnamesWrite(const std::vector<wxString>& clubNames)
{
    if (!s_pConfig) return false;
    InitGlobalNames();  // use global db, if set so
    wxString path = MakePath(KEY_MATCH_CLUBNAMES);
    s_pConfig->DeleteGroup(path);
    s_pConfig->SetPath(path);
    UINT index = 0;
    bool bResult = true;
    for (const auto& it : clubNames)
    {
        if (index && !it.IsEmpty()) 
            if (!s_pConfig->Write(FMT("%u",index), FMT("\"%s\"", it))) bResult = false;;
        ++index;
    }

    InitGlobalNames();  // back to local db
    return bResult;
} // ClubnamesWrite()

bool ScoresRead(vvScoreData& scoreData, UINT session)
{
    scoreData.clear();                  // remove old data
    scoreData.resize(cfg::MAX_GAMES+1); //  and assure room voor all games. We depend on the entries of this vector!
    if (!s_pConfig) return false;

    wxString path = MakePath(KEY_SESSION_GAMERESULT, session);
    ReadConfigGroup rd(s_pConfig, path);
    wxString key, value;
    while (rd.GetNextEntry(key, value))
    {
        UINT game = wxAtoi(key);
        if (game > cfg::MAX_GAMES ) { MyLogError(_("Reading scores: gamenr <%s> too high!"), key); continue; }
        auto splitValues = wxSplit(value, theSeparator);
        std::vector<score::GameSetData> gameData;
        score::GameSetData setData;
        for (const auto& it : splitValues)
        {
            char nsScore[10]={0};
            char ewScore[10]={0};
            auto count = wxSscanf(it," { %u , %u, %9[^, ] , %9[^} ] }", &setData.pairNS, &setData.pairEW, nsScore, ewScore);
            if (count != 4)
            {
                MyLogError(_("Reading scores: game %u: <%s> invalid!"), game, it);
                continue;
            }
            setData.scoreNS = score::ScoreFromString(nsScore);
            setData.scoreEW = score::ScoreFromString(ewScore);
            gameData.push_back(setData);
        }
        scoreData[game] = gameData;
    }
    return true;
} // ScoresRead()

bool ScoresWrite(const vvScoreData& scoreData, UINT session)
{
    if (!s_pConfig) return false;
    wxString path = MakePath(KEY_SESSION_GAMERESULT, session);
    s_pConfig->DeleteGroup(path);
    s_pConfig->SetPath(path);
    bool bResult = true;
    UINT game = 0;
    for (const auto& it : scoreData)
    {
        if (game && it.size() )
        {   // only games with data
            wxString theScores;
            wxChar separator = ' ';
            for (auto score : it)
            {
                theScores += FMT("%c{%u,%u,%s,%s}", separator, score.pairNS, score.pairEW,
                        score::ScoreToString(score.scoreNS), score::ScoreToString(score.scoreEW));
                separator = theSeparator;
            }
            if (!s_pConfig->Write(FMT("%u", game), theScores))
                bResult = false;;
        }
        ++game;
    }

    return bResult;
} // ScoresWrite()

void InitSdb()
{
    static bool bIsInited = false;
    if (bIsInited) return;
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

wxString ReadValue(keyId id, const wxString& default, UINT session)
{
    if (!s_pConfig) return default;
    return s_pConfig->Read(MakePath(id,session), default);
}   // ReadValue()

bool ReadValueBool(keyId id, bool default, UINT session)
{
    if (!s_pConfig) return default;
    return s_pConfig->ReadBool(MakePath(id,session), default);
}   // ReadValueBool()

long ReadValueLong(keyId id, long default, UINT session)
{
    if (!s_pConfig) return default;
    return s_pConfig->ReadLong(MakePath(id,session), default);
}   // ReadValueLong()

UINT ReadValueUINT(keyId id, UINT default, UINT session)
{
    if (!s_pConfig) return default;
    return s_pConfig->ReadLong(MakePath(id,session), default);
}   // ReadValueUINT()

bool WriteValue(keyId id, const wxString& value, UINT session)
{
    if (!s_pConfig) return false;
    return s_pConfig->Write(MakePath(id,session), value);
}   // WriteValue()

bool WriteValue(keyId id, bool value, UINT session)
{
    if (!s_pConfig) return false;
    return s_pConfig->Write(MakePath(id,session), value);
}   // WriteValue()

bool WriteValue(keyId id, long value, UINT session)
{
    if (!s_pConfig) return false;
    return s_pConfig->Write(MakePath(id,session), value);
}   // WriteValue()

bool WriteValue(keyId id, UINT value, UINT session)
{
    if (!s_pConfig) return false;
    return s_pConfig->Write(MakePath(id,session), value);
}   // WriteValue()

bool SchemaRead(cfg::SessionInfo& a_info, UINT a_session)
{
    if (!s_pConfig) return false;
    wxString default = FMT("{24,4,1}%c{14,0,\"6multi14\",\"\"}", theSeparator);   // remark: cannot scan empty sets!
    wxString info = s_pConfig->Read(MakePath(KEY_SESSION_SCHEMA, a_session), default);
    if (info.IsEmpty()) info = default;
    auto count = wxSscanf(info, " {%u ,%u ,%u }", &a_info.nrOfGames, &a_info.setSize, &a_info.firstGame);
    if ( count != 3)
    {   // on error, we just take a default value
        MyLogError(_("Error while reading schema <%s>"), info);
        info             = default;
        a_info.nrOfGames = 24;
        a_info.setSize   = 4;
        a_info.firstGame = 1;
    }
    auto split = wxSplit(info, theSeparator);
    split.erase(split.begin()); // now we have only schema descriptions
    a_info.groupData.clear();
    cfg::GROUP_DATA groupData;
    UINT groupOffset = 0;
    for (const auto& it : split )
    {
        char schema    [20]={0};
        char groupChars[20]={0};
        groupData.pairs = 0;
        groupData.absent = 0;
        count = wxSscanf(it, " {%u ,%u , \"%19[^\"]\" , \"%19[^\"]\" }", &groupData.pairs, &groupData.absent, schema, groupChars);
        // count == 2 -> empty schema
        // count == 3 -> empty groupchars
        if (count == 2)
        {   // no schema, but perhaps groupchars
            count = wxSscanf(it, " {%u ,%u , \"\" , \"%19[^\"]\" }", &groupData.pairs, &groupData.absent,  groupChars);
        }
        if ( count < 3)
        {   // schema and groupchars empty
            MyLogError(_("Error while reading schema <%s>"), info);
        }
        if ((groupChars[0] == ' ') && (groupChars[1] == 0) ) groupChars[0] = 0; // remove single space
        groupData.schema        = schema;
        groupData.groupChars    = groupChars;
        groupData.schemaId      = schema::GetId(groupData.schema);
        groupData.groupOffset   = groupOffset;
        groupOffset            += groupData.pairs;
        a_info.groupData.push_back(groupData);
    }
    return true;
}  // SchemaRead()

bool SchemaWrite(const cfg::SessionInfo& a_info, UINT a_session)
{
    if (!s_pConfig) return false;
    wxString info = FMT("{%u,%u,%u}", a_info.nrOfGames, a_info.setSize, a_info.firstGame);
    for (const auto& it : a_info.groupData)
    {
        wxString groupChars=it.groupChars;
//        if (groupChars.IsEmpty()) groupChars = " "; // can't scan empty strings....
        info += FMT("%c{%u,%u,\"%s\",\"%s\"}", theSeparator, it.pairs, it.absent, it.schema, groupChars);
    }

    return s_pConfig->Write(MakePath(KEY_SESSION_SCHEMA, a_session), info);
}// SchemaWrite()

bool MaxmeanRead(UINT& maxmean)
{
    if (!s_pConfig) return false;
    wxString default = FMT("%u.%02u", maxmean / 100, maxmean % 100);
    wxString sMaxMean = s_pConfig->Read(MakePath(KEY_MATCH_MAXMEAN), default);
    maxmean = AsciiTolong(sMaxMean, ExpectedDecimalDigits::DIGITS_2);
    return true;
}   // MaxmeanRead()

bool MaxmeanWrite(UINT maxmean)
{
    if (!s_pConfig) return false;
    wxString sMaxmean = FMT("%u.%02u", maxmean / 100, maxmean % 100);
    return s_pConfig->Write(MakePath(KEY_MATCH_MAXMEAN), sMaxmean);
} // MaxmeanWrite()

bool MinMaxClubRead(UINT& min, UINT& max)
{
    if (!s_pConfig) return false;
    wxString default = FMT("{%u,%u}", min, max);
    wxString sMinMax = s_pConfig->Read(MakePath(KEY_MATCH_MMCLUB), default);
    auto count = wxSscanf(sMinMax, " {%u ,%u }", &min, &max);
    return count == 2;
} // MinMaxClubRead()

bool MinMaxClubWrite(UINT min, UINT max)
{
    if (!s_pConfig) return false;
    wxString sMinMax = FMT("{%u,%u}", min, max);
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
    if (!s_pConfig) return false;
    int max;
    for ( max = a_names.size()-1; max > 0; --max)
    {   // no const reverse-iterator for wxArrayString....
        if (a_names[max] != names::GetNotSet() ) break;
    }
    wxString info;
    wxChar separator = ' ';
    for (int pair=1; pair <= max; ++pair)
    {
        info += FMT("%c%s", separator, a_names[pair]);
        separator = theSeparator;
    }
    return s_pConfig->Write(MakePath(KEY_SESSION_ASSIGNMENTS_NAME, a_session), info);
}   // SessionNamesWrite()

bool SessionNamesRead(wxArrayString& a_assignmentsName, UINT a_session)
{
    if (!s_pConfig) return false;
    wxString info = s_pConfig->Read(MakePath(KEY_SESSION_ASSIGNMENTS_NAME, a_session), ES);
    a_assignmentsName = wxSplit(info, theSeparator);
    a_assignmentsName.insert(a_assignmentsName.begin(),names::GetNotSet());
    a_assignmentsName.resize(cfg::MAX_PAIRS +1, names::GetNotSet());
    a_assignmentsName[1].Trim(TRIM_LEFT);   // first name has a ' ' in front of it
    return true;
}   // SessionNamesRead()

bool CorrectionsSessionRead(cor::mCorrectionsSession& a_mCorrectionsSession, UINT a_session)
{
    a_mCorrectionsSession.clear();
    if (!s_pConfig) return false;
    wxString corrections = s_pConfig->Read(MakePath(KEY_SESSION_CORRECTION, a_session), ES);
    if (corrections.IsEmpty()) return true;
    bool bResult = true;    // assume all is ok
    auto split = wxSplit(corrections, theSeparator);
    for (const auto& it : split)
    {
        UINT sessionPairnr;
        cor::CORRECTION_SESSION cs;
        char extraBuf[10+1]= {0};
        // {<session pairnr>,<correction><type>,<extra.1>,<max extra>}
        bool bErrorEntry = (5 != wxSscanf(it, " {%u ,%i %c , %10[^, ] ,%i }"
                                , &sessionPairnr, &cs.correction, &cs.type, extraBuf, &cs.maxExtra)
                            );
        cs.extra = AsciiTolong(extraBuf);
        if (!IsValidCorrectionSession(sessionPairnr, cs, it, bErrorEntry))
        {
            bResult = false;
        }
        else
        {   // add info to map
            a_mCorrectionsSession[sessionPairnr] = cs;
        }
    }
    return bResult;
}   // CorrectionsSessionRead()

bool CorrectionsSessionWrite(const cor::mCorrectionsSession& a_m_correctionsSession, UINT a_session)
{
    if (!s_pConfig) return false;
    wxString correction;
    wxChar separator = ' ';
    for (const auto& it : a_m_correctionsSession)
    {
        correction += FMT("%c{%u,%+i%c,%s,%i}"
            , separator
            , it.first
            , it.second.correction
            , it.second.type
            , LongToAscii1(it.second.extra).Trim(TRIM_RIGHT)
            , it.second.maxExtra
        );
        separator = theSeparator;
    }
    return s_pConfig->Write(MakePath(KEY_SESSION_CORRECTION, a_session), correction);
}   // CorrectionsSessionWrite()

bool CorrectionsEndRead(cor::mCorrectionsEnd& a_mCorrectionsEnd, UINT a_session, bool a_bEdit)
{
    if (!s_pConfig) return false;
    wxString info = s_pConfig->Read(MakePath(KEY_SESSION_CORRECTION_END, a_session), ES);
    if (info.IsEmpty()) return true;    // no results, but no error!
    auto split = wxSplit(info, theSeparator);
    bool bOk = true;
    for (const auto& it : split)
    {   // {<global pairnr>,<score.2>,<bonus.2>,<games>}
        cor::CORRECTION_END ce;
        char scoreBuf[10+1]={0};
        char bonusBuf[10+1]={0};
        UINT pairNr;
        bool bItemError = (4 != wxSscanf(it, " {%u , %10[^, ] , %10[^, ] ,%u }", &pairNr, scoreBuf, bonusBuf, &ce.games));
        ce.score = AsciiTolong(scoreBuf, ExpectedDecimalDigits::DIGITS_2);
        ce.bonus = AsciiTolong(bonusBuf, ExpectedDecimalDigits::DIGITS_2);
        if (cor::IsValidCorrectionEnd(pairNr, ce, it, bItemError) )
        {   // only add result if editing (map == empty) or when pair exists in supplied map
            if ( a_bEdit || a_mCorrectionsEnd.find(pairNr) != a_mCorrectionsEnd.end())
            {
                if (!a_bEdit && ce.score == SCORE_IGNORE)
                    ce.score = a_mCorrectionsEnd[pairNr].score;
                a_mCorrectionsEnd[pairNr] = ce;
            }
        }
        else bOk = false;
    }
    return bOk;
}   // CorrectionsEndRead()

bool CorrectionsEndWrite(const cor::mCorrectionsEnd& a_m_correctionsEnd, UINT a_session)
{
    if (!s_pConfig) return false;
    wxString correction;
    wxChar separator = ' ';
    for (const auto& it : a_m_correctionsEnd)
    {
        correction += FMT("%c{%u,%s,%s,%u}"
            , separator
            , it.first
            , LongToAscii2(it.second.score)
            , LongToAscii2(it.second.bonus)
            , it.second.games
        );
        separator = theSeparator;
    }
    return s_pConfig->Write(MakePath(KEY_SESSION_CORRECTION_END, a_session), correction);
}   // CorrectionsEndWrite()

bool SessionResultRead(cor::mCorrectionsEnd& a_mSessionResult, UINT a_session)    // write and read: different params!
{   // NB input MAP is initialized, so don't clear or resize it or add 'new' pairnrs!
    if (!s_pConfig) return false;
    wxString info = s_pConfig->Read(MakePath(KEY_SESSION_RESULT, a_session));
    if (info.IsEmpty()) return true;    // no results, but no error!
    bool bResult = true;    // assume all is ok
    auto split = wxSplit(info, theSeparator);
    for (const auto& it : split)
    {   // {<global pairnr>,<score.2>,<games>}
        cor::CORRECTION_END ce;
        char scoreBuf[10+1]={0};
        UINT pairNr;
        bool bItemError = (3 != wxSscanf(it, " {%u , %10[^, ] , %u }", &pairNr, scoreBuf, &ce.games));
        ce.score = AsciiTolong(scoreBuf, ExpectedDecimalDigits::DIGITS_2);
        if (a_mSessionResult.find(pairNr) == a_mSessionResult.end())
            bItemError = true;  // pair MUST be present!
        if (cor::IsValidCorrectionEnd(pairNr, ce, it, bItemError) )
        {
            a_mSessionResult[pairNr] = ce;
        }
        else bResult = false;

    }
    return bResult;
} //SessionResultRead()

bool SessionResultWrite(const cor::mCorrectionsEnd& a_mSessionResult, UINT a_session)
{
    if (!s_pConfig) return false;
    wxString result;
    wxChar separator = ' ';
    for (const auto& it : a_mSessionResult)              // save score of all pairs
    {   
        result += FMT("%c{%u,%s,%u}"
                    , separator
                    , it.first                          // global pairnr
                    , LongToAscii2(it.second.score)     // score
                    , it.second.games                   // played games
                );
        separator = theSeparator;
    }

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
    if (!s_pConfig) return false;
    bool bOk = true;
    a_vUint.clear();
    a_vUint.resize(cfg::MAX_PAIRS+1ULL, 0);
    wxString info = s_pConfig->Read(MakePath(a_id, a_session), ES);
    auto split = wxSplit(info, theSeparator);
    UINT entry = 1;  // entry zero is a  dummy
    UINT max = names::GetNumberOfGlobalPairs();
    for (const auto& it : split)
    {
        UINT value = (UINT)wxAtoi(it);
        if (value > max)
        {
            wxString err=FMT(_(" Error in database <%s> value in key <%s>: <%i> higher then max <%u>\nWill be ignored.")
                                , sDbFile, MakePath(a_id, a_session)
                                , (int)value, max);
            MyLogError("%s",err);
            MyMessageBox(err, _("database error"));
            bOk = false;
            continue;
        }
        a_vUint[entry] = value;
        if (++entry > cfg::MAX_PAIRS)
            break;
    }
    return bOk;
}   //UintVectorRead()

bool UintVectorWrite(const UINT_VECTOR& a_vUint, UINT a_session, keyId a_id)
{   // write contents of vector as UINT, ignoring entry 0
    if (!s_pConfig) return false;
    wxString info;
    auto it = std::find_if(a_vUint.rbegin(), a_vUint.rend(), [](UINT pair){return pair != 0U ;} );
    int maxPair = a_vUint.rend() - it - 1;  // index of highest pair in session, -1 if none found

    wxChar separator = ' ';
    for ( int pair = 1; pair <= maxPair; ++pair)
    {
        info += FMT("%c%u", separator, a_vUint[pair]);
        separator = theSeparator;
    }
    return s_pConfig->Write(MakePath(a_id, a_session), info);
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
