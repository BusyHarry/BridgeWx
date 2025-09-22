// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/string.h>
#include <wx/wxcrtvararg.h>
#include <wx/fileconf.h>
#include <wx/ffile.h>

#include "orgInterface.h"

#define TEST 0      /* filenames get extra extension ".tst" */
/*
* This files has the functions to read/write 'old' type of config files.
* Newer setup uses 1 database for all data that is needed from one session to another.
*/

namespace  org
{
    // last values in use for binairy config-files
    constexpr UINT MAX_PAIRS3       = 120 ; //cfg::MAX_PAIRS;       // nr of pairs (120)
    constexpr UINT MAX_GAMES3       = 32;   //cfg::MAX_GAMES
    constexpr UINT MAX_GROUPS3      = 9;    //cfg::MAX_GROUPS
    constexpr UINT MAX_CLUBNAMES3   = 99;   //cfg::MAX_CLUBNAMES
    constexpr UINT MAX_CLUBID_UNION3= 60;   //cfg::MAX_CLUBID_UNION
    constexpr UINT MAX_CLUB_SIZE3   = 25;   //cfg::MAX_CLUB_SIZE


    static wxFileConfig*    spConfigMatch    = 0;   // match global configuration items
    static wxFileConfig*    spConfigSession  = 0;   // session specific configuration items
    static wxString currentIniMatch;
    static wxString currentIniSession;

    static wxString _ConstructFilename(cfg::FileExtension a_fe, UINT a_session = CURRENT_SESSION)
    {
        #if TEST == 1
            return cfg::ConstructFilename(a_fe, a_session)+".tst";
        #else
            return cfg::ConstructFilename(a_fe, a_session);
        #endif
    }   // _ConstructFilename()

    wxFileConfig* GetpCfg(keyId a_id)
    {
        switch (a_id)
        {
            case KEY_SESSION_DISCR:
            case KEY_SESSION_SCHEMA:
            case KEY_SESSION_GAMERESULT:
            case KEY_SESSION_ASSIGNMENTS:
            case KEY_SESSION_ASSIGNMENTS_NAME:
            case KEY_SESSION_CORRECTION:
            case KEY_SESSION_CORRECTION_END:
            case KEY_SESSION_RESULT:
            case KEY_SESSION_RANK_SESSION:
            case KEY_SESSION_RANK_TOTAL:
                return spConfigSession;
            default:
                return spConfigMatch;
        }
    }   // GetpCfg()

    static bool sbIsInit = false;
    static std::map<keyId, wxString> smKeyToString;
    static wxString GetKeyName(keyId a_id)
    {
        if (!sbIsInit)
        {   // DON'T change keywords: database from other language will not be readable!
            sbIsInit = true;
            smKeyToString[KEY_PRG_VERSION]          = "versie";
            smKeyToString[KEY_MATCH_CMNT]           = "kommentaar";
            smKeyToString[KEY_MATCH_SESSION]        = "ronde";
            smKeyToString[KEY_MATCH_PRNT]           = "printer";
            smKeyToString[KEY_MATCH_MAX_ABSENT]     = "max_afwezig";
            smKeyToString[KEY_MATCH_MAXMEAN]        = "max_gemiddelde";
            smKeyToString[KEY_MATCH_CLOCK]          = "klok";
            smKeyToString[KEY_MATCH_WEIGHTAVG]      = "gewogengemiddelde";
            smKeyToString[KEY_MATCH_VIDEO]          = "bios_video";             //??
            smKeyToString[KEY_MATCH_LINESPP]        = "regels_per_blz";
            smKeyToString[KEY_MATCH_NEUBERG]        = "neuberg";
            smKeyToString[KEY_MATCH_GRPRESULT]      = "groep_uitslag";
            smKeyToString[KEY_MATCH_MMCLUB]         = "min_maks_club";
            smKeyToString[KEY_MATCH_FF]             = "form_feed";              //??
            smKeyToString[KEY_MATCH_GLOBALNAMES]    = "centrale_namen";         //??
            smKeyToString[KEY_SESSION_DISCR]        = "beschrijving";
            smKeyToString[KEY_MATCH_DISCR]          = "beschrijving";
            smKeyToString[KEY_MATCH_BUTLER]         = "butler";
            smKeyToString[KEY_SESSION_SCHEMA]       = "schema";
            smKeyToString[KEY_SESSION_GROUPLETTERS] = "groepletters";           //??
            smKeyToString[KEY_SESSION_FIRSTGAME]    = "eerste_spel";            //??
        }

        return smKeyToString[a_id];
    }   // GetKeyName()

    bool DatabaseFlush(io::GlbDbType a_type)
    {
        if (spConfigMatch   && (a_type & io::DB_MATCH  ) ) spConfigMatch  ->Flush();
        if (spConfigSession && (a_type & io::DB_SESSION) ) spConfigSession->Flush();
        return false;
    }   // DatabaseFlush()

