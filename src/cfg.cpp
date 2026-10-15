// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/fileconf.h>
#include <wx/event.h>
#include <wx/stdpaths.h>
#include <wx/app.h>
#include <wx/settings.h>

#include "cfg.h"
#include "utils.h"
#include "names.h"
#include "names.h"
#include "printer.h"
#include "scoreentry.h"
#include "fileio.h"
#include "main.h"

#include <iostream>

namespace cfg
{
    static wxString         s_sMainIni;
    static const wxString   s_sClubNames      ("club.nam");
    static const wxString   s_sCentralNameFile("centraal.nm");      // nm-filename when using global names
    static const wxString   s_sGlobalNameFile ("globalNames");      // db-filename when using global names
    static wxString         s_sBaseFolder;                          // folder where bridge.ini/globalNames.db is located
    static const Fdp        MAX_MEAN_FDP     ("52.50");             // default max average for final calculation if not present

    static wxString         s_sActiveMatch;         // base-name of the active match, used in filenames. Must be inited here!
    static wxString         s_sActiveMatchPath;     // location of match-data
    static wxString         s_sMatchIni;            // = "default.ini";
    static wxString         s_sSessionIni;          // = "default.i0";
    static wxString         s_sDescription;         // description for the active match
    static wxString         s_sComment;             // (c) for bridgewx
    static wxString         s_sPrinterAll;          // printer name: "list" or "WinP PDFCreator"
                                                    //  ^- this is split in printertype and real printername
    static wxString         s_sWinPrinter;          // windows printername
    static wxString         s_sPrinterName;         // the windows name of the printer
    static wxString         s_sMinMaxClub;          //  min/max as a string: "1 120"
    static int              s_iPrinterType;         // the type: file or winprinter
    static constexpr auto   INITIAL_HASH= -1;
    static int              s_iConfigHash= INITIAL_HASH;  // 'hash' from active game and session, so users can check when to save/reload data
    static UINT             s_uSession;             // 0=single match, >=1 match consists of more then one session
    static UINT             s_uMaxAbsent;           // max count of absency to be in the results
    static Fdp              s_fMaxMean;             // max mean result if 1 or more times absent
    static UINT             s_uLinesPerPage;        // lines per page when printing
    static UINT             s_uMaxClub;             // max nr of players used in club-result
    static UINT             s_uMinClub;             // min nr of players used in club-result
    static bool             s_bFormFeed;            // use ff after each printout
    static bool             s_bGlobalNameUse;       // use a global name file i.s.o names per match
    static bool             s_bNeuberg;             // use neuberg calculation if not all decks played or when there are adjusted score results
    static bool             s_bGroupResult;         // show result in matrix of users and decks/sets
    static bool             s_bClock;               // show clock
    static bool             s_bWeightedAvg;         // use weighted avarage in end-result in more matches
    static bool             s_bBiosVideo;           // not used anymore
    static bool             s_bDebug            = false;
    static bool             s_bIsScripttest     = false;
    static bool             s_bButler           = false;
    static bool             s_bNetworkPrinting  = false;    // default: do not enumerate networkrinters: often hangup
    static UINT             s_uFontsizeIncrease = 0;        // increase standard fontsize with 's_uFontsizeIncrease' %
    static wxFileConfig*    s_pConfigMain       = nullptr;  // main configuration items
    static SessionInfo      s_sessionInfo;                  // all the info of a session: gamecount, setSize, firstGame and groupInfo
    static long             s_lActiveDbType     = io::DB_DATABASE;      // default databasetype to use
    static int              s_iLanguage         = wxLANGUAGE_DEFAULT;   // means: not initialized yet

    static constexpr auto MAIN_INIFILE          = "bridge.ini";
    static constexpr auto CFG_MAIN_GAME         = "wedstrijd";
    static constexpr auto CFG_MAIN_VERSION      = "versie";
    static constexpr auto CFG_MAIN_DBTYPE       = "databaseType";
    static constexpr auto CFG_MAIN_LANGUAGE     = "taal";
    static constexpr auto CFG_MAIN_LANGUAGE_DESCRIPTION = "taalomschrijving";

    static constexpr auto CFG_MATCH_MAXA        = "max_afwezig";
    static constexpr auto CFG_MATCH_VIDEO       = "bios_video";
    static constexpr auto CFG_MATCH_VERSION     = "versie";
    static constexpr auto CFG_MATCH_CMNT        = "kommentaar";
    static constexpr auto CFG_MATCH_PRNT        = "printer";
    static constexpr auto CFG_MATCH_MAXMEAN     = "max_gemiddelde";
    static constexpr auto CFG_MATCH_CLOCK       = "klok";
    static constexpr auto CFG_MATCH_WEIGHTAVG   = "gewogengemiddelde";
    static constexpr auto CFG_MATCH_LINESPP     = "regels_per_blz";
    static constexpr auto CFG_MATCH_NEUBERG     = "neuberg";
    static constexpr auto CFG_MATCH_GRPRESULT   = "groep_uitslag";
    static constexpr auto CFG_MATCH_MMCLUB      = "min_maks_club";
    static constexpr auto CFG_MATCH_FF          = "form_feed";
    static constexpr auto CFG_MATCH_GLOBALNAMES = "centrale_namen";
    static constexpr auto CFG_MATCH_SESSION     = "ronde";

    static constexpr auto CFG_SESSION_DISCR     = "beschrijving";
    static constexpr auto CFG_SESSION_GRPCHARS  = "groepletters";
    static constexpr auto CFG_SESSION_VERSION   = "versie";
    static constexpr auto CFG_SESSION_FIRSTGAME = "eerste_spel";
    static constexpr auto CFG_SESSION_SCHEMA    = "schema";

