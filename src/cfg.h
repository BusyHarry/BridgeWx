// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _CFG_H_
#define _CFG_H_
#pragma once
#pragma warning(disable:26812)

#include <vector>

#include <wx/string.h>

#include "utils.h"
#include "schemaInfo.h"

class wxWindow;
class wxTextCtrl;

/*
    communicate with mainframe on the base of a menu-event
    @param id the value for the command handler
    @param pClientData optional pointer to extra data for this command
*/
void SendEvent2Mainframe(int id, void* const pClientData = nullptr);
void SendEvent2Mainframe(wxWindow* pWindows, int id, void* const pClientData = nullptr);
wxWindow* GetMainframe();   // only needed here, comes from ctor of the mainframe.

enum CfgFileEnum{ CFG_OK, CFG_ERROR, CFG_ONLY_READ, CFG_WRITE };  // used for handling config files

constexpr auto TRIM_LEFT  = false;
constexpr auto TRIM_RIGHT = true;

typedef unsigned char       UINT8;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef unsigned int        UINT;
typedef std::vector<UINT> UINT_VECTOR;

#define wxEVT_USER          wxEVT_MENU
#define PS                  wxFileName::GetPathSeparator()

/*
*   Cleanup class, just used for cleaningup things when the programm ends
*   Used for wx-stuff that needs to be deleted/cleanedup before the APP ends.
*/
class Cleanup
{   // https://github.com/wxWidgets/wxWidgets/issues/18571?cversion=0&cnum_hist=2
public:
    Cleanup() {}
    ~Cleanup();
};

enum MY_IDS
{
    ID_SYSTEM_INFO = wxID_HIGHEST + 1,
    ID_EXIT,
    ID_ABOUT,
    ID_MENU_LOG,

    ID_MENU_SETUP_FIRST,	// first id used to create new pages
    ID_MENU_SETUPGAME,
    ID_MENU_NAMEEDITOR,
    ID_MENU_SETUPSCHEMA,
    ID_MENU_SETUPPRINTER,
    ID_MENU_SETUPNEWMATCH,
    ID_MENU_ASSIGNNAMES,
    ID_MENU_SCORE_ENTRY,
    ID_MENU_COR_ENTRY_SESSION,
    ID_MENU_COR_ENTRY_END,
    ID_MENU_DEBUG,
    ID_MENU_DEBUG_GUIDES,
    ID_MENU_DEBUG_SCORE_SLIPS,
    ID_MENU_CALC_SCORES,
    ID_MENU_SETUP_LAST,		// last id used to create new pages

    ID_MENU_PRINTPAGE,
    ID_MENU_PRINTFILE,
    ID_MENU_OLD_TO_DBASE,
    ID_MENU_DBASE_TO_OLD,
    ID_MENU_OLD_DBASE,      // 'original' way of saving game-data: game.* files
    ID_MENU_NEW_DBASE,      // 'new'wayof saving  data: game.db, only one file
    ID_MENU_LANGUAGE,
    ID_SCHEMA_NEXTGROUP,
    ID_NAMEEDIT_SEARCH,
    ID_NAMEEDIT_ADD,
    ID_STATUSBAR_UPDATE,
    ID_STATUSBAR_SETTEXT,
    ID_UPDATE_CLOCK,
    ID_NAMEEDIT_SEARCH_ENTER,
    ID_SETUPSCHEMA_CHANGE_GAMES,
    ID_SETUPSCHEMA_CHANGE_SETSIZE,
    ID_SETUPSCHEMA_CHANGE_GROUPS,
    ID_SETUPSCHEMA_SELECT_GROUP,
    ID_SETUPSCHEMA_CHANGE_GROUPLETTERS,
    ID_SETUPSCHEMA_CHANGE_PAIRS,
    ID_SETUPSCHEMA_CHANGE_ABSENT,
    ID_SETUPSCHEMA_SELECT_SCHEMA,
    ID_ASSIGNNAME_UPDATEGRID,
    ID_BASEFRAME_SEARCH,
    ID_LOG_WINDOW
};

