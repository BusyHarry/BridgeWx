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
#include "scoreEntry.h"
#include "fileIo.h"
#include "main.h"

#include <iostream>

namespace cfg
{
    #define MAIN_INIFILE "bridge.ini"
    static wxString         ssMainIni;
    static const wxString   ssClubNames      ("club.nam");
    static const wxString   ssCentralNameFile("centraal.nm");       // nm-filename when using global names
    static const wxString   ssGlobalNameFile ("globalNames.db");    // db-filename when using global names
    static wxString         ssBaseFolder;           // folder where bridge.ini/globalNames.db is located


    static wxString         ssActiveMatch;          // base-name of the active match, used in filenames. Must be inited here!
    static wxString         ssActiveMatchPath;      // location of match-data
    static wxString         ssMatchIni;             // = "default.ini";
    static wxString         ssSessionIni;           // = "default.i0";
    static wxString         ssDescription;          // description for the active match
    static wxString         ssComment;              // (c) for bridgewx
    static wxString         ssPrinterAll;           // printer name: "list" or "WinP PDFCreator"
                                                    //  ^- this is split in printertype and real printername
    static wxString         ssWinPrinter;           // windows printername
    static wxString         ssPrinterName;          // the windows name of the printer
    static wxString         ssMinMaxClub;           //  min/max as a string: "1 120"
    static int              siPrinterType;          // the type: file or winprinter
    static constexpr auto   INITIAL_HASH= -1; 
    static int              siConfigHash= INITIAL_HASH;   // 'hash' from active game and session, so users can check when to save/reload data
    static UINT             suSession;              // 0=single match, >=1 match consists of more then one session
    static UINT             suMaxAbsent;            // max count of absency to be in the results
    static UINT             suMaxMean;              // max mean result if 1 or more times absent
    static UINT             suLinesPerPage;         // lines per page when printing
    static UINT             suMaxClub;              // max nr of players used in club-result
    static UINT             suMinClub;              // min nr of players used in club-result
    static bool             sbFormFeed;             // use ff after each printout
    static bool             sbGlobalNameUse;        // use a global name file i.s.o names per match
    static bool             sbNeuberg;              // use neuberg calculation if not all decks played or when there are adjusted score results
    static bool             sbGroupResult;          // show result in matrix of users and decks/sets
    static bool             sbClock;                // show clock
    static bool             sbWeightedAvg;          // use weighted avarage in end-result in more matches
    static bool             sbBiosVideo;            // not used anymore
    static bool             sbDebug          = false;
    static bool             sbIsScripttest   = false;
    static bool             sbButler         = false;
    static bool             sbNetworkPrinting= false;// default: do not enumerate networkrinters: often hangup
    static UINT             suFontsizeIncrease=0;   // increase standard fontsize with 'suFontsizeIncrease' %

    static wxFileConfig*    spConfigMain     = nullptr;             // main configuration items
    static SessionInfo      sSessionInfo;                           // all the info of a session: gamecount, setSize, firstGame and groupInfo
    static long             slActiveDbType   = io::DB_DATABASE;     // default databasetype to use
    static int              siLanguage       = wxLANGUAGE_DEFAULT;  // means: not initialized yet

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
    static void MaxmeanWrite(UINT maxmean);


    static bool     sbIsBackuped = false;
    static wxString ssActiveMatchBackup;        // original name of active match
    static wxString ssActiveMatchPathBackup;    // original folder of active match
    static UINT     suSessionBackup;            // original active session

    wxString GetBaseFolder()
    {
        return ssBaseFolder;
    }   // GetBaseFolder()

    void DataConversionBackup()
    {
        if (!sbIsBackuped)
        {
            ssActiveMatchBackup     = ssActiveMatch;
            ssActiveMatchPathBackup = ssActiveMatchPath;
            suSessionBackup         = suSession;
        }

        sbIsBackuped = true;
    }   // DataConversionBackup()

    void DataConversionRestore()
    {
        if (!sbIsBackuped) return;
        ssActiveMatch       = ssActiveMatchBackup;
        ssActiveMatchPath   = ssActiveMatchPathBackup;
        suSession           = suSessionBackup;
        sbIsBackuped        = false;
        wxArrayString empty;
        HandleCommandline( empty, false ); // open original db again
    }   // DataConversionRestore()
    
    void DataConversionSetMatch(const wxString& a_match)
    {
        if (!sbIsBackuped) DataConversionBackup();
        ssActiveMatch = a_match;
    }   // DataConversionSetMatch()