    static constexpr auto FILE_PRINTER_NAME     = "list";
    static constexpr auto WINPRINT_PREFIX       = "WinP ";  // for config: prefix if printer is windows-printer

    #define  DEFAULT_DESCRIPTION                  _("<no description yet>") /*NO static -> translation!*/
    static void HashIncrement();

    static bool     s_bIsBackuped = false;
    static wxString s_sActiveMatchBackup;       // original name of active match
    static wxString s_sActiveMatchPathBackup;   // original folder of active match
    static UINT     s_uSessionBackup;           // original active session

    wxString GetBaseFolder()
    {
        return s_sBaseFolder;
    }   // GetBaseFolder()

    void DataConversionBackup()
    {
        if ( !s_bIsBackuped )
        {
            s_sActiveMatchBackup     = s_sActiveMatch;
            s_sActiveMatchPathBackup = s_sActiveMatchPath;
            s_uSessionBackup         = s_uSession;
        }

        s_bIsBackuped = true;
    }   // DataConversionBackup()

    void DataConversionRestore()
    {
        if ( !s_bIsBackuped )
            return;
        s_sActiveMatch       = s_sActiveMatchBackup;
        s_sActiveMatchPath   = s_sActiveMatchPathBackup;
        s_uSession           = s_uSessionBackup;
        s_bIsBackuped        = false;
        wxArrayString empty;
        HandleCommandline( empty, false ); // open original db again
    }   // DataConversionRestore()

    void DataConversionSetMatch(const wxString& a_match)
    {
        if ( !s_bIsBackuped )
            DataConversionBackup();
        s_sActiveMatch = a_match;
    }   // DataConversionSetMatch()

    void DataConversionSetMatchPath(const wxString& a_matchPath)
    {
        if ( !s_bIsBackuped )
            DataConversionBackup();
        s_sActiveMatchPath = a_matchPath;
    }   // DataConversionSetMatchPath()

    void DataConversionSetSession(UINT a_session)
    {
        if ( !s_bIsBackuped )
            DataConversionBackup();
        s_uSession = a_session;
    }   // DataConversionSetSession()

    void DatabaseTypeSet(long a_type, bool a_bQuiet)
    {
        if ( s_lActiveDbType == a_type )
            return;
        s_lActiveDbType = a_type;
        if ( s_pConfigMain )
            (void)s_pConfigMain->Write(CFG_MAIN_DBTYPE, a_type);
        io::DatabaseTypeSet(static_cast<io::ActiveDbType>(a_type), a_bQuiet);
        if ( !a_bQuiet && GetMainframe() && !cfg::IsScriptTesting() )
        {
            wxCommandEvent event(wxEVT_MENU, ID_MENU_SETUPNEWMATCH);
            GetMainframe()->GetEventHandler()->AddPendingEvent(event);
        }
        HashIncrement();
    }   // DatabaseTypeSet()

    UINT GetNrOfSessionPairs()    // to do????: this is for active session ONLY!
    {
        auto count = s_sessionInfo.groupData.size();
        if ( count )
            return s_sessionInfo.groupData[count-1].groupOffset + s_sessionInfo.groupData[count-1].pairs;
        return 0;
    }   // GetNrOfSessionPairs()

    bool IsSessionPairAbsent(UINT a_sessionPair)
    {
        for ( const auto& it : s_sessionInfo.groupData )
        {
            if ( it.groupOffset > a_sessionPair )
                break;
            if ( it.groupOffset + it.absent == a_sessionPair )
                return true;
        }
        return false;
    }   // IsSessionPairAbsent()

    static void HashIncrement()
    {
        // update config hash
        ++s_iConfigHash;
    }   // HashIncrement()

    void FLushConfigs()
    {
        if ( s_pConfigMain )
            s_pConfigMain->Flush();
        io::DatabaseFlush();
    }   // FLushConfigs()

    void InitializeDefaults(int a_type)
    {
        if ( a_type & INIT_MAIN )
        {
            //bool bResult = wxSetWorkingDirectory("F:\\temp");     // for testing 'old' .ini files
            // baseFolder: location for bridge.ini and fallback for other files if a requested folder is not writable
            wxString localIni   = wxGetCwd() + PS + MAIN_INIFILE;
            s_sBaseFolder        = wxStandardPaths::Get().GetDocumentsDir() + PS + __PRG_NAME__;
            s_sMainIni           = s_sBaseFolder + PS + MAIN_INIFILE;
            s_sActiveMatch       = "default";
            // we need case-insensitive compare while cwd/stdpath can/will give different case for driveletter!
            if ( wxFile::Exists(localIni) && !s_sBaseFolder.IsSameAs(wxGetCwd(), false) )
            {   // 'old' program: bridge.ini was located in the working/match folder.
                // So we copy the file to its new standard location, and rename it on its original location.
                wxCopyFile  (localIni, s_sMainIni, true);
                wxRenameFile(localIni, localIni + ".old", true);
                s_sActiveMatchPath = wxGetCwd();
                s_lActiveDbType    = io::ActiveDbType::DB_ORG;    // old type is .ini
            }
            else
            {
                s_sActiveMatchPath = s_sBaseFolder;
                s_lActiveDbType    = io::ActiveDbType::DB_DATABASE;    // currently we prefer the db-type
            }

            s_iLanguage = wxLocale::GetSystemLanguage();
        }

        if ( a_type & INIT_MATCH )
        {
            s_bBiosVideo     = false;    // compatability
            s_bButler        = false;
            s_bClock         = true;
            s_bFormFeed      = false;    // compatability
            s_bGlobalNameUse = false;
            s_bGroupResult   = false;
            s_bNeuberg       = true;
            s_bWeightedAvg   = false;
            s_uLinesPerPage  = 64;
            s_uMaxAbsent     = 3;
            s_uMaxClub       = MAX_PAIRS;
            s_fMaxMean       = MAX_MEAN_FDP; //52.50% or 1.00 imps/game
            s_uMinClub       = 1;
            s_uSession       = 0;
            s_sPrinterAll    = FILE_PRINTER_NAME;
            s_sWinPrinter    = ES;
            s_sDescription   = DEFAULT_DESCRIPTION;    // is really a session value, but once set, use it as default for all sessions

        }

        if ( a_type & INIT_SESSION )
        {
            s_sessionInfo.groupData.clear();
            s_sessionInfo.firstGame = 1;
            s_sessionInfo.setSize   = 4;
            s_sessionInfo.nrOfGames = 24;
        }
    }   // InitializeDefaults()