    CfgFileEnum DatabaseOpen(io::GlbDbType a_dbType, CfgFileEnum a_how2Open)
    {
        wxString*           oldIni;
        wxFileConfig**      fileCfg;
        cfg::FileExtension  fe;
        if (a_dbType == io::DB_MATCH)
        {
            oldIni    = &currentIniMatch;
            fileCfg   = &spConfigMatch;
            fe        = cfg::EXT_MAIN_INI;
        }
        else
        {
            oldIni    = &currentIniSession;
            fileCfg   = &spConfigSession;
            fe        = cfg::EXT_SESSION_INI;
        }

        wxString newIni = _ConstructFilename(fe);
        if (a_how2Open == CFG_ONLY_READ)
        {   // only handle file if it already exists, so don't create it yet
            if (!wxFile::Exists(newIni))
                return CFG_ERROR;
        }

        if (*oldIni != newIni || *fileCfg == nullptr)
        {   // we have new config file, so close current and create a new one
            *oldIni = newIni;
            delete *fileCfg;
            //remark: stupid default of wxConfig mangles some strings on write, but does not unmangle them on read???
            *fileCfg = new wxFileConfig(wxEmptyString, wxEmptyString, newIni, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
            (*fileCfg)->SetRecordDefaults();   //write all requested entries to the file, if not present
        }

        return CFG_OK;
    }   // DatabaseOpen()

    bool DatabaseClose(io::GlbDbType a_type)
    {
        if (spConfigMatch   && (a_type & io::DB_MATCH  ) ) { delete spConfigMatch;   spConfigMatch   = nullptr;currentIniMatch.clear();}
        if (spConfigSession && (a_type & io::DB_SESSION) ) { delete spConfigSession; spConfigSession = nullptr;currentIniSession.clear();}
        return true;
    }   // DatabaseClose()

    bool DatabaseIsOpen(io::GlbDbType a_dbType)
    {
        return (a_dbType == io::DB_MATCH) ? (spConfigMatch != nullptr) : (spConfigSession != nullptr);
    }   // DatabaseIsOpen()

    wxString ReadValue(keyId a_id, const wxString& a_default, UINT /*session*/)
    {
        const wxFileConfig* pCfg = GetpCfg(a_id); if (pCfg == nullptr) return a_default;
        return pCfg->Read(GetKeyName(a_id), a_default);
    }   // ReadValue()

    bool ReadValueBool(keyId a_id, bool a_default, UINT /*session*/)
    {
        const wxFileConfig* pCfg = GetpCfg(a_id); if (pCfg == nullptr) return a_default;
        return pCfg->ReadBool(GetKeyName(a_id), a_default);
    }   // ReadValueBool()

    long ReadValueLong(keyId a_id, long a_default, UINT /*session*/)
    {
        const wxFileConfig* pCfg = GetpCfg(a_id); if (pCfg == nullptr) return a_default;
        return pCfg->Read(GetKeyName(a_id), a_default);
    }   // ReadValueLong()

    UINT ReadValueUINT(keyId a_id, UINT a_default, UINT /*session*/)
    {
        const wxFileConfig* pCfg = GetpCfg(a_id); if (pCfg == nullptr) return a_default;
        return GetpCfg(a_id)->Read(GetKeyName(a_id), a_default);
    }   // ReadValueUINT()

    bool WriteValue(keyId a_id, const wxString& a_value, UINT /*session*/)
    {
        wxFileConfig* pCfg = GetpCfg(a_id); if (pCfg == nullptr) return false;
        return pCfg->Write(GetKeyName(a_id), a_value);
    }   // WriteValue()

    bool WriteValue(keyId a_id, bool a_value, UINT /*session*/)
    {
        wxFileConfig* pCfg = GetpCfg(a_id); if (pCfg == nullptr) return false;
        return pCfg->Write(GetKeyName(a_id), a_value);
    }   // WriteValue()

    bool WriteValue(keyId a_id, long a_value, UINT /*session*/)
    {
        wxFileConfig* pCfg = GetpCfg(a_id); if (pCfg == nullptr) return false;
        return pCfg->Write(GetKeyName(a_id), a_value);
    }   // WriteValue()

    bool WriteValue(keyId a_id, UINT a_value, UINT /*session*/)
    {
        if (a_id == KEY_MATCH_LINESPP)
            {int x =1; (void)x;}
        wxFileConfig* pCfg = GetpCfg(a_id); if (pCfg == nullptr) return false;
        return pCfg->Write(GetKeyName(a_id), a_value);
    }   // WriteValue()

    bool MaxmeanRead(UINT& a_maxmean)
    {
        const wxFileConfig* pCfg = GetpCfg(KEY_MATCH_MAXMEAN); if (pCfg == nullptr) return false;
        wxString defaultValue = FMT("%u.%02u", a_maxmean / 100, a_maxmean % 100);
        wxString sMaxMean = pCfg->Read(GetKeyName(KEY_MATCH_MAXMEAN), defaultValue);
        a_maxmean = AsciiTolong(sMaxMean, ExpectedDecimalDigits::DIGITS_2);
        return true;
    }   // MaxmeanRead()

    bool MaxmeanWrite(UINT a_maxmean)
    {
        wxFileConfig* pCfg = GetpCfg(KEY_MATCH_MAXMEAN); if (pCfg == nullptr) return false;
        return pCfg->Write(GetKeyName(KEY_MATCH_MAXMEAN), FMT("%u.%02u", a_maxmean / 100, a_maxmean % 100));
    }   // MaxmeanWrite()

    static void ValidateMinMaxClub(UINT& a_min, UINT& a_max)
    {
        if (a_max > MAX_PAIRS3) a_max = MAX_PAIRS3;
        if (a_max == 0) a_max = 1;
        if (a_min == 0) a_min = 1;
        if (a_min > a_max) a_min = a_max;
    }   // ValidateMinMaxClub()

    bool MinMaxClubRead(UINT& a_min, UINT& a_max)
    {
        const wxFileConfig* pCfg = GetpCfg(KEY_MATCH_MMCLUB); if (pCfg == nullptr) return false;
        wxString tmp = pCfg->Read(GetKeyName(KEY_MATCH_MMCLUB), FMT("%u %u", a_min, a_max));
        wxSscanf(tmp, "%u %u", &a_min, &a_max);
        ValidateMinMaxClub(a_min, a_max);
        return true;
    }   // MinMaxClubRead()

    bool MinMaxClubWrite(UINT a_min, UINT a_max)
    {
        wxFileConfig* pCfg = GetpCfg(KEY_MATCH_MMCLUB); if (pCfg == nullptr) return false;
        ValidateMinMaxClub(a_min, a_max);
        return pCfg->Write(GetKeyName(KEY_MATCH_MMCLUB), FMT("%u %u", a_min, a_max));
    }   // MinMaxClubWrite()

    static void Handleschema(const wxString& a_schemaString, cfg::SessionInfo& a_sessionInfo)
    {   // interprete a string like S24S4P14A0"6multi14"P14A0"6t14"
        /*schema string: SxxSxx {PxxAxxOxx"schema"}*n
        *  S=nrOfGames
        *  S=setsize
        *  P=pairs
        *  A=absent pair
        *  O=offset for group : not in use since v1.29
        *  xx=the value that belongs to the previous character(s)
        */

        const wxChar* pData = a_schemaString.c_str();

        if ( *pData == 0 )
        {   // no groupletters and no schema, set defaults: S24 S4 P14 A0 "6multi14"
            a_sessionInfo.groupData.clear();
            cfg::GROUP_DATA grp;         // default groupinfo
            a_sessionInfo.nrOfGames = 24;
            a_sessionInfo.setSize   = 4;
            a_sessionInfo.firstGame = 1;
            a_sessionInfo.groupData.push_back(grp);
            return;
        }

        bool            bError      = false;
        int             nrOfGames   = 0;       // set some  defaults
        int             setSize     = 0;
        std::vector<cfg::GROUP_DATA> groupData;

        do // dummy while loop: easy exit via 'break'!
        {
            int items = wxSscanf(pData, wxT("S%uS%u"), &nrOfGames, &setSize);
            if (items != 2) { bError = true; break; }

            //now we have the nr of games and the setsize: get the values for all groups: {PxxAxx"schema"}*n
            size_t  offset      = a_schemaString.find('P');
            UINT    group       = 0;
            UINT    groupOffset = 0;
            for ( ; offset != wxString::npos; )
            {   //sets of "PxxAxx[Oxx]"schema"
                if (group >= MAX_GROUPS3)   { bError = true; break; }
                int pairs, absent;
                items = wxSscanf(pData+offset, wxT("P%uA%u"), &pairs, &absent);
                if (items != 2) { bError = true; break; }
                cfg::GROUP_DATA tmp;
                tmp.pairs       = pairs;
                tmp.absent      = absent;
                tmp.groupOffset = groupOffset;
                groupOffset    += pairs;    // next group starts at current+pairs
                size_t start, end;
                start = a_schemaString.find('"', offset);   // find begin of schema definition
                if (start == wxString::npos) { bError = true; break; }
                end = a_schemaString.find('"', start+1);    // find end of schema definition
                if (end == wxString::npos) { bError = true; break; }
                tmp.schema  = a_schemaString.substr(start + 1, end - start - 1);
                tmp.schemaId= schema::GetId(tmp.schema);
                groupData.push_back(tmp);
                ++group;
                offset = a_schemaString.find('P', offset + 1);  // find start of next definition
            }
            break;
        } while (0);

        if (bError)
        {
            LogError(_("groupcount: %u, schema: <%s>"), (UINT)groupData.size(), a_schemaString);
            MyMessageBox(a_schemaString, _("error in schema/groupchars"), wxOK | wxICON_INFORMATION);
        }
        else
        {   //takeover data if all ok
            a_sessionInfo.nrOfGames  = nrOfGames;
            a_sessionInfo.setSize    = setSize;
            a_sessionInfo.groupData  = groupData;
        }
    }   // Handleschema()

    bool SchemaRead(cfg::SessionInfo& a_sessionInfo, UINT /*session*/)
    {
        const wxFileConfig* pCfg = GetpCfg(KEY_SESSION_SCHEMA); if (pCfg == nullptr) return false;

        wxString tmp = pCfg->Read(GetKeyName(KEY_SESSION_SCHEMA), "S24S4P14A0\"6multi14\"");// "S24S4P14A0\"6multi14\"P16A3\"6t14\"" ); // <-- change to ""
        Handleschema(tmp, a_sessionInfo);
        a_sessionInfo.firstGame = pCfg->Read(GetKeyName(KEY_SESSION_FIRSTGAME), a_sessionInfo.firstGame );
        // now add groupletters, if present
        tmp = pCfg->Read(GetKeyName(KEY_SESSION_GROUPLETTERS), "");// "AA BB"      );             // <-- change to ES
        wxArrayString groupChars = wxSplit(tmp, ' ');
        for (UINT index = 0; index < a_sessionInfo.groupData.size(); ++index)
        {
            if (index >= groupChars.size()) break;
            a_sessionInfo.groupData[index].groupChars = groupChars[index];
        }

        return true;
    }   // SchemaRead()

    bool SchemaWrite(const cfg::SessionInfo& a_sessionInfo, UINT /*session*/)
    {
        wxFileConfig* pCfg = GetpCfg(KEY_SESSION_SCHEMA); if (pCfg == nullptr) return false;

        UINT maxGroup = a_sessionInfo.groupData.size();
        wxString groupChars;
        wxString schema = FMT("S%uS%u", a_sessionInfo.nrOfGames, a_sessionInfo.setSize);
        for (UINT group = 0; group < maxGroup; ++group)
        {
            groupChars += " " + a_sessionInfo.groupData[group].groupChars;
            schema += FMT("P%uA%u\"%s\""
                , a_sessionInfo.groupData[group].pairs
                , a_sessionInfo.groupData[group].absent
                , a_sessionInfo.groupData[group].schema.c_str()
            );
        }

        (void)pCfg->Write(GetKeyName(KEY_SESSION_GROUPLETTERS), groupChars);
        (void)pCfg->Write(GetKeyName(KEY_SESSION_SCHEMA)      , schema    );
        (void)pCfg->Write(GetKeyName(KEY_SESSION_FIRSTGAME)   , a_sessionInfo.firstGame);
        return pCfg->Flush();
    }   // SchemaWrite()

#ifdef _WIN32       // for VS (32/64 bit) we need 1 byte packing for the storage to be compatible
#pragma pack(1)
#endif
    // OLD3 stuff is now actual definition
    constexpr UINT NAME_LENGTH_OLD3 = 30;   //cfg::MAX_NAME_SIZE;   // pairnames max (30) chars
    //constexpr auto FILE_LENGTH_NAMES_OLD3  = 3993;                 // (MAX_PAIRS3+1)*((NAME_LENGTH_OLD3+1)+2) = (120+1)*((30+1)+2)
    typedef struct NAMES_CURRENT
    {   char    name[NAME_LENGTH_OLD3+1];
        INT16   clubindex;
    } NAMES_CURRENT;                          //  newest format MAX_PAIRS3=120,NAME_LENGTH_OLD3=30
#ifdef _WIN32
#pragma pack()
#endif

    bool PairnamesWrite(const names::PairInfoData& a_pairInfo)
    {
        wxString pairNames = _ConstructFilename(cfg::EXT_NAMES);
        wxFFile fn;
        fn.Open( pairNames, "wb");
        if (!fn.IsOpened())
        {
            MyMessageBox(_("Cannot open file for write: ") + pairNames);
            return false;
        }

        int iErrors = 0;
        for (size_t ii = 0; ii <= MAX_PAIRS3; ++ii)
        {
            union
            {
                NAMES_CURRENT   pairInfo;
                UINT16          pairs;
            } theData{0};

#define MY_NAMESIZE sizeof(theData.pairInfo.name)
#define MY_BUFSIZE  sizeof(theData.pairInfo)

            if ( ii == 0 )
            {   // first entry has only a 16 bit active pair count
                theData.pairs =  a_pairInfo.size()-1;
            }
            else
            {
                if ( ii < a_pairInfo.size())
                {
                    int count = Unicode2Ascii(a_pairInfo[ii].pairName, theData.pairInfo.name, MY_NAMESIZE);
                    MY_UNUSED(count);
                    theData.pairInfo.clubindex = a_pairInfo[ii].clubIndex;
                }
            }

            if (MY_BUFSIZE != fn.Write(&theData.pairInfo, MY_BUFSIZE) )
                ++iErrors;
        }

        if (iErrors)
        {
            MyMessageBox(_("Error(s) at writing to file: ") + pairNames);
        }

        fn.Close();

        return true;
    }   // PairnamesWrite()

    bool PairnamesRead(names::PairInfoData& a_pairInfo)
    {
#ifdef _WIN32       // for VS (32/64 bit) we need 1 byte packing for the storage to be compatible
#pragma pack(1)
#endif

        constexpr UINT AP_OLD1                      = 120;    // nr of pairs
        constexpr UINT AP_OLD2                      = 120;    // nr of pairs
        constexpr UINT NAME_LENGTH_OLD1             = 30;     // pairnames max 30 chars
        constexpr UINT NAME_LENGTH_OLD2             = 30;     // pairnames max 30 chars
        constexpr UINT FILE_LENGTH_NAMES_OLD1       = 3751;   // (AP_OLD1+1)*(NAME_LENGTH_OLD1+1) = (120+1)*(30+1)
        constexpr UINT FILE_LENGTH_NAMES_OLD2       = 6171;   // (2+1)*((NAME_LENGTH_OLD2+1)+(CLUB_LENGTH_OLD2+1)+2+2) = (120+1)*((30+1)+(15+1)+2+2)
        constexpr UINT FILE_LENGTH_NAMES_OLD3       = 3993;   // (MAX_PAIRS3+1)*((NAME_LENGTH_OLD3+1)+2) = (120+1)*((30+1)+2)
        constexpr UINT CLUB_LENGTH_OLD2             = 15;

        typedef struct NAMES_OLD2
        {
            char    name[NAME_LENGTH_OLD2+1];
            char    club[CLUB_LENGTH_OLD2+1];
            INT16   clubindex;
            INT16   klasse;
        } NAMES_OLD2;

        union
        {
            char            old1[1+AP_OLD1][1+NAME_LENGTH_OLD1];
            NAMES_OLD2      old2[1+AP_OLD2];
            NAMES_CURRENT   old3[1+MAX_PAIRS3];
            UINT16          pairIndex[1+MAX_PAIRS3];
            UINT16          pairs;
        } theData;

        UINT o1Size = sizeof(theData.old1);     MY_UNUSED(o1Size);
        UINT o2Size = sizeof(theData.old2);     MY_UNUSED(o2Size);
        UINT o3Size = sizeof(theData.old3);     MY_UNUSED(o3Size);
        UINT oSize  = sizeof(theData);          MY_UNUSED(oSize);

#ifdef _WIN32
#pragma pack()
#endif

        UINT nrOfPairs = 0;

        a_pairInfo.clear();
        a_pairInfo.push_back(names::PairInfo());     // dummy first entry

        wxString pairNames = _ConstructFilename(cfg::EXT_NAMES);
        if (!wxFileExists(pairNames)) return true;   // else error-popup when using logwindow :(
        wxFFile fn;
        fn.Open(pairNames, "rb");
        if (!fn.IsOpened())
            return false;

        auto fileSize = fn.Length();
        if (fileSize > sizeof(theData))
            return false;  // unhandled (old-) filesize

        fn.Read(&theData, fileSize);
        nrOfPairs = theData.pairs;

        bool bHandled = true;
        if (fileSize == FILE_LENGTH_NAMES_OLD1)             // 'old' setup
        {
            nrOfPairs = std::min(nrOfPairs, AP_OLD1);       // nr of active pairs in data

            for (UINT ii = 1; ii <= nrOfPairs;  ++ii)
            {
                a_pairInfo.push_back(names::PairInfo(Ascii2Unicode(theData.old1[ii])));// only name present
            }
        }   // end old1 name-structure
        else
            if (fileSize == FILE_LENGTH_NAMES_OLD2)         // 'old' setup2
            {
                nrOfPairs = std::min(nrOfPairs, AP_OLD2);   // nr of active pairs in data

                for (UINT ii=1; ii <= nrOfPairs; ++ii)
                {
                    a_pairInfo.push_back(names::PairInfo(Ascii2Unicode(theData.old2[ii].name), names::DetermineClubIndex(theData.old2[ii].club))); 
                }
            }
            else
                if (fileSize == FILE_LENGTH_NAMES_OLD3)  // 'newest' setup
                {
                    nrOfPairs = std::min(nrOfPairs, MAX_PAIRS3);

                    for (UINT ii = 1; ii <= nrOfPairs; ++ii)
                    {
                        wxString name = Ascii2Unicode(theData.old3[ii].name);
                        a_pairInfo.push_back(names::PairInfo(name, theData.old3[ii].clubindex)); 
                    }
                }
                else
                {
                    bHandled = false;  // unknown filesize
                }

        return bHandled;
    }   // PairnamesRead()

    bool Session2GlobalIdsRead(UINT_VECTOR& vuPairnrSession2Global, UINT /*session*/)
    {
        // The file has the pairs based on sessionpairnr order, entry zero = dummy
        // pairIndex[x]: sessionPairNr 'x' is mapped to globalPairNr 'pairIndex[x]'
        // example: pairIndex[0,3,20,5,7] -> sessionpair 1 <==> globalpair 3, sp 4 <==> gp 7
        UINT16 pairIndex[1+MAX_PAIRS3];

        vuPairnrSession2Global.clear();
        vuPairnrSession2Global.resize(MAX_PAIRS3+1ULL, 0);
        auto size = sizeof(pairIndex);
        bool bResult = ReadFileBinairy(_ConstructFilename(cfg::EXT_SESSION_ASSIGNMENT_ID), &pairIndex, size);
        for (UINT pairnr = 1; pairnr <= MAX_PAIRS3; ++pairnr)
        {
            vuPairnrSession2Global[pairnr] = pairIndex[pairnr];
        }
        return bResult;
    }   // Session2GlobalIdsRead()

    bool Session2GlobalIdsWrite(const UINT_VECTOR& vuPairnrSession2Global, UINT /*session*/)
    {
        UINT16 pairIndex[1+MAX_PAIRS3]={0};

        for (size_t pair = 0; (pair < vuPairnrSession2Global.size()) && (pair <= MAX_PAIRS3); ++pair)
        {
            pairIndex[pair] = vuPairnrSession2Global[pair];
        }

        return WriteFileBinairy(_ConstructFilename(cfg::EXT_SESSION_ASSIGNMENT_ID), pairIndex, sizeof(pairIndex));
    }   // Session2GlobalIdsWrite()

    bool ClubnamesRead(std::vector<wxString>& a_clubNames, UINT& a_uMaxId)
    {
        wxString clubNames = _ConstructFilename(cfg::EXT_NAMES_CLUB);
        a_clubNames.clear();                    // clear old data
        a_clubNames.resize(MAX_CLUBNAMES3 + 1 );  //allways sized upto reserved count
        a_uMaxId = MAX_CLUBID_UNION3;    // 'free' added clubnames get an id starting here

        if (!wxFileExists(clubNames)) return true;  // prevent popup error-window from logging :(
        wxTextFile tf ;
        tf.Open(clubNames, wxCSConv(wxFONTENCODING_CP437));
        if (!tf.IsOpened())
            return false;             // no clubname file, no error...

        //for getting the clubId, a ',' and as clubname the rest of the line with max size of MAX_CLUB_SIZE3
        wxString format=FMT(" %%u , %%%u[^\n]", MAX_CLUB_SIZE3 );
        // giving somthing like: " %u , %25[^\n]"     took me 3 hours to figure out :(  [xxx] means set of chars!

        for (auto buf = tf.GetFirstLine(); !tf.Eof(); buf = tf.GetNextLine())
        {
            buf.Trim(TRIM_LEFT);buf.Trim(TRIM_RIGHT);    // remove spaces in front and back
            if (buf.IsEmpty() || buf[0]==';')
                continue;                                // ignore empty lines and comment lines

            UINT id;
            wxChar name[MAX_CLUB_SIZE3+1];
            int count = wxSscanf(buf, format.wx_str(), &id, &name);
            if ( (count != 2) || (id < 1) || (id > MAX_CLUBNAMES3) || (name[0] == 0) )
            {
                MyMessageBox(FMT(_("wrong syntax clubname: <%s>"),buf));
                continue;    // ignore this entry
            }

            a_clubNames[id] = name;      //update info
            if (id > a_uMaxId)
                a_uMaxId = id;
        }
        return true;
    }   // ClubnamesRead()

    bool ClubnamesWrite(const std::vector<wxString>& a_clubNames)
    {
        wxString fileName = _ConstructFilename(cfg::EXT_NAMES_CLUB);
        wxRemoveFile(fileName);
        wxTextFile tf;
        tf.Open(fileName, wxCSConv(wxFONTENCODING_CP437));

        wxString formatOfFile = FMT(
            _( "; definition of the clubnames and there id.\n"
               "; syntax: <club nr> , <clubname>\n"
               "; length of clubname maximum %u characters."
            )
            , MAX_CLUB_SIZE3);
        tf.AddLine(formatOfFile);

        for ( UINT ii = 1; ii < a_clubNames.size(); ++ii)
        {
            if ( !a_clubNames[ii].IsEmpty() )
            {
                wxString tmp = FMT("%3u, %s", ii, a_clubNames[ii]);
                tf.AddLine(tmp);
            }
        }

        tf.Create(fileName);
        tf.Write(wxTextFileType_Dos,  wxCSConv(wxFONTENCODING_CP437));
        tf.Close();

        return true;
    }   // ClubnamesWrite()

    bool SessionNamesWrite(const wxArrayString& a_names, UINT /*session*/)
    {
        char names[MAX_PAIRS3+1][5] = {0};
        for (UINT pair = 0; (pair < a_names.size()) && (pair <= MAX_PAIRS3); ++pair)
        {
            // remark: (const char*) is a function cast: it will return a temp char pointer of the wxString
            strcpy(names[pair], (const char*)a_names[pair]);
        }

        return WriteFileBinairy(_ConstructFilename(cfg::EXT_SESSION_ASSIGNMENT_NAME), names, sizeof(names));
    }   // SessionNamesWrite()

    bool SessionNamesRead(wxArrayString& a_assignmentsName, UINT a_session)
    {
        a_assignmentsName.clear();
        a_assignmentsName.resize(MAX_PAIRS3+1, names::GetNotSet()); // if file does not exist: default to 'not assigned'
        wxString file = _ConstructFilename(cfg::EXT_SESSION_ASSIGNMENT_NAME, a_session);
        UINT size = MyGetFilesize(file);
        if (size == UINT_MAX) return true;  // file does not exist

        union assignments
        {
            char old[MAX_PAIRS3+1][4];
            char cur[MAX_PAIRS3+1][5];
        };
        assignments buf;
        bool bNew = (size == sizeof(buf.cur));

        if (! ReadFileBinairy(file, &buf, size))
            return false;
        
        for (UINT pair = 0; pair <= MAX_PAIRS3; ++pair)
        {
            a_assignmentsName[pair] = bNew ? buf.cur[pair] : buf.old[pair];
        }
        return true;
    }   // SessionNamesRead()


#ifdef _WIN32           // for vs 32/64bit we need 1 byte packing to read/write 'old' data
    #pragma pack(1)
#endif

        /*
        Layout of score-file:
        Scores[cfg::MAX_PAIRS/2+1]  -> entry[0] = DESCRIPTION, rest is zero
        N*Scores[x]                 -> N=games with data, entry[0]=SetCount, rest: Scores[nrOfSets] of GameSetData
        */

    struct GameSetDataOrg
    {
        UINT8   pairNS;         // database used 8bit variables to preserve space (640Kb in DOS!)
        UINT8   pairEW;         // so we were limited by this to 255 pairs
        INT16   scoreNS;        // Schemadata used 8bit signed pairs (+ -> NS, - -> EW)
        INT16   scoreEW;        // so pairs were limited to 127
    };

    struct DESCRIPTION
    {
        INT16   dataType;
        UINT16  nrOfGamesWithData;
        UINT16  restSize; //rest of data AFTER row 0 (== Scores[cfg::MAX_PAIRS/2+1]  )
    };

    struct SetCount
    {
        UINT8   dummy[2];
        UINT8   nrOfSets;
    };

    union Scores
    {
        GameSetDataOrg  setData;
        DESCRIPTION     description;
        SetCount        setCount;
    };

#ifdef _WIN32
    #pragma pack()
#endif


    static constexpr int    CURRENT_TYPE = 1;        // version of datastorage
    bool ScoresRead(vvScoreData& a_scoreData, UINT /*a_session*/)
    {
        Scores      gamesetdata[MAX_PAIRS3/2+1];
        wxString    scoreFile   = _ConstructFilename( cfg::EXT_SESSION_SCORE );
        bool        bError      = false;
        wxString    errorMsg    = _(": readerror");

        a_scoreData.clear();                    // remove old data
        a_scoreData.resize(MAX_GAMES3+1);       //  and assure room voor all games. We depend on the entries of this vector!
        FILE* fp; auto err = fopen_s(&fp, scoreFile, "rb"); MY_UNUSED(err);
        if (fp != nullptr)                      /*file exists*/
        {
            if (fread(gamesetdata,sizeof(gamesetdata),1,fp) != 1)      //read header
            {
                bError = true;
                goto error;
            }
            switch (gamesetdata[0].description.dataType)
            {
                case CURRENT_TYPE: // only type for now...
                {
                    std::vector<score::GameSetData> scores;
                    UINT nrOfGames = gamesetdata[0].description.nrOfGamesWithData;
                    assert(nrOfGames <= MAX_GAMES3);
                    for (UINT game = 1; game <= nrOfGames; ++game)
                    {
                        if (fread(&gamesetdata[0],sizeof(gamesetdata[0]),1,fp) != 1)
                        {    bError = true; goto error; }

                        UINT nrOfSets = gamesetdata[0].setCount.nrOfSets;
                        assert(nrOfSets <= MAX_PAIRS3/2);
                        if (fread(&gamesetdata[1], sizeof(gamesetdata[0]), nrOfSets, fp) != nrOfSets)
                        {    bError = true; goto error; }

                        scores.clear();
                        for (UINT sets = 1; sets <= nrOfSets; ++sets)
                        {
                            score::GameSetData newData;    // old to new
                            newData.pairNS  = gamesetdata[sets].setData.pairNS;
                            newData.pairEW  = gamesetdata[sets].setData.pairEW;
                            newData.scoreNS = gamesetdata[sets].setData.scoreNS;
                            newData.scoreEW = gamesetdata[sets].setData.scoreEW;
                            scores.push_back(newData);
                        }
                        a_scoreData[game] = scores;
                    }
                }
                break;
                default: bError = true; errorMsg = _(": unknown type");
            }

        error:
            if (bError)
            {
                // on error: keep what we could read.
                // svGameSetData.clear();
                // svGameSetData.resize(MAX_GAMES3+1); //  and assure room voor all games. We depend on the entries of this vector!
                MyMessageBox(scoreFile+errorMsg, _("Problem reading score-file"));
            }

            if (fp) (void)fclose(fp);
        }

        return !bError;
    }   //ScoresRead()

    bool ScoresWrite(const vvScoreData& a_scoreData, UINT /*a_session*/)
    {
        wxString scoreFile = _ConstructFilename( cfg::EXT_SESSION_SCORE );

        FILE* fp; auto err = fopen_s(&fp, scoreFile, "wb"); MY_UNUSED(err);
        if (fp == nullptr)
        {
            MyMessageBox(scoreFile + _(": open error"), _("Problem opening score-file"));
            return false;
        }

        Scores      gamesetdata[MAX_PAIRS3/2+1] = {0};
        bool        bError      = false;
        UINT        restSize    = 0;
        UINT        nrOfGames   = score::GetNumberOfGames(&a_scoreData);

        for (UINT game = 1; game <= nrOfGames; ++game)
            restSize += 1+a_scoreData[game].size();
        restSize *= sizeof(Scores);
        gamesetdata[0].description.dataType             = CURRENT_TYPE;
        gamesetdata[0].description.nrOfGamesWithData    = nrOfGames;
        gamesetdata[0].description.restSize             = restSize;
        if (fwrite(gamesetdata, sizeof(gamesetdata), 1 , fp) != 1)      //write header
        { bError = true; goto error; }

        gamesetdata[0].description={0}; // clear entry 0 completely!
                                        // now  write all game info
        for (UINT game = 1; game <= nrOfGames; ++game)
        {
            const std::vector<score::GameSetData>& gameData = a_scoreData[game];
            // done in score::WriteScoresToDisk() std::sort(gameData.begin(), gameData.end());   // 'old' programm did this ???
            UINT nrOfSets = gameData.size();
            gamesetdata[0].setCount.nrOfSets = nrOfSets;
            if (fwrite(&gamesetdata[0], sizeof(gamesetdata[0]), 1, fp) != 1)    // write sub-header
            {    bError = true; goto error; }

            for (UINT set = 1; set <= nrOfSets; ++set)
            {
                GameSetDataOrg oldData;    // new to old, ignoring possible contracts
                oldData.pairNS  = gameData[set-1].pairNS;
                oldData.pairEW  = gameData[set-1].pairEW;
                oldData.scoreNS = gameData[set-1].scoreNS;
                oldData.scoreEW = gameData[set-1].scoreEW;
                gamesetdata[set].setData = oldData;
            }
            if (fwrite(&gamesetdata[1], sizeof(gamesetdata[0]), nrOfSets, fp) != nrOfSets)  // write set-data
            {    bError = true; goto error; }
        }

    error:
        (void)fclose(fp);
        if (bError)
        {
            MyMessageBox(scoreFile + _(": write error"), _("Problem writing score-file"));
        }

        return !bError;
    }   //ScoresWrite()

    static bool WriteBinairyUSHORT(const std::vector<unsigned int>& a_vUINT, const wxString& a_file)
    {
        UINT16 buf[MAX_PAIRS3+1] = {0};
        wxASSERT(a_vUINT.size()<=MAX_PAIRS3+1);
        for (UINT ii = 0; ii < a_vUINT.size(); ++ii)
        {
            buf[ii] = a_vUINT[ii];
        }

        return WriteFileBinairy(a_file, &buf, sizeof(buf));
    }   //WriteBinairyUSHORT()

    static bool ReadBinairyUSHORT(std::vector<unsigned int>& a_vUINT, const wxString& a_file)
    {
        a_vUINT.clear();
        a_vUINT.resize(MAX_PAIRS3+1, 0); // default to rank 0

        UINT size = MyGetFilesize(a_file);
        if (size == UINT_MAX) return false;  // file does not exist

        UINT16 buf[MAX_PAIRS3+1];
        if (!ReadFileBinairy(a_file, &buf, size))
            return false;

        for (UINT ii = 0; ii <= MAX_PAIRS3; ++ii)
        {
            a_vUINT[ii] = std::min((UINT)buf[ii],MAX_PAIRS3);
        }

        return true;
    }   //ReadBinairyUSHORT()

    bool SessionRankRead(UINT_VECTOR& a_vuRank, UINT a_session)
    {
        wxString file = _ConstructFilename(cfg::EXT_SESSION_RANK, a_session);
        return ReadBinairyUSHORT(a_vuRank, file);
    }   // SessionRankRead()

    bool SessionRankWrite(const UINT_VECTOR& a_vuRank, UINT a_session)
    {
        wxString file = _ConstructFilename(cfg::EXT_SESSION_RANK, a_session);
        return WriteBinairyUSHORT(a_vuRank, file);
    }   // SessionRankWrite()

    bool TotalRankRead(UINT_VECTOR& a_vuRank, UINT a_session)
    {
        wxString file = _ConstructFilename(cfg::EXT_SESSION_RANK_TOTAL, a_session);
        return ReadBinairyUSHORT(a_vuRank, file);
    }   // TotalRankRead()

    bool TotalRankWrite(const UINT_VECTOR& a_vuRank, UINT a_session)
    {
        wxString file = _ConstructFilename(cfg::EXT_SESSION_RANK_TOTAL, a_session);
        return WriteBinairyUSHORT(a_vuRank, file);
    }   // TotalRankWrite()

    bool CorrectionsEndWrite(const cor::mCorrectionsEnd& a_correctionsEnd, UINT a_session)
    {
        // ;score.2 glbpaar bonus.2 spellen paarnaam
        // 100.00     1     11.00   s16     xxx - xxx

        wxString correctionFile = _ConstructFilename( cfg::EXT_SESSION_CORRECTION_END, a_session ); 
        MyTextFile file(correctionFile, MyTextFile::WRITE);

        if (!file.IsOk())
        {
            wxString msg = FMT(_("Unable to write corrections to: %s"), correctionFile);
            MyLogError(msg);
            MyMessageBox(msg);
            return false;
        }

        file.AddLine(_(";score.2 glbpair bonus.2 games   pairname"));    // content description/spec
        for (const auto& it : a_correctionsEnd)
        {
            wxString correction = FMT("%+6s   %3d     %5s   s%-4u   %-s"
                , LongToAscii2(it.second.score)
                , it.first
                , LongToAscii2(it.second.bonus)
                , it.second.games
                , names::GetGlobalPairInfo(it.first).pairName
            );
            file.AddLine(correction);
        }

        return true;
    }   // CorrectionsEndWrite()


    static bool CorrectionsEndRead(const wxString& a_fileName, cor::mCorrectionsEnd& a_correctionsEnd, bool a_bCorrections, bool a_bForceAdd)
    {
        // read sessionresult (a_bCorrections = false) or corrections for this session (a_bCorrections=true)
        // dif:
        //      - corrections are ONLY applied if pairnr exist
        //      - corrections have extra entry 'bonus' between pairnr and nr of games
        bool        bResult = true;
        MyTextFile  tfile(a_fileName, MyTextFile::READ);

        if (!tfile.IsOk())
            return false;   // file does not exist
        for(wxString str = tfile.GetFirstLine(); !tfile.Eof(); str = tfile.GetNextLine()) // read all lines one by one, until the end of the file
        {   // <pairname> only for info
            // <score.2> <glbpair> <bonus.2> S<games> <pairname>
            // +100.00    1        [-99.99 ] s16   xxx - xxx
            str.Trim(TRIM_LEFT); str.Trim(TRIM_RIGHT);
            if ( str.IsEmpty() || str[0] == ';' ) continue;

            bool    bLineError  = false;
            int     count;
            UINT    pairnr;
            char    charS;
            cor::CORRECTION_END ce;
            #undef  MAX_SIZE
            #define MAX_SIZE 9  /* +100.00 */
            char scoreBuf[MAX_SIZE+1]={0};
            char bonusBuf[MAX_SIZE+1]={0};

            if (a_bCorrections)
            {   //;<score.2> <glbpair> <bonus.2> S<games> <pairname>
                count = wxSscanf(str, "%9s %u %9s %c%u", &scoreBuf, &pairnr, &bonusBuf, &charS, &ce.games);
            }
            else
            {   //sessionResult: <score.2> <glbpair> S<games> <pairname>
                count = 1+wxSscanf(str, "%9s %u %c%u"  , &scoreBuf, &pairnr           , &charS, &ce.games);
            }
            if ( (count != 5) || ((charS != 's') && (charS != 'S')) )
            {
                bLineError = true;
            }

            ce.score = AsciiTolong( scoreBuf, ExpectedDecimalDigits::DIGITS_2);
            ce.bonus = AsciiTolong( bonusBuf, ExpectedDecimalDigits::DIGITS_2);

            if (cor::IsValidCorrectionEnd(pairnr, ce, str, bLineError))
            {   // add info to map
                if ( a_bForceAdd || !a_bCorrections || a_correctionsEnd.find(pairnr) != a_correctionsEnd.end())
                {   // if corrections: only accept existing pairnr
                    if (!a_bForceAdd && a_bCorrections && ce.score == SCORE_IGNORE)
                        ce.score = a_correctionsEnd[pairnr].score;
                    a_correctionsEnd[pairnr] = ce;
                }
            }
            else
            {
                bResult = false;
            }
        }

        return bResult;
    } // CorrectionsEndRead() implementation

    bool CorrectionsEndRead(cor::mCorrectionsEnd& a_correctionsEnd, UINT a_session, bool a_bEdit)
    {   // if edit, we need all data, when calctotal, we only ADD data for existing pairs
        if (a_bEdit) a_correctionsEnd.clear();
        wxString fileName = _ConstructFilename(cfg::EXT_SESSION_CORRECTION_END, a_session);
        return CorrectionsEndRead(fileName, a_correctionsEnd, true, a_bEdit);
    }   // CorrectionsEndRead()

    bool CorrectionsSessionWrite(const cor::mCorrectionsSession& a_correctionsSession, UINT /*a_session*/)
    {
        //;<correctie><type> <sessie paarnr> <extra.1> <max extra> <paarnaam
        //    +3        %      1              0           0      paar 28

        wxString correctionFile = _ConstructFilename( cfg::EXT_SESSION_CORRECTION );
        MyTextFile file(correctionFile, MyTextFile::WRITE);

        if (!file.IsOk())
        {
            wxString msg = FMT(_("Unable to write corrections to: %s"), correctionFile);
            MyLogError(msg);
            MyMessageBox(msg);
            return false;
        }

        // newer versions have also a <games> value, used for butler calculation. We ignore this in an old config!
        file.AddLine(_(";<correction><type> <session pairnr> <extra.1> <max extra> <pairname>"));    // content description/spec
        for (const auto& it : a_correctionsSession)
        {
            wxString correction = FMT("%+5i%c%20u%15s%12i     %s"
                , it.second.correction
                , it.second.type
                , it.first
                , LongToAscii1(it.second.extra)
                , it.second.maxExtra
                , names::PairnrSession2GlobalText(it.first)
            );
            file.AddLine(correction);
        }

        return true;
    }   // CorrectionsSessionWrite()

    bool CorrectionsSessionRead(cor::mCorrectionsSession& a_correctionsSession, UINT a_session)
    {
        a_correctionsSession.clear();

        wxString    fileName    = _ConstructFilename(cfg::EXT_SESSION_CORRECTION, a_session);
        bool        bError      = false;
        MyTextFile  tfile(fileName, MyTextFile::READ);

        if (!tfile.IsOk())
            return false;   // file does not exist
        for(wxString str = tfile.GetFirstLine(); !tfile.Eof(); str = tfile.GetNextLine()) // read all lines one by one, until the end of the file
        {
            //;<correctie><type> <sessiepaarnr> <extra.1> <max extra> <paarnaam>
            //    +3        %      28              0           0      paar 28
            str.Trim(TRIM_LEFT); str.Trim(TRIM_RIGHT);
            if ( str.IsEmpty() || str[0] == ';' ) continue;

            UINT    charCount;
            UINT    sessionPairnr;
            cor::CORRECTION_SESSION cs;

            #undef  MAX_SIZE
            #define MAX_SIZE 10
            char extraBuf[MAX_SIZE+1]={0};

            #define RESOLVE(size) "%i %c %u %" #size "s %n %u"
            #define FORMAT(S)     RESOLVE(S)
            #define formatstring  FORMAT(MAX_SIZE)
            bool bEntryError = (5 !=  wxSscanf(str, formatstring, &cs.correction, &cs.type, &sessionPairnr, &extraBuf, &charCount, &cs.maxExtra));
            cs.extra = AsciiTolong(extraBuf);
            if (IsValidCorrectionSession(sessionPairnr, cs, str, bEntryError))
            {   // add info to map
                cs.games = cs.maxExtra ? 4 : 0;  // we need/use this for butler. Assume a default setsize of 4
                a_correctionsSession[sessionPairnr] = cs;
            }
            else
            {
                bError = true;
            }
        }

        return bError;
    }   //  CorrectionsSessionRead()

    bool SessionResultWrite(const cor::mCorrectionsEnd& a_mSessionResult, UINT /*a_session*/)
    {   // write and read: different params!
        MyTextFile file(_ConstructFilename(cfg::EXT_SESSION_RESULT), MyTextFile::WRITE);
        if (!file.IsOk()) return false;
        file.AddLine(_(";score 'glb pair' s<games>   name"));
        for (const auto& it : a_mSessionResult)              // save score of all pairs
        {
            wxString tmp;
            tmp.Printf("%5s %3u  s%-2u %s",
                LongToAscii2(it.second.score),              // score
                it.first,                                   // global pairnr...
                it.second.games,                            // played games
                names::PairnrGlobal2GlobalText(it.first));  // global pairname
            file.AddLine(tmp);
        }
        return true;
    }   // SessionResultWrite()

    bool SessionResultRead(cor::mCorrectionsEnd& a_mSessionResult, UINT a_session)
    {    // write and read: different params!
        a_mSessionResult.clear();
        wxString fileName = _ConstructFilename(cfg::EXT_SESSION_RESULT, a_session);
        return CorrectionsEndRead(fileName, a_mSessionResult, false, true);
    }   // SessionResultRead()

} // namespace org