    void DataConversionSetMatchPath(const wxString& a_matchPath)
    {
        if (!sbIsBackuped) DataConversionBackup();
        ssActiveMatchPath = a_matchPath;
    }   // DataConversionSetMatchPath()

    void DataConversionSetSession(UINT a_session)
    {
        if (!sbIsBackuped) DataConversionBackup();
        suSession = a_session;
    }   // DataConversionSetSession()

    void DatabaseTypeSet(long a_type, bool a_bQuiet)
    {
        if (slActiveDbType == a_type) return;
        slActiveDbType = a_type;
        if (spConfigMain) (void)spConfigMain->Write(CFG_MAIN_DBTYPE, a_type);
        io::DatabaseTypeSet(static_cast<io::ActiveDbType>(a_type), a_bQuiet);
        if (!a_bQuiet && GetMainframe() && !cfg::IsScriptTesting())
        {
            wxCommandEvent event(wxEVT_MENU, ID_MENU_SETUPNEWMATCH);
            GetMainframe()->GetEventHandler()->AddPendingEvent(event);
        }
        HashIncrement();
    }   // DatabaseTypeSet()

    UINT GetNrOfSessionPairs()    // to do????: this is for active session ONLY!
    {
        UINT count = sSessionInfo.groupData.size();
        if (count)
            return sSessionInfo.groupData[count-1].groupOffset + sSessionInfo.groupData[count-1].pairs;
        return 0;
    }   // GetNrOfSessionPairs()

    bool IsSessionPairAbsent(UINT a_sessionPair)
    {
        for (const auto& it : sSessionInfo.groupData)
        {
            if (it.groupOffset > a_sessionPair) break;
            if (it.groupOffset + it.absent == a_sessionPair)
                return true;
        }
        return false;
    }   // IsSessionPairAbsent()

    void HashIncrement()
    {
        // update config hash
        ++siConfigHash;
    }   // HashIncrement()

    void FLushConfigs()
    {
        if (spConfigMain)       spConfigMain   ->Flush();
        io::DatabaseFlush();
    }   // FLushConfigs()

    void InitializeDefaults(int a_type)
    {
        if (a_type & INIT_MAIN)
        {
            //bool bResult = wxSetWorkingDirectory("F:\\temp");     // for testing 'old' .ini files
            // baseFolder: location for bridge.ini and fallback for other files if a requested folder is not writable
            wxString localIni   = wxGetCwd() + PS + MAIN_INIFILE;
            ssBaseFolder        = wxStandardPaths::Get().GetDocumentsDir() + PS + __PRG_NAME__;
            ssMainIni           = ssBaseFolder + PS + MAIN_INIFILE;
            ssActiveMatch       = "default";
            // we need case-insensitive compare while cwd/stdpath can/will give different case for driveletter!
            if (wxFile::Exists(localIni) && !ssBaseFolder.IsSameAs(wxGetCwd(), false))
            {   // 'old' program: bridge.ini was located in the working/match folder.
                // So we copy the file to its new standard location, and rename it on its original location.
                wxCopyFile  (localIni, ssMainIni, true);
                wxRenameFile(localIni, localIni + ".old", true);
                ssActiveMatchPath = wxGetCwd();
                slActiveDbType    = io::ActiveDbType::DB_ORG;    // old type is .ini
            }
            else
            {
                ssActiveMatchPath = ssBaseFolder;
                slActiveDbType    = io::ActiveDbType::DB_DATABASE;    // currently we prefer the db-type
            }

            siLanguage = wxLocale::GetSystemLanguage();
        }

        if (a_type & INIT_MATCH)
        {
            sbBiosVideo     = false;    // compatability
            sbButler        = false;
            sbClock         = true;
            sbFormFeed      = false;    // compatability
            sbGlobalNameUse = false;
            sbGroupResult   = false;
            sbNeuberg       = true;
            sbWeightedAvg   = false;
            suLinesPerPage  = 64;
            suMaxAbsent     = 3;
            suMaxClub       = MAX_PAIRS;
            suMaxMean       = 5250;         //52.50% or 1.00 imps/game
            suMinClub       = 1;
            suSession       = 0;
            ssPrinterAll    = FILE_PRINTER_NAME;
            ssWinPrinter    = ES;
            ssDescription   = DEFAULT_DESCRIPTION;    // is really a session value, but once set, use it as default for all sessions

        }

        if (a_type & INIT_SESSION)
        {
            sSessionInfo.groupData.clear();
            sSessionInfo.firstGame = 1;
            sSessionInfo.setSize   = 4;
            sSessionInfo.nrOfGames = 24;
        }
    }   // InitializeDefaults()