    wxString    GetActiveMatch()        { return s_sActiveMatch;             }
    wxString    GetActiveMatchPath()    { return s_sActiveMatchPath+PS;      }
    UINT        GetActiveSession()      { return s_uSession;                 }
    wxString    GetBareMainIni()        { return MAIN_INIFILE;              }
    bool        GetClock()              { return s_bClock;                   }
    int         GetConfigHash()         { return s_iConfigHash;              }
    wxString    GetDescription()        { return s_sDescription;             }
    bool        GetFF()                 { return s_bFormFeed;                }
    wxString    GetFilePrinterName()    { return FILE_PRINTER_NAME;         }
    UINT        GetFirstGame()          { return s_sessionInfo.firstGame;    }
    UINT        GetFontsizeIncrease()   { return s_uFontsizeIncrease;        }
    wxString    GetGlobalNameFile()     { return s_sBaseFolder + PS + s_sGlobalNameFile+GetDbExtension();}
    bool        GetGlobalNameUse()      { return s_bGlobalNameUse;           }
    const vGroupData* GetGroupData()    { return &s_sessionInfo.groupData;   }
    bool        GetGroupResult()        { return s_bGroupResult;             }
    UINT        GetLinesPerPage()       { return s_uLinesPerPage;            }
    UINT        GetMaxAbsent()          { return s_uMaxAbsent;               }
    UINT        GetMaxClubcount()       { return s_uMaxClub;                 }
    Fdp         GetMaxMean()            { return s_fMaxMean;                 }
    UINT        GetMinClubcount()       { return s_uMinClub;                 }
    bool        GetNetworkPrinting()    { return s_bNetworkPrinting;         }
    bool        GetNeuberg()            { return  s_bNeuberg;                }
    UINT        GetNrOfGames()          { return s_sessionInfo.nrOfGames;    }
    const wxString&    GetPrinterName() { return s_sPrinterAll;              }
    const SessionInfo*GetSessionInfo()  { return &s_sessionInfo;             }
    UINT        GetSetSize()            { return s_sessionInfo.setSize;      }
    bool        GetWeightedAvg()        { return s_bWeightedAvg;             }
    wxString    GetWinPrintPrefix()     { return WINPRINT_PREFIX;           }
    bool        IsDebug()               { return s_bDebug;                   }
    bool        IsScriptTesting()       { return s_bIsScripttest;            }
    void        UpdateConfigHash()      { ++s_iConfigHash;                   }
    bool        GetButler()             { return s_bButler;                  }


    int GetLanguage()
    {
        if ( s_iLanguage == wxLANGUAGE_DEFAULT )
        {   // not initialized yet: try to read the bridge.ini file
            InitializeDefaults(INIT_MAIN);
            (void)UpdateConfigMain(CFG_ONLY_READ);
        }
        return s_iLanguage;
    }   // GetLanguage()

    wxString GetCopyright()          // return string to copyright
    {
        // © (c)opyright sign: C2,A9 or E2,92,B8 in utf8 or Alt+0169
        // during scriptesting we want a "fixed" value: less irrelevant diffs
        return FMT("(c)opyright 1991-%s hl/tc", IsScriptTesting() ? __YEAR__AUTO : __YEAR__);
    }   // GetCopyright()

    wxString GetVersion()            // return string to current version
    {
        if ( IsScriptTesting() )
            return FMT("\"%s\" %s %s, %s", __PRG_NAME__, _("version"), __VERSION__AUTO, __DATE__AUTO);
        else
            return FMT("\"%s\" %s %s, %s", __PRG_NAME__, _("version"), __VERSION__    , __DATE__);
    }   // GetVersion()

    wxString GetCopyrightDateTime()
    {
        return FMT("\n%s, %s\n", GetCopyright(), GetDateTime());
    }   // GetCopyrightDateTime)_

    void SetNetworkPrinting(bool a_bSet)    { s_bNetworkPrinting = a_bSet;  }

    static void SchemaWrite()
    {
        // all data is updated, now update the configfile
/*???*/ SetFirstGame(s_sessionInfo.firstGame);
        io::SchemaWrite(s_sessionInfo, s_uSession);
        HashIncrement();
    }   // SchemaWrite()

