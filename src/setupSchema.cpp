// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valnum.h>
#include <wx/wxcrt.h>
#include <wx/msgdlg.h>
#include <wx/valtext.h>

#include "validators.h"
#include "cfg.h"
#include "schemaInfo.h"
#include "setupSchema.h"
#include "printer.h"
#include "score.h"
#include "names.h"

#define CHOICE_SCHEMA   "ChoiceSchema"
#define CHOICE_GROUP    "ChoiceGroup"

SetupSchema::SetupSchema(wxWindow* a_pParent, UINT a_pageId) : Baseframe(a_pParent, a_pageId)
{
//rounds and nr of games for this match: 2*wxStaticText+wxTextCtrl
    auto nrOfRounds      = new wxStaticText(this, wxID_ANY, _("Rounds:"));
    m_pTxtCtrlNrOfRounds = new MyTextCtrlWithValidator  (this, wxID_ANY, "Rounds", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlNrOfRounds->SetToolTip(_("Number of rounds in this session"));
    m_pTxtCtrlNrOfRounds->SetMinMax(1, schema::GetMaxRound());
    m_pTxtCtrlNrOfRounds->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusRoundsSetSize, this);
    m_pTxtCtrlNrOfRounds->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterRoundsSetSize,     this);

    auto nrOfGames      = new wxStaticText(this, wxID_ANY, "    " + _("Games:"));
    m_pTxtCtrlNrOfGames = new wxTextCtrl  (this, wxID_ANY, "NrOfGames", MY_SIZE_TXTCTRL_NUM(3), wxTE_READONLY);
    m_pTxtCtrlNrOfGames->SetToolTip(FMT(_("Number of games to play in this session (max=%u)"),cfg::MAX_GAMES));

    auto firstGame      = new wxStaticText(this, wxID_ANY, "    " + _("First game:"));
    m_pTxtCtrlFirstGame = new MyTextCtrlWithValidator(this, wxID_ANY, "FirstGame", MY_SIZE_TXTCTRL_NUM(2));
    m_pTxtCtrlFirstGame->SetToolTip(_("Number of the first game (1, or f.e. 17 for session 2)"));
    m_pTxtCtrlFirstGame->SetMinMax(1, cfg::MAX_GAMES);
    m_pTxtCtrlFirstGame->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusRoundsSetSize, this);
    m_pTxtCtrlFirstGame->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterRoundsSetSize,     this);

    wxFlexGridSizer* fgs = new wxFlexGridSizer(3 /*rows*/, 6 /*columns*/, 20 /*v-gap*/, 10 /*h-gap*/);
    // row 1
    fgs->Add(nrOfRounds          , 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(m_pTxtCtrlNrOfRounds, 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(nrOfGames           , 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(m_pTxtCtrlNrOfGames , 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(firstGame           , 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(m_pTxtCtrlFirstGame , 0, wxALIGN_CENTER_VERTICAL);

//nr of games per table = setSize:  wxStaticText + wxTextCtrl
    auto setSize = new wxStaticText(this, wxID_ANY, _("Games per table:"));
    m_pTxtCtrlSetSize = new MyTextCtrlWithValidator(this, wxID_ANY, "GamesPerTable", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlSetSize->SetToolTip(_("Number of games per table"));
    m_pTxtCtrlSetSize->SetMinMax(1, cfg::MAX_GAMES);

    m_pTxtCtrlSetSize->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusRoundsSetSize, this);
    m_pTxtCtrlSetSize->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterRoundsSetSize,     this);
    #define Dummy new wxStaticText(this, wxID_ANY,"")   /* a dummy entry for the flex grid sizer*/
    // row 2
    fgs->Add(setSize          , 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(m_pTxtCtrlSetSize, 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(Dummy); fgs->Add(Dummy); fgs->Add(Dummy); fgs->Add(Dummy); // fill flexsizer

//nr of groups:  wxStaticText + wxTextCtrl
    auto nrOfGroups      = new wxStaticText(this, wxID_ANY, _("Groups:"));
    m_pTxtCtrlNrOfGroups = new MyTextCtrlWithValidator(this, wxID_ANY,"Groups", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlNrOfGroups->SetToolTip(_("Number of groups that play the same games"));
    m_pTxtCtrlNrOfGroups->SetMinMax(1, cfg::MAX_GROUPS);
    m_pTxtCtrlNrOfGroups->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusNrOfGroups, this);
    m_pTxtCtrlNrOfGroups->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterNrOfGroups    , this);
    // row 3
    fgs->Add(nrOfGroups          , 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(m_pTxtCtrlNrOfGroups, 0, wxALIGN_CENTER_VERTICAL);

// groupinfo
//1) wxStaticText+wxChoice(group)   wxButton(nextgroup)
    m_pChoiceBoxGroup = new MY_CHOICE(this, _("Group:"), _("Info of the different groups"), Unique(CHOICE_GROUP));
    m_pChoiceBoxGroup->Bind(wxEVT_CHOICE, &SetupSchema::OnSelectGroup, this );

    auto nextGroup = new wxButton(this, wxID_ANY, _("++group"));
    nextGroup->SetToolTip(_("Info for the next group"));
    nextGroup->Bind(wxEVT_BUTTON, &SetupSchema::OnNextGroup, this );

    wxBoxSizer* groupCountSizer = new wxBoxSizer(wxHORIZONTAL);
    groupCountSizer->MyAdd(m_pChoiceBoxGroup); groupCountSizer->AddSpacer(20);
    groupCountSizer->Add(nextGroup);

//2)wxStaticText+wxTextCtrl(groupletters) + wxStaticText+wxTextCtrl(nr of pairs) + wxStaticText+wxChoice(schema) +  wxStaticText+wxTextCtrl(absent pair)
    wxStaticText* groupChars = new wxStaticText(this, wxID_ANY, _("Group characters:  "));
    m_pTxtCtrlGroupChars     = new wxTextCtrl  (this, wxID_ANY, "GroupChars", MY_SIZE_TXTCTRL_NUM(3), wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_ALPHA));
    m_pTxtCtrlGroupChars->SetMaxLength(2);
    m_pTxtCtrlGroupChars->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusGroupChars, this );
    m_pTxtCtrlGroupChars->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterGroupChars    , this );

    auto pairs = new wxStaticText(this, wxID_ANY, _("Number of pairs:  "));
    m_pTxtCtrlPairs = new MyTextCtrlWithValidator(this, wxID_ANY,"NrOfPairs", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlPairs->SetMinMax(1, cfg::MAX_PAIRS_PER_GROUP);
    m_pTxtCtrlPairs->Bind(wxEVT_KILL_FOCUS,         &SetupSchema::OnLostFocusNrOfPairs, this );
    m_pTxtCtrlPairs->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterPairs        , this );

    m_pChoiceBoxSchemas = new MyChoiceMC(this, _("Schema")+':', _("Schema for this group"), Unique(CHOICE_SCHEMA));
    m_pChoiceBoxSchemas->Bind(wxEVT_CHOICE, &SetupSchema::OnSelectSchema, this );

    auto absent = new wxStaticText(this, wxID_ANY, _("Absent pair:  "));
    m_pTxtCtrlAbsent = new MyTextCtrlWithValidator(this, wxID_ANY, "AbsentPair", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlAbsent->SetMinMax(0, cfg::MAX_PAIRS_PER_GROUP);
    m_pTxtCtrlAbsent->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusAbsent, this );
    m_pTxtCtrlAbsent->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterAbsent    , this );

    wxBoxSizer* groupInfoSizer = new wxBoxSizer(wxHORIZONTAL);
    groupInfoSizer->Add(groupChars, 0, wxALIGN_CENTER_VERTICAL); groupInfoSizer->Add  (m_pTxtCtrlGroupChars, 0); groupInfoSizer->AddSpacer( 30 );
    groupInfoSizer->Add(pairs     , 0, wxALIGN_CENTER_VERTICAL); groupInfoSizer->Add  (m_pTxtCtrlPairs     , 0); groupInfoSizer->AddSpacer( 30 );
    groupInfoSizer->Add(absent    , 0, wxALIGN_CENTER_VERTICAL); groupInfoSizer->Add  (m_pTxtCtrlAbsent    , 0); groupInfoSizer->AddSpacer(30);
    groupInfoSizer->MyAdd(m_pChoiceBoxSchemas, 1);

// action buttons: keep/undo
    auto okCancel = CreateOkCancelButtons();

    // add all sizers to vertical sizer
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Schema entry for this session"));
    vBox->AddSpacer( 30 );  vBox->Add(fgs                   , 0, wxALIGN_LEFT   );
    vBox->AddSpacer( 30 );  vBox->Add(groupCountSizer       , 0, wxALIGN_LEFT   );
    vBox->AddSpacer(  5 );  vBox->Add(groupInfoSizer        , 0                 );
    vBox->AddStretchSpacer( 30 );  vBox->Add(okCancel       , 0, wxALL | wxALIGN_CENTER, MY_BORDERSIZE );

    this->SetSizer(vBox);
    RefreshInfo();      // populate the controls

    AUTOTEST_ADD_WINDOW(m_pTxtCtrlNrOfRounds, "Rounds"      );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlFirstGame , "FirstGame"   );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlSetSize   , "SetSize"     );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlNrOfGroups, "Groups"      );
    AUTOTEST_ADD_WINDOW(nextGroup           , "NextGroup"   );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlGroupChars, "GroupChars"  );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlPairs     , "Pairs"       );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlAbsent    , "Absent"      );
    AUTOTEST_ADD_WINDOW(m_pChoiceBoxSchemas , CHOICE_SCHEMA );
    AUTOTEST_ADD_WINDOW(m_pChoiceBoxGroup   , CHOICE_GROUP  );
    m_description = "SetupSchema";
}   // SetupSchema()