    wxString    GetActiveMatch()        { return ssActiveMatch;             }
    wxString    GetActiveMatchPath()    { return ssActiveMatchPath+PS;      }
    UINT        GetActiveSession()      { return suSession;                 }
    wxString    GetBareMainIni()        { return MAIN_INIFILE;              }
//  bool        GetBiosVideo()          { return sbBiosVideo;               }
    bool        GetClock()              { return sbClock;                   }
    int         GetConfigHash()         { return siConfigHash;              }
    wxString    GetDescription()        { return ssDescription;             }
    bool        GetFF()                 { return sbFormFeed;                }
    wxString    GetFilePrinterName()    { return FILE_PRINTER_NAME;         }
    UINT        GetFirstGame()          { return sSessionInfo.firstGame;    }
    UINT        GetFontsizeIncrease()   { return suFontsizeIncrease;        }
    wxString    GetGlobalNameFile()     { return ssBaseFolder + PS + ssGlobalNameFile;}
    bool        GetGlobalNameUse()      { return sbGlobalNameUse;           }
    const vGroupData* GetGroupData()    { return &sSessionInfo.groupData;   }
    bool        GetGroupResult()        { return sbGroupResult;             }
    UINT        GetLinesPerPage()       { return suLinesPerPage;            }
    UINT        GetMaxAbsent()          { return suMaxAbsent;               }
    UINT        GetMaxClubcount()       { return suMaxClub;                 }
    UINT        GetMaxMean()            { return suMaxMean;                 }
    UINT        GetMinClubcount()       { return suMinClub;                 }
    bool        GetNetworkPrinting()    { return sbNetworkPrinting;         }
    bool        GetNeuberg()            { return  sbNeuberg;                }
    UINT        GetNrOfGames()          { return sSessionInfo.nrOfGames;    }
    const wxString&    GetPrinterName() { return ssPrinterAll;              }
    const SessionInfo*GetSessionInfo()  { return &sSessionInfo;             }
    UINT        GetSetSize()            { return sSessionInfo.setSize;      }
    bool        GetWeightedAvg()        { return sbWeightedAvg;             }
    wxString    GetWinPrintPrefix()     { return WINPRINT_PREFIX;           }
    bool        IsDebug()               { return sbDebug;                   }
    bool        IsScriptTesting()       { return sbIsScripttest;            }
    void        UpdateConfigHash()      { ++siConfigHash;                   }
    bool        GetButler()             { return sbButler;                  }


    int GetLanguage()
    {
        if (siLanguage == wxLANGUAGE_DEFAULT)
        {   // not initialized yet: try to read the bridge.ini file
            InitializeDefaults(INIT_MAIN);
            (void)UpdateConfigMain(CFG_ONLY_READ);
        }
        return siLanguage;
    }   // GetLanguage()

    wxString GetCopyright()          // return string to copyright
    {
        // © (c)opyright sign: C2,A9 or E2,92,B8 in utf8 or Alt+0169
        // during scriptesting we want a "fixed" value: less irrelevant diffs
        return FMT("(c)opyright 1991-%s hl/tc", IsScriptTesting() ? __YEAR__AUTO : __YEAR__);
    }   // GetCopyright()

    wxString GetVersion()            // return string to current version
    {
        if (IsScriptTesting())
            return FMT("\"%s\" %s %s, %s", __PRG_NAME__, _("version"), __VERSION__AUTO, __DATE__AUTO);
        else
            return FMT("\"%s\" %s %s, %s", __PRG_NAME__, _("version"), __VERSION__    , __DATE__);
    }   // GetVersion()

    wxString GetCopyrightDateTime()
    {
        return FMT("\n%s, %s\n", GetCopyright(), GetDateTime());
    }   // GetCopyrightDateTime)_

    void SetNetworkPrinting(bool a_bSet)    { sbNetworkPrinting = a_bSet;  }

    static void SchemaWrite()
    {
        // all data is updated, now update the configfile
/*???*/ SetFirstGame(sSessionInfo.firstGame);
        io::SchemaWrite(sSessionInfo, suSession);
        HashIncrement();
    }   // SchemaWrite()

    void UpdateSessionInfo(const SessionInfo& a_info)
    {
        if (a_info ==  sSessionInfo) return;
        SetFirstGame(a_info.firstGame);
        sSessionInfo = a_info;
        SchemaWrite();
    }   // UpdateSessionInfo()