    void UpdateSessionInfo(const SessionInfo& a_info)
    {
        if ( a_info == s_sessionInfo )
            return;
        SetFirstGame(a_info.firstGame);
        s_sessionInfo = a_info;
        SchemaWrite();
    }   // UpdateSessionInfo()

    void  SetButler(bool a_bOn)
    {
        if ( a_bOn == s_bButler )
            return;
        s_bButler = a_bOn;
        auto maxMean = s_bButler ? std::min(Fdp(1),s_fMaxMean) : std::max(MAX_MEAN_FDP,s_fMaxMean);
        SetMaxMean(maxMean);

        io::WriteValue(KEY_MATCH_BUTLER, s_bButler);
        HashIncrement();
    }   // SetButler()

    void SetActiveMatch(const wxString& a_sMatch, const wxString& a_sMatchPath)
    {
        if (    ( s_sActiveMatch == a_sMatch )
             && ( !a_sMatchPath.IsEmpty() && (a_sMatchPath == s_sActiveMatchPath))
           )
            return;

        s_sActiveMatch     = a_sMatch;
        s_sActiveMatchPath = a_sMatchPath;

        if ( s_pConfigMain )
        {
            wxString tmp = s_sActiveMatchPath + PS + a_sMatch;
            (void)s_pConfigMain->Write(CFG_MAIN_GAME, tmp);
        }

        HashIncrement();
        SendEvent2Mainframe(ID_STATUSBAR_UPDATE);
    }   // SetActiveMatch()

    static bool s_bLanguageRestart = false; // don't cleanup (close databases), if restarting for language-change.
    void SetLanguage(int a_language, const wxString& a_description)
    {
        if ( s_iLanguage == a_language )
            return;
        if ( s_pConfigMain == nullptr )
            UpdateConfigMain(); // re-open main config after language change
        s_iLanguage = a_language;
        (void)s_pConfigMain->Write(CFG_MAIN_LANGUAGE, a_language);
        (void)s_pConfigMain->Write(CFG_MAIN_LANGUAGE_DESCRIPTION, a_description);
        s_pConfigMain->Flush();
        s_bLanguageRestart = true;
        HashIncrement();            // 'something' changed! , need it in CalcSession()
    }   // SetLanguage()

    wxString GetActiveMatchAndSession()
    {   // for statusbar display
        if ( s_uSession == 0 )
            return s_sActiveMatch;   // TRANSLATORS: 'S' is first char of Session
        return s_sActiveMatch + FMT(_(":S%u"), s_uSession);
    }   // GetActiveMatchAndSession()

    void SetActiveSession(UINT a_activeSession)
    {
        if ( s_uSession == a_activeSession )
            return;

        s_uSession = a_activeSession;
        io::WriteValue(KEY_MATCH_SESSION, s_uSession);
        HashIncrement();
    }   // SetActiveSession()

    void SetDescription(const wxString& a_newDescription)
    {
        if ( s_sDescription == a_newDescription )
            return;

        s_sDescription = a_newDescription;
        io::WriteValue(KEY_SESSION_DISCR, s_sDescription, s_uSession);
        if ( DEFAULT_DESCRIPTION == io::ReadValue(KEY_MATCH_DISCR, s_sDescription) )
            io::WriteValue(KEY_MATCH_DISCR, s_sDescription);
        HashIncrement();
    }   // SetDescription()

    void SetMaxAbsent(UINT a_maxAbsent)
    {
        if ( s_uMaxAbsent == a_maxAbsent )
            return;
        s_uMaxAbsent = a_maxAbsent;
        io::WriteValue(KEY_MATCH_MAX_ABSENT, s_uMaxAbsent);
        HashIncrement();
    }   // SetMaxAbsent()

#if 0
    void SetBiosVideo(bool a_bBiosVideo)
    {
        s_bBiosVideo = a_bBiosVideo;
        io::WriteValue(KEY_MATCH_VIDEO, s_bBiosVideo);
    }   // SetBiosVideo()
#endif

    void SetNeuberg(bool a_bNeuberg)
    {
        if ( s_bNeuberg == a_bNeuberg )
            return;
        s_bNeuberg = a_bNeuberg;
        io::WriteValue(KEY_MATCH_NEUBERG, s_bNeuberg);
        HashIncrement();
    }   // SetNeuberg()

    void SetWeightedAvg(bool a_bWA)
    {
        if ( s_bWeightedAvg == a_bWA )
            return;
        s_bWeightedAvg = a_bWA;
        io::WriteValue(KEY_MATCH_WEIGHTAVG, s_bWeightedAvg);
        HashIncrement();
    }   // SetWeightedAvg()

    void ValidateMinMaxClub(UINT& a_minClub, UINT& a_maxClub)
    {
        if ( a_maxClub > MAX_PAIRS )
            a_maxClub = MAX_PAIRS;
        if ( a_maxClub == 0 )
            a_maxClub = 1;
        if ( a_minClub == 0 )
            a_minClub = 1;
        if ( a_minClub > a_maxClub )
            a_minClub = a_maxClub;
    }   // ValidateMinMaxClub()

    void MinMaxClubWrite(UINT a_minClub, UINT a_maxClub)
    {   // integer validater in wxTextCtrl does not work: if input < non-zero minimum, no digits can be entered.
        ValidateMinMaxClub(a_minClub, a_maxClub);
        if ( a_minClub == s_uMinClub && a_maxClub == s_uMaxClub )
            return;

        s_uMinClub = a_minClub;
        s_uMaxClub = a_maxClub;
        io::MinMaxClubWrite(s_uMinClub, s_uMaxClub);
        HashIncrement();
    }   // MinMaxClubWrite()