void SetupSchema::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    AutoTestAddWindowsNames(a_pFile, m_description);
}   // AutotestRequestMousePositions()

// layout is done, now we have our setup/update functions
void SetupSchema::RefreshInfo()
{
    const auto& sessionInfo = *cfg::GetSessionInfo();
    UINT games   = sessionInfo.nrOfGames;
    UINT setSize = sessionInfo.setSize;
    m_pTxtCtrlNrOfRounds->SetValue(U2String( games/setSize) );
    m_pTxtCtrlNrOfGames ->SetValue(U2String( games        ) );
    m_pTxtCtrlSetSize   ->SetValue(U2String( setSize      ) );
    m_pTxtCtrlFirstGame ->SetValue(U2String( sessionInfo.firstGame ) );

    m_groupData = sessionInfo.groupData;  // get all info about existing groups
    UINT ng     = m_groupData.size();
    m_pTxtCtrlNrOfGroups->SetValue(U2String(ng));

    RefreshInfoGroup(0);    // show info of first group
    Layout();
}   // RefreshInfo()

void SetupSchema::OnSelectGroup(wxCommandEvent&)
{
    AUTOTEST_BUSY("selectGroup");
    int choice = m_pChoiceBoxGroup->GetSelection();
    LogMessage("SetupSchema::SelectGroup(%i)", choice);
    RefreshInfoGroup(choice);
}   // OnSelectGroup()

