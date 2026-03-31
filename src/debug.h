// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _DEBUG_H_
#define _DEBUG_H_
#pragma once

#include "baseframe.h"

class MyGrid;
class NameInfo;
class Console;

class Debug: public Baseframe
{
public:
    enum DebugType
    {
        DebugConsole    = 0,
        Guides          = 1,
        ScoreSlips      = 2
    };

    explicit    Debug(wxWindow* pParent, UINT pageId, DebugType type = DebugConsole);
               ~Debug() override = default;
    void        RefreshInfo() final;   // (re)populate the grid
    void        PrintPage()   final;
    void        AutotestRequestMousePositions(MyTextFile* pFile) final;

protected:
    void        BackupData      () final{/* default: do nothing */}

private:
    void HandleCommandLine      (wxString cmdLine);
    void DoCommand              (const wxString& cmd);
    void CalcScore              (const wxChar* pBuf);
    void List                   (const wxChar* pBuf);
    void GuideCard              (UINT pair, bool bAskExtra = true);
    void Usage                  ();
    void SetPrompt              ();
    UINT AtoiRound              (const wxChar * pBuf);
    UINT AtoiPair               (const wxChar * pBuf);
    UINT AtoiGroup              (const wxChar * pBuf);
    UINT AtoiTable              (const wxChar * pBuf);
    UINT AtoiGame2Set           (const wxChar * pBuf);
    void OutputText             (const wxString& msg);
    void PrintSeparator         ();
    void StringDoubler          (const wxString& msg, UINT pair);
    wxString GetNsEwString      (UINT pair, UINT round) const;
    wxString SetToGamesAsString (UINT set, bool bWide = true) const;
    wxString GetBorrowTableAsString( UINT table, UINT round) const;
    UINT GetOpponent            (UINT pair, UINT round) const;
    void InitGroupData          ();
    void GroupOverview          ();
    void TestSchemas            ();
    void OnSelectGroup          (const wxCommandEvent&);
    void OnSelectPair           (const wxCommandEvent&);
    void OnPrint                (const wxCommandEvent&);
    void OnExample              (const wxCommandEvent&);
    void OnExportSchema         (wxCommandEvent&);
    void PrintOrExample();
    void SetFocusAfter          (const wxCommandEvent&);  // set focus back to console-window
    void InitGuideStuff         ();
    void PrintScoreSlips        (UINT setSize, UINT firstSet, UINT nrOfSets, UINT repeatCount, const wxString& extra);
    void InitSlips              ();
    void PrintGuideNew          (UINT pair);
    void PrintSchemaOverviewNew ();

    wxCheckBox*     m_pCheckBoxPrintNext;
    wxCheckBox*     m_pChkBoxGuide;
    bool            m_bSchema = false;

    Console*        m_pConsole;
    bool            m_bPrintNext = false;
    UINT            m_group  = 1;       // selected group
    UINT            m_rounds;           // rounds for this group
    UINT            m_tables;           // tables for this group
    UINT            m_pairs;            // pairs for this group
    UINT            m_setSize;          // setsize for this match
    bool            m_bAllPairs;        // generate data for all higher numbered pairs
    UINT            m_linesPrinted;
    wxString        m_explanation;      // text printed on each guideletter
    SchemaInfo      m_schema;
    bool            m_bDontTest = false;
    DebugType       m_debugType;
    wxBoxSizer*     m_pHsizerGuides;    // controls used when making guides for players
    wxBoxSizer*     m_pHsizerScoreSlips;// controls used when making scoreslips for a game
    wxBoxSizer*     m_pHsizerSlipsExtra;// extra textinput for scoreslips
    wxString        m_pStatusBarInfo;
    MY_CHOICE*      m_pGroupChoice;
    MyChoiceMC*     m_pPairChoice;
    MY_CHOICE*      m_pSetSize;
    MyChoiceMC*     m_pFirstSet;
    MyChoiceMC*     m_pNrOfSets;
    MyChoiceMC*     m_pRepeatCount;
    wxTextCtrl*     m_pTxtCtrlExtra;
    wxArrayString   m_consoleOutput;

    std::vector<cfg::GROUP_DATA> m_groupData;
    cfg::GROUP_DATA* m_pActiveGroupInfo;
};

#endif