namespace cfg
{
#include "version.h"
    enum FileExtension
    {
        EXT_DATABASE,                      //  "db"
        EXT_BIN,                           //  "bin"
        EXT_CLUB_TOTAL,                    //  "clt"
        EXT_FKW,                           //  "fkw"
        EXT_MAIN_INI,                      //  "ini"
        EXT_NAMES,                         //  "nm"
        EXT_RESULT_GROUP,                  //  "grp"
        EXT_NAMES_GLOBAL,                  //  "centraal.nm"
        EXT_NAMES_CLUB,                    //  "club.nam"
        EXT_RESULT_SESSION_RANK,           //  "scr"       session result on rank order
        EXT_RESULT_TOTAL,                  //  "tot"       total result as ascii file
        EXT_RESULT_SESSION_NAME,           //  "sc"        session result on pair order
        EXT_SESSION_CLUB,                  //  'c.'
        EXT_SESSION_CORRECTION,            //  'k.'
        EXT_SESSION_CORRECTION_END,        //  'e.'
        EXT_SESSION_INI,                   //  'i.'        ini file for specific session
        EXT_SESSION_ASSIGNMENT_ID,         //  'n.'        x[pair] = index in pairNames[]
        EXT_SESSION_ASSIGNMENT_NAME,       //  'x.'        x[pair] = full sessionpairname (groupchars+pair like AA12)
        EXT_SESSION_ASSIGNMENT_NAME_PREV,  //  'x.'        x[pair] = previous session: full sessionpairname (groupchars+pair like AA12)
        EXT_SESSION_RANK,                  //  'r.'        x[pair] = rank in session       
        EXT_SESSION_RANK_PREV,             //  'r.'        x[pair] = rank in previous session       
        EXT_SESSION_RANK_TOTAL,            //  't.'        x[pair] = total rank of 'pair' upto specific round (binairy)
        EXT_SESSION_RANK_TOTAL_PREV,       //  't.'        x[pair] = total rank of 'pair' upto previous round (binairy)
        EXT_SESSION_RESULT,                //  'u.'        x[pair] = result of 'pair' like "50.00   1  s4  paar 1", zero based
        EXT_SESSION_SCORE,                 //  's.'        binairy scores for specific session
        EXT_MAX
    };

    constexpr UINT MAX_PAIRS            = 120;
    constexpr UINT MAX_GROUPS           = 9;
    constexpr UINT MAX_SETS             = 15;
    constexpr UINT MAX_PAIRS_PER_GROUP  = 32;
    constexpr UINT MAX_GAMES            = 32;
    constexpr UINT MAX_SESSIONS         = 15;
    constexpr UINT MAX_CLUBID_UNION     = 60;
    constexpr UINT MAX_CLUBNAMES        = 99;
    constexpr UINT MAX_CLUB_SIZE        = 25;
    constexpr UINT MAX_NAME_SIZE        = 30;
    constexpr UINT MAX_DESCRIPTION      = 32;

    class SCHEMA_DATA;
    typedef struct GROUP_DATA
    {
        GROUP_DATA(){pairs=14;absent=0;groupOffset=0;schemaId=schema::defaultId;schema=schema::defaultSchema;}
        bool operator == (const GROUP_DATA& rhs) const
        {
            return     pairs        == rhs.pairs
                    && absent       == rhs.absent
                    && groupOffset  == rhs.groupOffset
                    && schemaId     == rhs.schemaId
                    && schema       == rhs.schema
                    && groupChars   == rhs.groupChars
                ;
        }
        UINT        pairs;              // nr of pairs fot this group
        UINT        absent;             // nr of absent pair
        UINT        groupOffset;        // first pairnr in (next) group
        int         schemaId;           // id of actual schema
        wxString    schema;             // the schema to use
        wxString    groupChars;         // 1/2 char for this group, if more then 1 group
    } GROUP_DATA;

    typedef  std::vector<GROUP_DATA> vGroupData;

    typedef struct SessionInfo
    {   // to have the updated values at the same time!
        UINT        nrOfGames   = 0;
        UINT        setSize     = 0;
        UINT        firstGame   = 0;
        vGroupData  groupData;
        bool operator == (const SessionInfo& rhs) const
        {
            return     nrOfGames == rhs.nrOfGames
                    && setSize   == rhs.setSize
                    && firstGame == rhs.firstGame
                    && groupData == rhs.groupData;
        }
    } SessionInfo;

    #define CURRENT_SESSION UINT_MAX
    wxString    ConstructFilename(const wxString& basename, FileExtension ext, UINT a_sessionId = CURRENT_SESSION);   // based on active path/session and supplied extensiontype
    wxString    ConstructFilename( FileExtension ext, UINT a_sessionId = CURRENT_SESSION );         // based on active path/match and wanted session and supplied extensiontype

    int         GetLanguage();                              // language to use for interface
    wxString    GetActiveMatch();                           // name of current match
    wxString    GetActiveMatchPath();                       // full pathname inclusive '/'

    UINT        GetActiveSession();                         // the current session of this match
    wxString    GetActiveMatchAndSession();                 // name and session (if non-zero)
    bool        GetClock();                                 // clock wanted?
    wxString    GetCopyright();                             // my (c) string
    wxString    GetVersion();                               // programm version
    wxString    GetDescription();                           // the description of the active match
    int         GetConfigHash();                            // identification of active confuguration

    bool        GetFF();                                    // FF wanted after each printout?
    UINT        GetMaxAbsent();                             // max times allowed to be not present
    UINT        GetMaxClubcount();                          // maximum nr of players for the club result
    UINT        GetMaxMean();                               // max mean when absent
    UINT        GetFontsizeIncrease() ;                     // get wanted increse of fontsize in %
    UINT        GetMinClubcount();                          // minimum nr of players for the club result
    bool        GetNeuberg();                               // state of the Neuberg calculation
    bool        GetWeightedAvg();                           // the type of calculation over more matches