void SetupSchema::OnSelectSchema(wxCommandEvent&)
{
    AUTOTEST_BUSY("selectSchema");
    int grp                     = m_pChoiceBoxGroup->GetSelection();            // this group is changing
    wxString schema             = m_pChoiceBoxSchemas->GetStringSelection();
    int id                      = schema::GetId(schema);
    m_groupData[grp].schema     = schema;
    m_groupData[grp].schemaId   = id;
    LogMessage("SetupSchema::SelectSchema(%i,%i)", grp, id);
}   // OnSelectSchema()

void SetupSchema::BackupData()
{
    //updating 1) nrOfGames 2) setSize 3) groupinfo 4) firstGame
    // NB 1/2 MUST be updated before 3 (or we must change the update to include ALL of the data at once...)


    if (m_groupData.size() > 1)
    {   // if more then one group, groupchars should not be empty
        wxChar grp = 'A';
        for (auto& it : m_groupData)
        {
            if (it.groupChars == ES) it.groupChars = grp;
            ++grp;
        }
    }

    // If the schema has changed, probably the best thing to do is clear all entered data and start clean.
    // But we are stubborn, so we try to 'rescue' as much data as possible!
    // now check if we already have scores and/or assignments. If so, try to adjust the data to be consistent
    score::ReadScoresFromDisk();
    names::InitializePairNames();
    if (score::ExistGameData() || names::ExistAssignments())
    {   // check if number of players has changed in any group, if so, update data
        bool        bDataChanged = false;
        const auto& oldGroupData = cfg::GetSessionInfo()->groupData;
        UINT        oldSize      = oldGroupData.size();
        UINT        newSize      = m_groupData.size();

        for (UINT index = newSize; index < oldSize; ++index)
        {   // group(s) removed, so remove all scores/assignments for the pairs in these groups
            auto oldOffset = oldGroupData[index].groupOffset;
            auto oldPairs  = oldGroupData[index].pairs;
            for (UINT pair = 1; pair <= oldPairs; ++pair)
            {
                bDataChanged |= score::DeleteScoresFromPair    (pair + oldOffset);
                bDataChanged |= names::DeleteAssignmentFromPair(pair + oldOffset);
            }
        }

        UINT minSize = std::min(newSize, oldSize);
        for (UINT index = 0; index < minSize; ++index)
        {   // only need to check as many groups as the smallest item contains
            auto    oldPairs    = oldGroupData[index].pairs;
            auto    newOffset   = m_groupData[index].groupOffset;
            auto    newPairs    = m_groupData[index].pairs;
            int     deltaPairs  = newPairs - oldPairs;

            if (deltaPairs < 0)
            {   // less pairs in new group: remove scores/assignments of the removed pairs
                for (UINT pair = oldPairs; pair > newPairs; --pair)
                {
                    bDataChanged |= score::DeleteScoresFromPair    (pair + newOffset);
                    bDataChanged |= names::DeleteAssignmentFromPair(pair + newOffset);
                }
            }
            if (deltaPairs)
            {   // more/less pairs in group: adjust pairnrs in scores/assignments for next group(s)
                UINT pair = newOffset + oldPairs + 1;   // first pairnr to change
                bDataChanged |= score::AdjustPairNrs    (pair, deltaPairs);
                bDataChanged |= names::AdjustAssignments(pair, deltaPairs);
            }
        }
        if (bDataChanged)
        {
            wxString msg = _("scores/assignments adapted as result of changes in schema");
            MyLogDebug("%s", msg);
            //MyMessageBox(msg, _("schema"));
            score::WriteScoresToDisk();
            names::WriteAssignmentsToDisk();
        }
    }   // changed scores/assignments

    for (const auto& group : m_groupData)
    {
        if (group.schemaId == schema::ID_NONE)
        {
            wxString msg = _("not all groups have a valid schema");
            MyLogDebug("SetupSchema: %s", msg);
            MyMessageBox( msg, _("Warning"));
            break;
        }
    }
    cfg::SessionInfo si;
    si.nrOfGames = wxAtoi(m_pTxtCtrlNrOfGames->GetValue());
    si.setSize   = wxAtoi(m_pTxtCtrlSetSize  ->GetValue());
    si.firstGame = wxAtoi(m_pTxtCtrlFirstGame->GetValue());
    si.groupData = m_groupData;
    cfg::UpdateSessionInfo(si);
}   // BackupData()