    void  SetButler(bool a_bOn)
    {
        if ( a_bOn == sbButler) return;
        sbButler = a_bOn;
        suMaxMean = sbButler ? std::min(100U,suMaxMean) : std::max(5250U,suMaxMean);
        MaxmeanWrite(suMaxMean);

        io::WriteValue(KEY_MATCH_BUTLER, sbButler);
        HashIncrement();
    }   // SetButler()

    void SetActiveMatch(const wxString& a_sMatch, const wxString& a_sMatchPath)
    {
        if (    ( ssActiveMatch == a_sMatch )
             && ( !a_sMatchPath.IsEmpty() && (a_sMatchPath == ssActiveMatchPath))
           ) return;

        ssActiveMatch     = a_sMatch;
        ssActiveMatchPath = a_sMatchPath;

        if (spConfigMain)
        {
            wxString tmp = ssActiveMatchPath + PS + a_sMatch;
            (void)spConfigMain->Write(CFG_MAIN_GAME, tmp);
        }

        HashIncrement();
        SendEvent2Mainframe(ID_STATUSBAR_UPDATE);
    }   // SetActiveMatch()

    static bool sbLanguageRestart = false;  // don't cleanup (close databases), if restarting for language-change.
    void SetLanguage(int a_language, const wxString& a_description)
    {
        if (siLanguage == a_language) return;
        if (spConfigMain == nullptr)
            UpdateConfigMain(); // re-open main config after language change
        siLanguage = a_language;
        (void)spConfigMain->Write(CFG_MAIN_LANGUAGE, a_language);
        (void)spConfigMain->Write(CFG_MAIN_LANGUAGE_DESCRIPTION, a_description);
        spConfigMain->Flush();
        sbLanguageRestart = true;
        HashIncrement();            // 'something' changed! , need it in CalcSession()
    }   // SetLanguage()

    wxString GetActiveMatchAndSession()
    {   // for statusbar display
        if ( suSession == 0 )
            return ssActiveMatch;   // TRANSLATORS: 'S' is first char of Session
        return ssActiveMatch + FMT(_(":S%u"), suSession);
    }   // GetActiveMatchAndSession()

    void SetActiveSession(UINT a_activeSession)
    {
        if (suSession == a_activeSession) return;

        suSession = a_activeSession;
        io::WriteValue(KEY_MATCH_SESSION, suSession);
        HashIncrement();
    }   // SetActiveSession()

    void SetDescription(const wxString& a_newDescription)
    {
        if (ssDescription == a_newDescription) return;

        ssDescription = a_newDescription;
        io::WriteValue(KEY_SESSION_DISCR, ssDescription, suSession);
        if (DEFAULT_DESCRIPTION == io::ReadValue(KEY_MATCH_DISCR, ssDescription ))
            io::WriteValue(KEY_MATCH_DISCR, ssDescription);
        HashIncrement();
    }   // SetDescription()

    void SetMaxAbsent(UINT a_maxAbsent)
    {
        if (suMaxAbsent == a_maxAbsent) return;
        suMaxAbsent = a_maxAbsent;
        io::WriteValue(KEY_MATCH_MAX_ABSENT, suMaxAbsent);
        HashIncrement();
    }   // SetMaxAbsent()

#if 0
    void SetBiosVideo(bool a_bBiosVideo)
    {
        sbBiosVideo = a_bBiosVideo;
        io::WriteValue(KEY_MATCH_VIDEO, sbBiosVideo);
    }   // SetBiosVideo()
#endif

    void SetNeuberg(bool a_bNeuberg)
    {
        if (sbNeuberg == a_bNeuberg)  return;
        sbNeuberg = a_bNeuberg;
        io::WriteValue(KEY_MATCH_NEUBERG, sbNeuberg);
        HashIncrement();
    }   // SetNeuberg()

    void SetWeightedAvg(bool a_bWA)
    {
        if (sbWeightedAvg == a_bWA) return;
        sbWeightedAvg = a_bWA;
        io::WriteValue(KEY_MATCH_WEIGHTAVG, sbWeightedAvg);
        HashIncrement();
    }   // SetWeightedAvg()

    void ValidateMinMaxClub(UINT& a_minClub, UINT& a_maxClub)
    {
        if (a_maxClub > MAX_PAIRS) a_maxClub = MAX_PAIRS;
        if (a_maxClub == 0) a_maxClub = 1;
        if (a_minClub == 0) a_minClub = 1;
        if (a_minClub > a_maxClub) a_minClub = a_maxClub;
    }   // ValidateMinMaxClub()

