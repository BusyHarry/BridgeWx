// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined_SLIPSERVER_H_
#define _SLIPSERVER_H_
#pragma once

#include "baseframe.h"

class MyGrid;
class wxRadioBox;
class wxFileSystemWatcher;
class wxFileSystemWatcherEvent;
class wxSocketServer;
class wxSocketEvent;
class wxSocketBase;

class SlipServer : public Baseframe
{
public:
    explicit     SlipServer     (wxWindow* pParent, UINT pageId);
                ~SlipServer     ()          override;
    void         RefreshInfo    ()          override final;     // (re)populate the grid
    void         PrintPage      ()          override final;     // not really useful here..
    void         AutotestRequestMousePositions(MyTextFile* pFile) override final;

protected:
    void         OnOk           ()          override final;     // accept button pressed
    void         OnCancel       ()          override final;     // cancel button pressed
    virtual void DoSearch       (wxString&) override final;     // handler for 'any' search in derived class
    virtual void BackupData     ()          override final;     // backup changed data (not really useful here)

private:
    static const UINT SERVER_MSG_ID = 0xFE;     // messages from client should start with this id
    static const UINT SERVER_PORT   = 45678;    // the port used for tx/rx

    enum class SlipResult
    {                           // returned status for an incoming msg if using sockets
          ERROR_NONE = 0        // no error
        , ERROR_BAD_CMD         // unkown cmd supplied
        , ERROR_PARAM_COUNT     // wrong nr of parameters for a cmd
        , ERROR_PARAM_OOR       // one or more Parameter(s) Out Of Range
        , ERROR_FORMAT          // wrong format of cmd
    };

    enum class InputChoice
    {                           // radiobutton choice: file or socket
          InputFile    = 0
        , InputNetwork = 1
    };

    enum class TableBackground
    {                           // possible backgroundcolor types
          NotPresent = 0
        , NotReady   = 1
        , Ready      = 2
        , LoggedIn   = 3
    };

    struct TableInfo
    {                           // running status for all groups/rounds/tables/sets
        bool bPresent = false;
        bool bReady   = false;
    };

    struct GameInputData
    {                           // incoming data for session result as struct for easier use in other methods
        UINT session = 0; UINT group = 0; UINT table  = 0; UINT round    = 0;
        UINT ns      = 0; UINT ew    = 0; UINT game   = 0; UINT declarer = 0;
        UINT level   = 0; UINT suit  = 0; int  tricks = 0; UINT doubled  = 0;
        int  nsScore = 0;
    };

    enum class Declarer
    {                   // the possible declarer types
          Nodata = 0  , dclMin = Nodata  // no data entered
        , NP            // not played
        , Pass          // bye
        , North
        , East
        , South
        , West        , dclMax = West
    };

    enum class Suits
    {                   // the possible suits
          Clubs = 1   , sMin = Clubs
        , Diamonts
        , Hearts
        , Spades
        , NoTrump     , sMax = NoTrump
    };

    enum class Doubled
    {                   // the doubled state
          NotDoubled = 0  , dblMin = NotDoubled
        , Doubled
        , Redoubled       , dblMax = Redoubled
    };

    void        Add2Log    (const wxString& newMsg, bool bAddTime = false); // append time and 'newMsg' to log window and add a '\n'
    wxString    ContractAsString    (const GameInputData& data, bool bNs);  // get slipcontract as string
    void        CreateFileWatcher   (bool bCreate);                         // create or delete filewatcher
    void        CreateHtmlTableInfo (MyTextFile& file);                     // schema to use in .php
    void        CreateNetworkWatcher(bool bCreate);                         // create or delete socket watcher
    wxString    DateYMD             ();                                     // get date as '2025.10.07'
    void        DisplayGroupsReady  ();                                     // color group ready: true->green, false->red
    void        DisplayTableReady   (UINT group, UINT table, TableBackground tbg);      // color ready: true->green, false->red
    wxString    EscapeHtmlChars     (const wxString& str);                  // escape special chars in html
    wxString    GetLogFile          ();                                     // name of log-file to use in .php for logging
    wxString    GetMyIpv4           ();                                     // get ipv4 of current machine
    wxString    GetSlipResultsFile  (bool bfilenameOnly = false);           // name of file to receive the slip-results
    void        HandleInputSelection(int selection);                        // 0=file, 1=network
    bool        HandleResultFile    ();                                     // handle a file contaning results
    SlipResult  HandleResultLine    (const wxString& result);               // handle a line from the resultsfile: comment, logon, slipresult, ready. return error, 0 if no error
    bool        HasPlayed           (const schema::NS_EW& pairs, UINT set, UINT groupOffset);   // check if pairs have played this set
    bool        OkGameData          (const GameInputData& data);            // check if gamedata is correct
    void        OnClearLog          (wxCommandEvent& evt);                  // clear log window, append it to logfile
    void        OnFileSystemEvent   (wxFileSystemWatcherEvent& event);      // the watched file has changed
    void        OnGenHtmlSlipData   (wxCommandEvent&);                      // generate a .php definition/language file
    void        OnInputChoice       (wxCommandEvent& event);                // new input method selected
    void        OnNextRound         (wxCommandEvent&);                      // easier choice for next round
    bool        OkPairs             (const GameInputData& data);            // check if pairs are in this group
    void        OnSelectRound       (wxCommandEvent&);                      // new round selected
    void        OnServerEvent       (wxSocketEvent& event);                 // new connection
    void        OnSocketEvent       (wxSocketEvent& event);                 // something coming in
    void        SetupGrid           ();                                     // (re-)create grid, if config changes
    void        SocketGetInput      (wxSocketBase *pSock);                  // get <len><msg> and handle it
    void        SocketPutResult     (wxSocketBase *pSock, SlipResult error, const char buf[]);  // put result buf: <id><err><len><msg>
    void        UpdateTableInfo     (UINT round, bool bUpdateDisplay=true); // update results to show if round is ready

    std::vector< std::vector< std::vector<TableInfo> > > m_tableInfo;       // [round][group][table].TableInfo all 1-based!
    UINT                    m_activeRound;      // the (current) round to show the info (1-based)
    bool                    m_bCancelInProgress;// true, if cancel pressed
    bool                    m_bDataChanged;     // true, if we got new scores
    UINT                    m_groups;           // nr of groups for this match
    size_t                  m_linesReadInResult;// nr of lines already read in slipresult file
    wxString                m_logFile;          // filename for appending contents of m_pLog
    UINT                    m_maxTable;         // max nr of tables over all groups
    int                     m_numClients;       // number of clients currently connected
    MY_CHOICE*              m_pChoiceBoxRound;  // choose the round to handle its info
    wxFileSystemWatcher*    m_pFsWatcher;       // the file system watcher
    wxRadioBox*             m_pInputChoice;     // input through a file or a special network connection
    wxTextCtrl*             m_pLog;             // show gotten data
    wxSocketServer*         m_pSocketServer;    // the server listening for slip-results
    UINT                    m_rounds;           // nr of rounds for this session
    std::vector<UINT>       m_tables;           // number of tables in each group
    wxString                m_tempLogFile;      // file, used temporarily for saving contents of m_plog
    MyGrid*                 m_theGrid;          // the grid to show progress of slipdata entry
};
#endif