void SetupSchema::PrintPage()
{
    bool bResult = prn::BeginPrint(_("Schemasetup page:\n")); MY_UNUSED(bResult);
    prn::PrintLine(FMT("%-13s:%2s    %s: %s    %s: %s\n"
                        , _("Rounds"    ), m_pTxtCtrlNrOfRounds->GetValue()
                        , _("Games"     ), m_pTxtCtrlNrOfGames ->GetValue()
                        , _("First game"), m_pTxtCtrlFirstGame ->GetValue()
                      )
                  );
    prn::PrintLine(FMT("%-13s:%2s\n"  , _("Games/table"), m_pTxtCtrlSetSize   ->GetValue() ));
    prn::PrintLine(FMT("%-13s:%2s\n\n", _("Groups"     ), m_pTxtCtrlNrOfGroups->GetValue() ));

    prn::PrintLine(FMT("%-6s%-13s%-6s%-16s%s\n"
                        , _("group"      )
                        , _("groupchars" )
                        , _("pairs"      )
                        , _("schema"     )
                        , _("absent pair")
                      )
                  );
    //m_groupData;
    for (UINT group = 0; group < m_groupData.size(); ++group)
    {
        wxString info;
        info = FMT("%2u    %-12s %-3u   %-15s %-u\n",
                    group+1,
                    m_groupData[group].groupChars,
                    m_groupData[group].pairs,
                    m_groupData[group].schema,
                    m_groupData[group].absent
                );
        prn::PrintLine(info);
    }

    prn::PrintLine(cfg::GetCopyrightDateTime());
    prn::EndPrint();
}   // PrintPage()

void SetupSchema::OnOk()
{
    LogMessage("SetupSchema::OnOk()");
    BackupData();
    cfg::FLushConfigs();    // update diskfiles
}   // OnOk()

void SetupSchema::OnCancel()
{
    LogMessage("SetupSchema::Cancel()");
    RefreshInfo();  // restore original content
}   // OnCancel()