    void MinMaxClubRead(UINT& a_minClub, UINT& a_maxClub)
    {
        io::MinMaxClubRead(a_minClub, a_maxClub);
        HashIncrement();
    }   // MinMaxClubRead()

    void SetFF(bool a_bFF)
    {
        if ( s_bFormFeed == a_bFF )
            return;
        s_bFormFeed = a_bFF;
        io::WriteValue(KEY_MATCH_FF, s_bFormFeed);
    }   // SetFF()

    void SetClock(bool a_bClock)
    {
        if ( s_bClock == a_bClock )
            return;
        s_bClock = a_bClock;
        io::WriteValue(KEY_MATCH_CLOCK, s_bClock);
        SendEvent2Mainframe(ID_UPDATE_CLOCK);
    }   // SetClock()

    void SetLinesPerPage(UINT a_linesPerPage)
    {
        if ( s_uLinesPerPage == a_linesPerPage )
            return;
        s_uLinesPerPage = a_linesPerPage;
        prn::SetLinesPerPage(s_uLinesPerPage);
        io::WriteValue(KEY_MATCH_LINESPP, s_uLinesPerPage);
    }   // SetLinesPerPage()

    void SetFirstGame(UINT a_firstGame)
    {
        if ( s_sessionInfo.firstGame == a_firstGame )
            return;
        s_sessionInfo.firstGame = a_firstGame;
        io::SchemaWrite(s_sessionInfo, s_uSession );
        HashIncrement();
    }   // SetFirstGame()

    wxString ConstructFilename( FileExtension a_ext, UINT a_sessionId )
    {
// done in next call???        if ( a_sessionId == CURRENT_SESSION )  a_sessionId = s_uSession;
        return ConstructFilename( s_sActiveMatch, a_ext, a_sessionId );
    }   // ConstructFilename()

    wxString GetDbExtension(FileExtension a_ext)
    {
        wxString extension;
        if ( a_ext == cfg::EXT_MAX )
        {
            auto type = io::DatabaseTypeGet();
            a_ext = (type == io::DB_ORG)
                    ? cfg::EXT_MAIN_INI
                    : (type == io::DB_DATABASE)
                      ? cfg::EXT_DATABASE
                      : cfg::EXT_SQLITE;
        }
        switch ( a_ext )
        {
            case EXT_DATABASE:
                extension = ".db";
                break;
            case EXT_SQLITE:
                extension = ".sqlite";
                break;
            case EXT_MAIN_INI:
                extension = ".ini";
                break;
        }
        return extension;
    }   // GetDbExtension()

    wxString ConstructFilename(const wxString& a_basename, FileExtension a_ext, UINT a_sessionId)
    {
        wxString    theWantedName;
        wxString    extension;
        bool        bKnownExtension = true;

        if ( a_sessionId == CURRENT_SESSION )
            a_sessionId = s_uSession;
        switch ( a_ext )
        {
        case EXT_DATABASE:
        case EXT_SQLITE:
        case EXT_MAIN_INI:
            extension = GetDbExtension(a_ext);
            break;
        case EXT_BIN:
            extension = ".bin";
            break;
        case EXT_SESSION_CLUB:
            extension =FMT(".c%u", a_sessionId);
            break;
        case EXT_CLUB_TOTAL:
            extension = ".clt";
            break;
        case EXT_SESSION_CORRECTION:
            extension =FMT(".k%u", a_sessionId);
            break;
        case EXT_SESSION_CORRECTION_END:
            extension = FMT(".e%u", a_sessionId);
            break;
        case EXT_FKW:
            extension = ".fkw";
            break;
        case EXT_SESSION_INI:
            extension = FMT(".i%u", a_sessionId);
            break;
        case EXT_SESSION_ASSIGNMENT_ID:
            extension = FMT(".n%u", a_sessionId);
            break;
        case EXT_SESSION_ASSIGNMENT_NAME_PREV:
            extension = FMT(".x%u", s_uSession-1);
            break;
        case EXT_SESSION_ASSIGNMENT_NAME:
            extension = FMT(".x%u", a_sessionId);
            break;
        case EXT_NAMES: case EXT_NAMES_GLOBAL:
            if ( s_bGlobalNameUse )
            {
                extension = s_sCentralNameFile;
                bKnownExtension = false;    // don't add basename
            }
            else
                extension = ".nm";
            break;
        case EXT_NAMES_CLUB:
            extension = s_sClubNames;
            bKnownExtension = false;    // don't add basename
            break;
        case EXT_SESSION_RANK:
            extension = FMT(".r%u", a_sessionId);
            break;
        case EXT_SESSION_RANK_PREV:
            extension = FMT(".r%u", s_uSession-1);
            break;
        case EXT_SESSION_RANK_TOTAL:
            extension = FMT(".t%u", a_sessionId);
            break;
        case EXT_SESSION_RANK_TOTAL_PREV:
            extension = FMT(".t%u", s_uSession-1);
            break;
        case EXT_SESSION_RESULT:
            extension = FMT(".u%u", a_sessionId);
            break;
        case EXT_RESULT_GROUP:
            extension = ".grp";
            break;
        case EXT_RESULT_SESSION_RANK:
            extension = ".scr";
            break;
        case EXT_RESULT_TOTAL:
            extension = ".tot";
            break;
        case EXT_RESULT_SESSION_NAME:
            extension = ".sc";
            break;
        case EXT_SESSION_SCORE:
            if ( s_uSession >= 1 )
                extension = FMT(".s%u", a_sessionId);
            break;
        default:
            bKnownExtension = false;
            break;
        }

        if ( bKnownExtension )
        {
            theWantedName = GetActiveMatchPath() + a_basename + extension;
        }
        else
        {
            theWantedName = GetActiveMatchPath() + extension;
        }

        return theWantedName;
    }   // ConstructFilename()