    int         HandleCommandline(const wxArrayString& argv, bool bInit = true);  //...
    bool        IsDebug();                                  // true if we want some extra output
    bool        IsScriptTesting();                          // true if running auto-tests
    wxString    MaxMeanToString();                          // get this value as string
    void        SetActiveMatch(const wxString& sMatch, const wxString& a_sMatchPath = wxEmptyString);     // Set (new) name for current match
    void        SetActiveSession(UINT activeSession);       // set the new session for a set of games
    void        SetClock(bool bClock);                      // (re)set the clockdisplay
    void        SetDescription(const wxString& newComment); // set the new game description
    void        SetFF(bool bFF);                            // (re)set the wanted flag
    void        SetLanguage(int language, const wxString& description); // set the language for the user-interface

    void        SetMaxMean( const wxString& maxMean);       // set max mean in multiple games when not always present
    void        SetNeuberg(bool bNeuberg);                  // (re)set the use Neuberg calculation
    void        SetWeightedAvg(bool bWA);                   // (re)set the use of weighted mean

    void        UpdateConfigHash();                         // next call to getconfighash() will trigger updates
    int         UpdateConfigMatch(CfgFileEnum a_type = CFG_WRITE);  // (possible) update of the active match config
    int         UpdateConfigMain (CfgFileEnum a_type = CFG_WRITE);  // (possible) update of the main config
    int         UpdateConfigSession(CfgFileEnum a_type = CFG_WRITE);// (possible) update of current session config
    void        FLushConfigs();                             // write changes to disk
    void        ValidateMinMaxClub(UINT& minClub, UINT& maxClub); // correct values on error
    void        MinMaxClubWrite(UINT minCount, UINT maxCount);// set the values min/max nr of needed club-players
    UINT        GetLinesPerPage();                          // printer setting
    void        SetLinesPerPage(UINT linesPerPage);         // printer setting
    bool        GetGroupResult();                           // special resulttype
    bool        GetGlobalNameUse();                         // use of global namefile
    void        SetGlobalNameUse(bool bGlobalNameUse);      // use of a global name file
    wxString    GetGlobalNameFile();                        // name of .db file for global use of names
    void        SetGroupResult(bool bGroupResult);          // resulttype
    wxString    GetFilePrinterName();                       // filename used for printing to disk
    wxString    GetWinPrintPrefix();                        // indication of a windows-printer
    const wxString& GetPrinterName();                       // get the choosen printername
    void        SetPrinterName( const wxString& printer);   // set the new printer name
    UINT        GetNrOfGames();                             // nr of games for a match
    UINT        GetSetSize();                               // nr of games per table
    UINT        GetFirstGame();                             // first game number of a session
    void        SetFirstGame(UINT firstGame);               // first game number of a session
    wxString    GetBareMainIni();                           // get filename/extension of main ini file (should be 'bridge.ini')
    void        SetNetworkPrinting(bool bSet);              // set the search for a network printer
    bool        GetNetworkPrinting();                       // get the search for a network printer
    UINT        GetNrOfSessionPairs();                      // nr of pairs for current session
    bool        IsSessionPairAbsent(UINT sessionPair);      // check if 'sessionPair' is absent

    wxString    GetCopyrightDateTime();                     // as said, with leading/ending '\n'
    wxString    GetBaseFolder();                            // main storage folder, fallback for non-writable folders 
    bool        GetButler();                                // get the type of result-calculation
    void        SetButler(bool bOn);                        // set the type of result-calculation

    const vGroupData* GetGroupData();                       // all the info of all groups
    const SessionInfo*GetSessionInfo();                     // all the sessioninfo, inclusive groupInfo
    void        UpdateSessionInfo(const SessionInfo& info); // all updates of the schemasetup of a session
    void        DatabaseTypeSet(long type, bool bQuiet = false);

    void DataConversionBackup();
    void DataConversionRestore();
    void DataConversionSetMatch(const wxString& match);
    void DataConversionSetMatchPath(const wxString& matchPath);
    void DataConversionSetSession(UINT session);

    enum INITIALIZE   // init the variables for the xxx cfg file to its defaults
    {
          INIT_MAIN     = 0x01  // the main
        , INIT_MATCH    = 0x02  // the match
        , INIT_SESSION  = 0x04  // the session
        , INIT_ALL      = INIT_MAIN | INIT_MATCH | INIT_SESSION
    };

    void InitializeDefaults( int theType = INIT_ALL); // reset values for new match

}   // end namespace cfg

#pragma warning(suppress: 4505)   //unreferenced function with internal linkage
static bool ConfigChanged(bool bForceUpdate = false)    // if true, confighash is updated. Needed in EACH file!
{
    static int m_iCurrentConfigHash = -1;
    if (bForceUpdate) cfg::UpdateConfigHash();
    int activeHash = cfg::GetConfigHash();
    bool bChanged = activeHash != m_iCurrentConfigHash;
    m_iCurrentConfigHash = activeHash;
    return bChanged;
}
#endif