void SetupSchema::OnNextGroup(wxCommandEvent&)
{
    AUTOTEST_BUSY("nextGroup");
    UINT count  = m_pChoiceBoxGroup->GetCount();
    if (count == 1) return; // nothing to do
    UINT as     = m_pChoiceBoxGroup->GetSelection();    //ActiveSelection
    as          = (as+1) % count;
    RefreshInfoGroup(as);
    LogMessage("SetupSchema::OnNextGroup(%u)",as);
}   // OnNextGroup()

void SetupSchema::RefreshInfoGroup(int a_index)
{
    m_pChoiceBoxGroup->Init(m_groupData.size(), a_index);

    wxArrayString   schemas;
    INT_VECTOR      nameIds;
    UINT            rounds  = wxAtoi(m_pTxtCtrlNrOfRounds->GetValue());
    schema::FindSchema(rounds, m_groupData[a_index].pairs, nameIds, &schemas); if (nameIds.size()){;}

    m_pChoiceBoxSchemas ->Init(schemas, m_groupData[a_index].schema);   // (try to) set original name as preset
    m_pTxtCtrlGroupChars->SetValue( m_groupData[a_index].groupChars);
    m_pTxtCtrlPairs     ->SetValue( U2String(m_groupData[a_index].pairs));
    m_pTxtCtrlAbsent    ->SetValue( U2String(m_groupData[a_index].absent));
    Layout();
}   // RefreshInfoGroup()

void SetupSchema::OnLostFocusGroupChars(wxFocusEvent& a_event )
{
    a_event.Skip();
    if (m_bIsScriptTesting) return;
    LogMessage("SetupSchema::OnLostFocusGroupChars()");
    HandleGroupChars();
}   // OnLostFocusGroupChars()

void SetupSchema::OnEnterGroupChars(wxCommandEvent&)
{
    LogMessage("SetupSchema::OnEnterGroupChars()");
    HandleGroupChars();
}   // OnEnterGroupChars()

void SetupSchema::OnLostFocusNrOfPairs(wxFocusEvent& a_event)
{
    a_event.Skip();
    if (m_bIsScriptTesting) return;
    LogMessage("SetupSchema::OnLostFocusNrOfPairs()");
    HandlePairs();
}   // OnLostFocusNrOfPairs()

void SetupSchema::OnLostFocusAbsent(wxFocusEvent& a_event)
{
    a_event.Skip();
    if (m_bIsScriptTesting) return;     // if testing, {ENTER} will have handled it
    LogMessage("SetupSchema::OnLostFocusAbsent()");
    HandleAbsent();
}   // OnLostFocusAbsent()

void SetupSchema::OnLostFocusGames(wxFocusEvent& a_event)
{
    a_event.Skip();
    if (m_bIsScriptTesting) return;     // if testing, {ENTER} will have handled it
    LogMessage("SetupSchema::OnLostFocusGames()");
}   // OnLostFocusGames()

void SetupSchema::OnEnterRoundsSetSize(wxCommandEvent& )
{
    LogMessage("SetupSchema::OnEnterRoundsSetSize()");
    UpdateNrOfGames();
//       a_event.Skip(); will ring the bell ??????????
}   // OnEnterRoundsSetSize()

void SetupSchema::OnLostFocusRoundsSetSize(wxFocusEvent& a_event)
{
    a_event.Skip();
    if (m_bIsScriptTesting) return;     // if testing, {ENTER} will have handled it
    LogMessage("SetupSchema::OnLostFocusRoundsSetSize()");
    UpdateNrOfGames();
}   // OnLostFocusRoundsSetSize()

void SetupSchema::OnLostFocusNrOfGroups(wxFocusEvent& a_event)
{
    a_event.Skip();
    if (m_bIsScriptTesting) return;     // if testing, {ENTER} will have handled it
    LogMessage("SetupSchema::OnLostFocusNrOfGroups()");
    HandleNrOfGroups();
}   // OnLostFocusNrOfGroups()

void SetupSchema::UpdateNrOfGames()
{
    AUTOTEST_BUSY("NrG_FG_SS_R");
    UINT rounds     = wxAtoi(m_pTxtCtrlNrOfRounds->GetValue());
    UINT setSize    = wxAtoi(m_pTxtCtrlSetSize   ->GetValue());
    UINT firstGame  = wxAtoi(m_pTxtCtrlFirstGame ->GetValue());
    UINT games      = rounds * setSize;

    if ( games + firstGame - 1 > cfg::MAX_GAMES)
    {
        m_pTxtCtrlNrOfGames->SetForegroundColour(*wxRED);
        RingBell();
    }
    else
    {
        m_pTxtCtrlNrOfGames->SetForegroundColour(*wxBLACK);
    }

    Refresh();  //(only) needed when backgroundcolour has changed
    m_pTxtCtrlNrOfGames->SetValue(U2String(games));

    RefreshInfoGroup( m_pChoiceBoxGroup->GetSelection() ); //perhaps used schema is invalid...
}   // UpdateNrOfGames()