    void SetMaxMean( const Fdp& a_maxMean )
    {
        if ( a_maxMean == s_fMaxMean )
            return;
        s_fMaxMean = a_maxMean;
        io::MaxmeanWrite(a_maxMean);
        HashIncrement();
    }   // SetMaxMean()

    void SetGroupResult(bool a_bGroupResult)
    {
        if ( s_bGroupResult == a_bGroupResult )
            return;
        s_bGroupResult = a_bGroupResult;
        io::WriteValue(KEY_MATCH_GRPRESULT, s_bGroupResult);
        HashIncrement();
    }   // SetGroupResult()

    void SetGlobalNameUse(bool a_bGlobalNameUse)
    {
        if ( s_bGlobalNameUse == a_bGlobalNameUse )
            return;
        s_bGlobalNameUse = a_bGlobalNameUse;
        io::WriteValue(KEY_MATCH_GLOBALNAMES, s_bGlobalNameUse);
        HashIncrement();
    }   // SetGlobalNameUse()

    void SetPrinterName( const wxString& a_printer)
    {
        wxString tmp;
        if ( a_printer.IsEmpty() )
            tmp = FILE_PRINTER_NAME;
        else
            tmp = a_printer;

        if ( tmp == s_sPrinterAll )
            return;

        s_sPrinterAll = tmp;
        prn::SetPrinterName(s_sPrinterAll);
        io::WriteValue(KEY_MATCH_PRNT, s_sPrinterAll);    // no 'WinP ' anymore
        SendEvent2Mainframe(ID_STATUSBAR_UPDATE);
    }   // SetPrinterName()


    int UpdateConfigSession(CfgFileEnum a_type)
    {
        auto result = io::DatabaseOpen(io::DB_SESSION, a_type);
        if ( a_type == CFG_ONLY_READ && result == CFG_ERROR )
        {
                return CFG_ERROR;
        }

        io::WriteValue(KEY_PRG_VERSION, GetVersion());
        s_sDescription = io::ReadValue(KEY_SESSION_DISCR , s_sDescription, s_uSession );
        return io::SchemaRead (s_sessionInfo, s_uSession );
    }   // UpdateConfigSession()

    int UpdateConfigMatch(CfgFileEnum a_type)
    {   // assume (new) active match filename is set.
        auto result = io::DatabaseOpen(io::DB_MATCH, a_type);
        if ( a_type == CFG_ONLY_READ && result == CFG_ERROR )
        {
            return CFG_ERROR;
        }

        io::WriteValue(KEY_PRG_VERSION, GetVersion());
        s_sComment       = io::ReadValue     (KEY_MATCH_CMNT      , GetCopyright()     );
        (void)            io::ReadValue      (KEY_MATCH_DISCR     , s_sDescription     );

        s_uSession       = io::ReadValueUINT (KEY_MATCH_SESSION   , s_uSession         );
        s_sPrinterAll    = io::ReadValue     (KEY_MATCH_PRNT      , s_sPrinterAll      );
        s_sPrinterAll.Replace(WINPRINT_PREFIX, ES, false);   // remove winprint prefix
        prn::SetPrinterName(s_sPrinterAll);
        s_uMaxAbsent     = io::ReadValueUINT (KEY_MATCH_MAX_ABSENT, s_uMaxAbsent       );
        (void)io::MaxmeanRead(s_fMaxMean);
        s_bClock         = io::ReadValueBool (KEY_MATCH_CLOCK     , s_bClock           );
        s_bWeightedAvg   = io::ReadValueBool (KEY_MATCH_WEIGHTAVG , s_bWeightedAvg     );
        s_bBiosVideo     = io::ReadValueBool (KEY_MATCH_VIDEO     , s_bBiosVideo       );
        s_uLinesPerPage  = io::ReadValueUINT (KEY_MATCH_LINESPP   , s_uLinesPerPage    );
        prn::SetLinesPerPage(s_uLinesPerPage);
        s_bNeuberg       = io::ReadValueBool (KEY_MATCH_NEUBERG   , s_bNeuberg         );
        s_bGroupResult   = io::ReadValueBool (KEY_MATCH_GRPRESULT , s_bGroupResult     );
        MinMaxClubRead(s_uMinClub, s_uMaxClub);
        s_bFormFeed      = io::ReadValueBool (KEY_MATCH_FF        , s_bFormFeed        );
        s_bGlobalNameUse = io::ReadValueBool (KEY_MATCH_GLOBALNAMES,s_bGlobalNameUse   );
        s_bButler        = io::ReadValueBool (KEY_MATCH_BUTLER    , s_bButler          );

        return CFG_OK;
    }   // UpdateConfigMatch()