    void MinMaxClubWrite(UINT a_minClub, UINT a_maxClub)
    {   // integer validater in wxTextCtrl does not work: if input < non-zero minimum, no digits can be entered.
        ValidateMinMaxClub(a_minClub, a_maxClub);
        if ( a_minClub == suMinClub && a_maxClub == suMaxClub) return;

        suMinClub = a_minClub;
        suMaxClub = a_maxClub;
        io::MinMaxClubWrite(suMinClub, suMaxClub);
        HashIncrement();
    }   // MinMaxClubWrite()

    void MinMaxClubRead(UINT& a_minClub, UINT& a_maxClub)
    {
        io::MinMaxClubRead(a_minClub, a_maxClub);
        HashIncrement();
    }   // MinMaxClubRead()

    void SetFF(bool a_bFF)
    {
        if (sbFormFeed == a_bFF) return;
        sbFormFeed = a_bFF;
        io::WriteValue(KEY_MATCH_FF, sbFormFeed);
    }   // SetFF()

    void SetClock(bool a_bClock)
    {
        if (sbClock == a_bClock) return;
        sbClock = a_bClock;
        io::WriteValue(KEY_MATCH_CLOCK, sbClock);
        SendEvent2Mainframe(ID_UPDATE_CLOCK);
    }   // SetClock()

    void SetLinesPerPage(UINT a_linesPerPage)
    {
        if (suLinesPerPage == a_linesPerPage) return;
        suLinesPerPage = a_linesPerPage;
        prn::SetLinesPerPage(suLinesPerPage);
        io::WriteValue(KEY_MATCH_LINESPP, suLinesPerPage);
    }   // SetLinesPerPage()

    void SetFirstGame(UINT a_firstGame)
    {
        if (sSessionInfo.firstGame == a_firstGame) return;
        sSessionInfo.firstGame = a_firstGame;
        io::SchemaWrite(sSessionInfo, suSession );
        HashIncrement();
    }   // SetFirstGame()

    wxString ConstructFilename( FileExtension a_ext, UINT a_sessionId )
    {
// done in next call???        if (a_sessionId == CURRENT_SESSION)  a_sessionId = suSession;
        return ConstructFilename( ssActiveMatch, a_ext, a_sessionId );
    }   // ConstructFilename()

    wxString ConstructFilename(const wxString& a_basename, FileExtension a_ext, UINT a_sessionId)
    {
        wxString    theWantedName;
        wxString    extension;
        bool        bKnownExtension = true;

        if (a_sessionId == CURRENT_SESSION)  a_sessionId = suSession;
        switch (a_ext)
        {
        case EXT_DATABASE: 
            extension = ".db";
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
        case EXT_MAIN_INI:
            extension = ".ini";
            break;
        case EXT_SESSION_ASSIGNMENT_ID:
            extension = FMT(".n%u", a_sessionId);
            break;
        case EXT_SESSION_ASSIGNMENT_NAME_PREV:
            extension = FMT(".x%u", suSession-1);
            break;
        case EXT_SESSION_ASSIGNMENT_NAME:
            extension = FMT(".x%u", a_sessionId);
            break;
        case EXT_NAMES: case EXT_NAMES_GLOBAL:
            if (sbGlobalNameUse)
            {
                extension = ssCentralNameFile;
                bKnownExtension = false;    // don't add basename
            }
            else
                extension = ".nm";
            break;
        case EXT_NAMES_CLUB:
            extension = ssClubNames;
            bKnownExtension = false;    // don't add basename
            break;
        case EXT_SESSION_RANK:
            extension = FMT(".r%u", a_sessionId);
            break;
        case EXT_SESSION_RANK_PREV:
            extension = FMT(".r%u", suSession-1);
            break;
        case EXT_SESSION_RANK_TOTAL:
            extension = FMT(".t%u", a_sessionId);
            break;
        case EXT_SESSION_RANK_TOTAL_PREV:
            extension = FMT(".t%u", suSession-1);
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
            if (suSession >= 1)
                extension = FMT(".s%u", a_sessionId);
            break;
        default:
            bKnownExtension = false;
            break;
        }

        if (bKnownExtension)
        {
            theWantedName = GetActiveMatchPath() + a_basename + extension;
        }
        else
        {
            theWantedName = GetActiveMatchPath() + extension;
        }

        return theWantedName;
    }   // ConstructFilename()