void SetupSchema::OnEnterPairs(wxCommandEvent&)
{
    LogMessage("SetupSchema::OnEnterPairs()");
    HandlePairs();
}   // OnEnterPairs()

void SetupSchema::OnEnterAbsent(wxCommandEvent&)
{
    LogMessage("SetupSchema::OnEnterAbsent()");
    HandleAbsent();
}   // OnEnterAbsent()

void SetupSchema::OnEnterNrOfGroups(wxCommandEvent&)
{
    LogMessage("SetupSchema::OnEnterNrOfGroups()");
    HandleNrOfGroups();
}   // OnEnterNrOfGroups()

void SetupSchema::HandlePairs()
{
    AUTOTEST_BUSY("Pairs");
    int grp    = m_pChoiceBoxGroup->GetSelection();            // this group is changing
    UINT pairs = wxAtoi(m_pTxtCtrlPairs->GetValue());
    if (pairs != m_groupData[grp].pairs)
    {
        if (pairs < m_groupData[grp].absent)
        {
            m_groupData[grp].absent = 0;
        }
        m_groupData[grp].pairs = pairs;
        m_groupData[grp].schema = ES;
        m_groupData[grp].schemaId = schema::ID_NONE;
        RefreshInfoGroup(grp);
        for ( ; static_cast<size_t>(grp) < m_groupData.size()-1; ++grp)
        {   // update groupoffset for higher numbered groups
            UINT nextGroupOffset = m_groupData[grp].groupOffset + m_groupData[grp].pairs;
            m_groupData[grp+1LL].groupOffset = nextGroupOffset;
        }
    }
}   // HandlePairs()

void SetupSchema::HandleNrOfGroups()
{
    AUTOTEST_BUSY("NrOfGroups");
    int oldCount        = m_groupData.size();
    int newCount        = wxAtoi(m_pTxtCtrlNrOfGroups->GetValue());
    int oldSelection    = m_pChoiceBoxGroup->GetSelection();

    if (newCount == oldCount) return;   // no changes
    if (newCount == 0) newCount = 1;    // should not happen
    while (newCount < oldCount)
    {   // remove groups
        m_groupData.pop_back();
        --oldCount;
    }

    while (newCount > oldCount)
    {   // add groups
        cfg::GROUP_DATA grp;
        grp.groupChars   = FMT("%c%c", 'A'+oldCount,'A'+oldCount);
        grp.groupOffset  = m_groupData[oldCount-1LL].groupOffset + m_groupData[oldCount-1LL].pairs;
        m_groupData.push_back(grp);
        ++oldCount;
    }

    if (oldSelection >= newCount)
        oldSelection = 0;
    RefreshInfoGroup(oldSelection);
}   // HandleNrOfGroups()

void SetupSchema::HandleAbsent()
{
    AUTOTEST_BUSY("Absent");
    int grp     = m_pChoiceBoxGroup->GetSelection();            // this group is changing
    UINT absent = wxAtoi(m_pTxtCtrlAbsent->GetValue());
    if (absent <=  m_groupData[grp].pairs)
    {
        m_groupData[grp].absent = absent;
    }
    else
    {
        m_groupData[grp].absent = 0;
        m_pTxtCtrlAbsent->SetValue("0");
        RingBell();
    }
}   // HandleAbsent()

void SetupSchema::HandleGroupChars()
{
    AUTOTEST_BUSY("GroupChars");
    UINT        grp      = m_pChoiceBoxGroup   ->GetSelection();       // this group is changing
    wxString    grpChars = m_pTxtCtrlGroupChars->GetValue().Upper();
    for (UINT ii = 0; ii < m_groupData.size(); ++ii)
    {
        if (ii == grp) continue;
        if (m_groupData[ii].groupChars == grpChars)
        {
            MyLogError(_("Equal groupcharacters for group %u and %u"), ii+1, grp+1);
            RingBell();
            m_pTxtCtrlGroupChars->SetValue(m_groupData[grp].groupChars);    //restore original value
            return;
        }
    }

    m_groupData[grp].groupChars = grpChars;
}   // HandleGroupChars()