    int UpdateConfigMain(CfgFileEnum a_type)
    {
        if ( a_type == CFG_ONLY_READ )
        {   // only handle file if it already exists, so don't create it yet
            if ( !wxFile::Exists(s_sMainIni) )
                return CFG_ERROR;
        }

        if ( s_pConfigMain == nullptr )
        {
            //remark: stupid default of wxConfig mangles some strings on write, but does not unmangle them on read???
            s_pConfigMain = new wxFileConfig(wxEmptyString, wxEmptyString, s_sMainIni, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
            s_pConfigMain->SetRecordDefaults();   //write all requested entries to the file, if not present
        }

        s_pConfigMain->Write(CFG_MAIN_VERSION, GetVersion());
        wxString tmp      = GetActiveMatchPath() + GetActiveMatch();  // base+name....
        wxFileName tmp2   = s_pConfigMain->Read(CFG_MAIN_GAME, tmp);
        s_sActiveMatch    = tmp2.GetFullName();
        if ( !tmp2.GetPath().IsEmpty() )
        {   // new ini: path + match-name
            s_sActiveMatchPath = tmp2.GetPath();
        }
        else
        {   // old ini: update match-name to full path and set type .ini
            s_lActiveDbType = io::ActiveDbType::DB_ORG;
            tmp = s_sActiveMatchPath + PS + s_sActiveMatch;
            (void)s_pConfigMain->Write(CFG_MAIN_GAME, tmp);
        }

        s_iLanguage        = s_pConfigMain->Read(CFG_MAIN_LANGUAGE, s_iLanguage);
        (void)               s_pConfigMain->Read(CFG_MAIN_LANGUAGE_DESCRIPTION, "default");
        s_lActiveDbType    = static_cast<io::ActiveDbType>(s_pConfigMain->Read(CFG_MAIN_DBTYPE, s_lActiveDbType));
        io::DatabaseTypeSet(static_cast<io::ActiveDbType>(s_lActiveDbType), true);  // silently set db-type

        s_pConfigMain->Flush();
        return CFG_OK;
    }   // UpdateConfigMain()

#if 0
    static bool IsInputRedirected()
    {
        // Attempt to read from standard input
        auto chr = getc(stdin);
        if ( chr != EOF ) //_read(_fileno(stdin), &buffer, 1) == 1 )
        {   // If reading was successful, assume input was redirected
            (void)ungetc(chr, stdin);   // push char back
            return true;
        }
        return false;
    }   // IsInputRedirected()
#endif

    int HandleCommandline( const wxArrayString& a_argv, bool a_bInit )
    {
        // first we check if we are autotesting:
        // we need it set/reset on return of this function, even if we have an error
        // to do: next test does not work in GUI programms????
        //       if ( !_isatty(_fileno(stdin)) )
        //           s_bIsScripttest = true;
        // Ok, now we 'need'/'use' a commandline param: '-u', see below

        if ( a_bInit )
        {   // this is on start: need total init. Later changes use active values
            InitializeDefaults();   // initialize everything to defaults
            // now (try to) read existing cfg files
            if ( UpdateConfigMain(CFG_ONLY_READ) == CFG_OK )        // don't create the file yet, only read it if it exists
                if ( UpdateConfigMatch(CFG_ONLY_READ) == CFG_OK )   // only read this if previous file ok
                    UpdateConfigSession(CFG_ONLY_READ);             // only read this if previous file ok
        }

        const wxChar*   pArgptr;
        const wxChar*   pErrorString    { 0 };              // 'init' to prevent compiler warning.....
        bool            bError          { false };
        bool            bNewMatch       { false };
        bool            bNewSession     { false };

        // next l-variables are used to receive the commandline values
        // After the handling of the cmd-line, they will set the real values
        UINT    lsuMaxAbsent    = s_uMaxAbsent;
        bool    lsbButler       = s_bButler;
        Fdp     lsfMaxMean      = s_fMaxMean;
        bool    lsbGroupResult  = s_bGroupResult;
        bool    lsbClock        = s_bClock;
        UINT    lsiLinesPerPage = s_uLinesPerPage;
        bool    lsbNeuberg      = s_bNeuberg;
        UINT    lsiFirstGame    = s_sessionInfo.firstGame;
        UINT    lsiSession      = s_uSession;
        wxString lssActiveMatch = s_sActiveMatch;
        wxString lssActiveMatchPath = s_sActiveMatchPath;

        // now check command-line for parameters
        //lint --e{850}  for loop index variable is changed in body. so what????????
        for ( size_t index = 1; !bError && (index < a_argv.size()); ++index )   // from 1 to size-1  !!!!
        {
            wxChar parameter;
            pErrorString = pArgptr = a_argv[index];
            parameter = (wxChar)tolower(pArgptr[0]);
            if ( (parameter == '-') || (parameter == '/') )
                parameter = (wxChar)tolower(*++pArgptr);
            if ( (*++pArgptr == 0) && (parameter != 'd') && (parameter != 'u') )           /* fe "-r 3"  */
            {
                if ( ++index ==  a_argv.size() )
                {
                    bError = true;
                    break;
                }
                pArgptr = a_argv[index];
                if ( (*pArgptr == '-') || (*pArgptr == '/') )
                {   // assume empty/missing argument
                    bError = true;
                    break;
                }
            }
            switch ( parameter )
            {
            case 'a':
                lsuMaxAbsent = wxAtoi(pArgptr);
                break;
            case 'b':
                lsbButler = wxAtoi(pArgptr);
                break;
            case 'd':
                s_bDebug = 1;
                MyLog::SetAppDebugging();
                break;
            case 'g':
                lsbGroupResult = wxAtoi(pArgptr);
                break;
            case 'm':
                {
                    lsfMaxMean = Fdp(pArgptr);
                }
                break;
            case 'k':
                lsbClock = wxAtoi(pArgptr);
                break;
            case 'l':
                lsiLinesPerPage = wxAtoi(pArgptr);
                break;
            case 'n':
                lsbNeuberg = wxAtoi(pArgptr);
                break;
            case 's':
                lsiFirstGame = wxAtoi(pArgptr);
                break;
            case 'r':
            case 'z':
                lsiSession = wxAtoi(pArgptr);
                if ( lsiSession != s_uSession )
                    bNewSession = true;
                break;
            case 'w':   // name of game
                // deleteAutotestFiles(pArgptr);
                lssActiveMatch = pArgptr;
                if ( lssActiveMatch != s_sActiveMatch )
                    bNewMatch = true;
                break;
            case 'f':   // path of match
                // deleteAutotestFiles(pArgptr);
                lssActiveMatchPath = pArgptr;
                if ( lssActiveMatchPath != s_sActiveMatchPath )
                    bNewMatch = true;
                break;
            case 'q':
            {   // fontsize will be defaultSize * (1 + tmp/100)
                UINT tmp = wxAtoi(pArgptr);
                if ( tmp <= 100 )
                    s_uFontsizeIncrease = tmp;
            }
            break;
            case 'u':
                s_bIsScripttest = true;
                MyLog::SetScriptTesting();
                break;
            default:
                std::cout << FMT(_("Unknown/faulty/missing parameter <%s>\n"), pErrorString);
                bError = true;
            }
        }

        if ( bError )
        {
            std::cout << _("Error in parameter: ") << wxString(pErrorString) << "\n";
            std::cout << FMT("%s%s%s\n",
                "\n" __PRG_NAME__ + _(", version ") + __VERSION__ + _(" of ") + __DATE__ ",",
                GetCopyright(),
                  _("\n"
                    "  activation: BridgeWx [-ax] [-bx] [-gx] [-kx] [-lx] [-nx] [-rx] [-wx] [-fx] [-qx] [-d] [-u]\n"
                    "  ax: maximum allowed Absent count = x\n"
                    "  bx: results are calculated according butler method (x=1), or as percentage (x=0)\n"
                    "  d:  enable Debug for extra info\n"
                    "  gx: display Groupresult yes (x=1), no (x=0)\n"
                    "  kx: Clock on display (x=1), no Clock (x=0)\n"
                    "  lx: nr of Lines per page during print = x (50<x<100)\n"
                    "  mx: Maximum average for non-present sessions = x\n"
                    "  nx: do Neuberg-calculation (x=1) on adjusted scores, no (x=0)\n"
                    "  sx: number of first game, default 1 (meant for guides 2' session)\n"
                    "  zx: Session 'x' (x= 1, 2, ... 15)\n"
                    "  wx: matchname 'x'\n"
                    "  fx: datafolder 'x' for current match\n"
                    "  qx: enlarge fontsize with x%\n"
                    "  u : Unittest/Autotest\n"
                )
            );
            return CFG_ERROR;
        }

        if ( bNewMatch )    // CURRENT (== active) ini file is updated -> it now points to new game/folder
            SetActiveMatch(lssActiveMatch, lssActiveMatchPath);

        //now we can update/create/read the configuration files
        UpdateConfigMain();

        if ( bNewMatch )
        {
            InitializeDefaults(INIT_MATCH);
        }
        UpdateConfigMatch();    // create/update/read match cfgfile

        // now we can update the match data;
        SetMaxMean      (lsfMaxMean);
        SetActiveSession(lsiSession);
        SetMaxAbsent    (lsuMaxAbsent);
        SetNeuberg      (lsbNeuberg);
        SetButler       (lsbButler);
        SetClock        (lsbClock);
        SetLinesPerPage (lsiLinesPerPage);
        SetGroupResult  (lsbGroupResult);

        if ( bNewSession || bNewMatch )
        {
            InitializeDefaults(INIT_SESSION);
        }
        UpdateConfigSession();      // create/update/read session cfgfile

        SetFirstGame(lsiFirstGame);
        io::DatabaseFlush();
        HashIncrement();
        SendEvent2Mainframe(ID_STATUSBAR_UPDATE);

        return CFG_OK;
    }   // HandleCommandline()

    const GROUP_DATA* GetGroupDataFromSessionPair(UINT a_sessionPair)
    {
        for ( const auto& it : s_sessionInfo.groupData )
        {
            if ( it.groupOffset + it.pairs >= a_sessionPair )
                return &it;
        }
        return nullptr; // should not happen
    }   // GetGroupDataFromSessionPair()

    bool IsDark()
    {
        static bool s_bIsInit = false;
        static bool s_bIsDark = false;
        if ( !s_bIsInit )
        {
            s_bIsInit = true;
            s_bIsDark = wxSystemSettings::GetAppearance().IsDark();
        }
        return s_bIsDark;
    }   // IsDark()

    wxColor GetLightOrDark(const wxColor& a_lightColor)
    {
        #define UC(x) (static_cast<unsigned char>(255U - (x)))
        if ( IsDark() ) // get complement color
            return wxColor({  UC(a_lightColor.GetRed())
                            , UC(a_lightColor.GetGreen())
                            , UC(a_lightColor.GetBlue())
                            , (unsigned char)a_lightColor.GetAlpha()
                           });
        return a_lightColor;
        #undef UC
    }   // GetLightOrDark()
}   // end namespace cfg

//   Cleanup dtor
Cleanup::~Cleanup()
{
    if ( !cfg::s_bLanguageRestart )
    {   // real exit, so close databases
        wxDELETE(cfg::s_pConfigMain);    // will set the ptr to zero after deletion
        io::DatabaseClose();
    }
    cfg::s_bLanguageRestart = false;
}   // ~Cleanup()