    wxString MaxMeanToString()
    {
        return FMT("%u.%02u", suMaxMean / 100, suMaxMean % 100);
    }   // MaxMeanToString()

    UINT MaxMeanFromString( const wxString& a_sMaxMean)
    {
        return AsciiTolong( a_sMaxMean, ExpectedDecimalDigits::DIGITS_2);
    }   // MaxMeanFromString()

    static void MaxmeanRead(UINT& a_maxmean)
    {
        io::MaxmeanRead(a_maxmean);
    }   // MaxmeanRead()

    static void MaxmeanWrite(UINT maxmean)
    {
        io::MaxmeanWrite(maxmean);
        HashIncrement();
    }   // MaxmeanWrite()

    void SetMaxMean( const wxString& a_maxMean )
    {
        auto tmp = MaxMeanFromString(a_maxMean);
        if (tmp == suMaxMean) return;
        suMaxMean = tmp;
        MaxmeanWrite(suMaxMean);
    }   // SetMaxMean()

    void SetGroupResult(bool a_bGroupResult)
    {
        if (sbGroupResult == a_bGroupResult) return;
        sbGroupResult = a_bGroupResult;
        io::WriteValue(KEY_MATCH_GRPRESULT, sbGroupResult);
        HashIncrement();
    }   // SetGroupResult()

    void SetGlobalNameUse(bool a_bGlobalNameUse)
    {
        if (sbGlobalNameUse == a_bGlobalNameUse) return;
        sbGlobalNameUse = a_bGlobalNameUse;
        io::WriteValue(KEY_MATCH_GLOBALNAMES, sbGlobalNameUse);
        HashIncrement();
    }   // SetGlobalNameUse()

    void SetPrinterName( const wxString& a_printer)
    {
        wxString tmp;
        if (a_printer.IsEmpty())
            tmp = FILE_PRINTER_NAME;
        else
            tmp = a_printer;

        if (tmp == ssPrinterAll) return;

        ssPrinterAll = tmp;
        prn::SetPrinterName(ssPrinterAll);
        io::WriteValue(KEY_MATCH_PRNT, ssPrinterAll);    // no 'WinP ' anymore
        SendEvent2Mainframe(ID_STATUSBAR_UPDATE);
    }   // SetPrinterName()


    int UpdateConfigSession(CfgFileEnum a_type)
    {
        CfgFileEnum result = io::DatabaseOpen(io::DB_SESSION, a_type);
        if (a_type == CFG_ONLY_READ && result == CFG_ERROR)
        {
                return CFG_ERROR;
        }

        io::WriteValue(KEY_PRG_VERSION, GetVersion());
        ssDescription = io::ReadValue(KEY_SESSION_DISCR , ssDescription, suSession );
        return io::SchemaRead (sSessionInfo, suSession );
    }   // UpdateConfigSession()

    int UpdateConfigMatch(CfgFileEnum a_type)
    {   // assume (new) active match filename is set.
        CfgFileEnum result = io::DatabaseOpen(io::DB_MATCH, a_type);
        if (a_type == CFG_ONLY_READ && result == CFG_ERROR)
        {
            return CFG_ERROR;
        }

        io::WriteValue(KEY_PRG_VERSION, GetVersion());
        ssComment       = io::ReadValue     (KEY_MATCH_CMNT      , GetCopyright()    );
        (void)            io::ReadValue     (KEY_MATCH_DISCR     , ssDescription     );

        suSession       = io::ReadValueUINT (KEY_MATCH_SESSION   , suSession         );
        ssPrinterAll    = io::ReadValue     (KEY_MATCH_PRNT      , ssPrinterAll      );
        ssPrinterAll.Replace(WINPRINT_PREFIX, ES, false);   // remove winprint prefix
        prn::SetPrinterName(ssPrinterAll);
        suMaxAbsent     = io::ReadValueUINT (KEY_MATCH_MAX_ABSENT, suMaxAbsent       );
        MaxmeanRead(suMaxMean);
        sbClock         = io::ReadValueBool (KEY_MATCH_CLOCK     , sbClock           );
        sbWeightedAvg   = io::ReadValueBool (KEY_MATCH_WEIGHTAVG , sbWeightedAvg     );
        sbBiosVideo     = io::ReadValueBool (KEY_MATCH_VIDEO     , sbBiosVideo       );
        suLinesPerPage  = io::ReadValueUINT (KEY_MATCH_LINESPP   , suLinesPerPage    );
        prn::SetLinesPerPage(suLinesPerPage);
        sbNeuberg       = io::ReadValueBool (KEY_MATCH_NEUBERG   , sbNeuberg         );
        sbGroupResult   = io::ReadValueBool (KEY_MATCH_GRPRESULT , sbGroupResult     );
        MinMaxClubRead(suMinClub, suMaxClub);
        sbFormFeed      = io::ReadValueBool (KEY_MATCH_FF        , sbFormFeed        );
        sbGlobalNameUse = io::ReadValueBool (KEY_MATCH_GLOBALNAMES,sbGlobalNameUse   );
        sbButler        = io::ReadValueBool (KEY_MATCH_BUTLER    , sbButler          );

        return CFG_OK;
    }   // UpdateConfigMatch()

    int UpdateConfigMain(CfgFileEnum a_type)
    {
        if (a_type == CFG_ONLY_READ)
        {   // only handle file if it already exists, so don't create it yet
            if (!wxFile::Exists(ssMainIni))
                return CFG_ERROR;
        }

        if (spConfigMain == nullptr)
        {
            //remark: stupid default of wxConfig mangles some strings on write, but does not unmangle them on read???
            spConfigMain = new wxFileConfig(wxEmptyString, wxEmptyString, ssMainIni, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
            spConfigMain->SetRecordDefaults();   //write all requested entries to the file, if not present
        }

        spConfigMain->Write(CFG_MAIN_VERSION, GetVersion());
        wxString tmp      = GetActiveMatchPath() + GetActiveMatch();  // base+name....
        wxFileName tmp2   = spConfigMain->Read(CFG_MAIN_GAME, tmp);
        ssActiveMatch     = tmp2.GetFullName();
        if ( !tmp2.GetPath().IsEmpty())
        {   // new ini: path + match-name
            ssActiveMatchPath = tmp2.GetPath();
        }
        else
        {   // old ini: update match-name to full path and set type .ini
            slActiveDbType = io::ActiveDbType::DB_ORG;
            tmp = ssActiveMatchPath + PS + ssActiveMatch;
            (void)spConfigMain->Write(CFG_MAIN_GAME, tmp);
        }

        siLanguage        = spConfigMain->Read(CFG_MAIN_LANGUAGE, siLanguage);
        (void)              spConfigMain->Read(CFG_MAIN_LANGUAGE_DESCRIPTION, "default");
        slActiveDbType    = static_cast<io::ActiveDbType>(spConfigMain->Read(CFG_MAIN_DBTYPE, slActiveDbType));
        io::DatabaseTypeSet(static_cast<io::ActiveDbType>(slActiveDbType), true);  // silently set db-type

        spConfigMain->Flush();
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
        //           sbIsScripttest = true;
        // Ok, now we 'need'/'use' a commandline param: '-u', see below

        if (a_bInit)
        {   // this is on start: need total init. Later changes use active values
            InitializeDefaults();   // initialize everything to defaults
            // now (try to) read existing cfg files
            if (UpdateConfigMain(CFG_ONLY_READ) == CFG_OK)      // don't create the file yet, only read it if it exists
                if (UpdateConfigMatch(CFG_ONLY_READ) == CFG_OK) // only read this if previous file ok
                    UpdateConfigSession(CFG_ONLY_READ);         // only read this if previous file ok
        }

        const wxChar*   pArgptr;
        const wxChar*   pErrorString    { 0 };              // 'init' to prevent compiler warning.....
        bool            bError          { false };
        bool            bNewMatch       { false };
        bool            bNewSession     { false };

        // next l-variables are used to receive the commandline values
        // After the handling of the cmd-line, they will set the real values
        UINT    lsuMaxAbsent    = suMaxAbsent;
//      bool    lsbBiosVideo    = sbBiosVideo;
        bool    lsbButler       = sbButler;
        UINT    lsiMaxMean      = suMaxMean;
        bool    lsbGroupResult  = sbGroupResult;
        bool    lsbClock        = sbClock;
        UINT    lsiLinesPerPage = suLinesPerPage;
        bool    lsbNeuberg      = sbNeuberg;
        UINT    lsiFirstGame    = sSessionInfo.firstGame;
        UINT    lsiSession      = suSession;
        wxString lssActiveMatch = ssActiveMatch;
        wxString lssActiveMatchPath = ssActiveMatchPath;

        // now check command-line for parameters
        //lint --e{850}  for loop index variable is changed in body. so what????????
        for (size_t index = 1; !bError && (index < a_argv.size()); ++index)       // from 1 to size-1  !!!!
        {
            wxChar parameter;
            pErrorString = pArgptr = a_argv[index];
            parameter = (wxChar)tolower(pArgptr[0]);
            if ((parameter == '-') || (parameter == '/'))
                parameter = (wxChar)tolower(*++pArgptr);
            if ((*++pArgptr == 0) && (parameter != 'd') && (parameter != 'u'))           /* fe "-r 3"  */
            {
                if (++index ==  a_argv.size())
                {
                    bError = true;
                    break;
                }
                pArgptr = a_argv[index];
            }
            switch (parameter)
            {
            case 'a':
                lsuMaxAbsent = wxAtoi(pArgptr);
                break;
            case 'b':
//              lsbBiosVideo = wxAtoi(pArgptr);
                lsbButler = wxAtoi(pArgptr);
                break;
            case 'd':
                sbDebug = 1;
                MyLog::SetAppDebugging();
                break;
            case 'g':
                lsbGroupResult = wxAtoi(pArgptr);
                break;
            case 'm':
                {
                    wxString tmp = pArgptr;
                    lsiMaxMean = MaxMeanFromString(tmp);
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
                if (lsiSession != suSession)
                    bNewSession = true;
                break;
            case 'w':   // name of game
                // deleteAutotestFiles(pArgptr);
                lssActiveMatch = pArgptr;
                if (lssActiveMatch != ssActiveMatch)
                    bNewMatch = true;
                break;
            case 'f':   // path of match
                // deleteAutotestFiles(pArgptr);
                lssActiveMatchPath = pArgptr;
                if (lssActiveMatchPath != ssActiveMatchPath)
                    bNewMatch = true;
                break;
            case 'q':
            {   // fontsize will be defaultSize * (1 + tmp/100)
                UINT tmp = wxAtoi(pArgptr);
                if (tmp <= 100)
                    suFontsizeIncrease = tmp;
            }
            break;
            case 'u':
                sbIsScripttest = true;
                MyLog::SetScriptTesting();
                break;
            default:
                std::cout << FMT(_("Unknown/faulty/missing parameter <%s>\n"), pErrorString);
                bError = true;
            }
        }

        if (bError)
        {
            std::cout << _("Error in parameter: ") << wxString(pErrorString) << "\n";
            std::cout << FMT("%s%s%s\n",
                "\n" __PRG_NAME__ + _(", version ") + __VERSION__ + _(" of ") + __DATE__ ",",
                GetCopyright(),
                  _("\n"
                    "  activation: BridgeWx [-ax] [-bx] [-gx] [-kx] [-lx] [-nx] [-rx] [-wx] [-fx] [-qx] [-d] [-u]\n"
                    "  ax: maximum allowed Absent count = x\n"
  //                "  bx: video trough bios (x=1), or direct access (x=0)\n"
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

        if (bNewMatch)      // CURRENT (== active) ini file is updated -> it now points to new game/folder
            SetActiveMatch(lssActiveMatch, lssActiveMatchPath);

        //now we can update/create/read the configuration files
        UpdateConfigMain();

        if (bNewMatch)
        {
            InitializeDefaults(INIT_MATCH);
        }
        UpdateConfigMatch();    // create/update/read match cfgfile

        // now we can update the match data;
        SetActiveSession(lsiSession);
        SetMaxAbsent    (lsuMaxAbsent);
        SetNeuberg      (lsbNeuberg);
//      SetBiosVideo    (lsbBiosVideo);
        SetButler       (lsbButler);
        SetClock        (lsbClock);
        SetLinesPerPage (lsiLinesPerPage);
        SetGroupResult  (lsbGroupResult);

        if (bNewSession || bNewMatch)
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
        for (const auto& it : sSessionInfo.groupData)
        {
            if ( it.groupOffset + it.pairs >= a_sessionPair ) return &it;
        }
        return nullptr; // should not happen
    }   // GetGroupDataFromSessionPair()

    bool IsDark()
    {
        static bool bIsInit = false;
        static bool bIsDark = false;
        if ( !bIsInit )
        {
            bIsInit = true;
            bIsDark = wxSystemSettings::GetAppearance().IsDark();
        }
        return bIsDark;
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
    if (!cfg::sbLanguageRestart)
    {   // real exit, so close databases
        wxDELETE(cfg::spConfigMain);    // will set the ptr to zero after deletion
        io::DatabaseClose();
    }
    cfg::sbLanguageRestart = false;
}   // ~Cleanup()
