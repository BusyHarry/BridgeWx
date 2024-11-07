// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valnum.h>
#include <wx/wxcrt.h>
#include <wx/msgdlg.h>
#include <wx/valtext.h>

#include "cfg.h"
#include "schemaInfo.h"
#include "setupSchema.h"
#include "printer.h"

//#define MY_SIZE_STATIC_TEXT wxDefaultPosition, {150,-1}
#define MY_SIZE_STATIC_TEXT wxDefaultPosition, {16*GetCharWidth(),-1}
#define CHOICE_SCHEMA   "ChoiceSchema"
#define CHOICE_GROUP    "ChoiceGroup"

SetupSchema::SetupSchema(wxWindow* a_pParent, UINT a_pageId) : Baseframe(a_pParent, a_pageId)
{
//rounds and nr of games for this match: 2*wxStaticText+wxTextCtrl
    auto nrOfRounds = new wxStaticText(this, wxID_ANY, _("Rounds:"), MY_SIZE_STATIC_TEXT);
    m_pTxtCtrlNrOfRounds = new MyTextCtrl  (this, wxID_ANY, "Rounds",MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlNrOfRounds->SetToolTip(_("Number of rounds in this session"));
    m_pTxtCtrlNrOfRounds->SetMinMax(1, schema::GetMaxRound());
    m_pTxtCtrlNrOfRounds->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusRoundsSetSize, this);
    m_pTxtCtrlNrOfRounds->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterRoundsSetSize,     this);

    auto nrOfGames = new wxStaticText(this, wxID_ANY, _("Games:  "));
    m_pTxtCtrlNrOfGames = new wxTextCtrl  (this, wxID_ANY, "NrOfGames", MY_SIZE_TXTCTRL_NUM(3), wxTE_READONLY);
    m_pTxtCtrlNrOfGames->SetToolTip(FMT(_("Number of games to play in this session (max=%u)"),cfg::MAX_GAMES));

    auto firstGame = new wxStaticText(this, wxID_ANY, _("First game:  "));
    m_pTxtCtrlFirstGame = new MyTextCtrl(this, wxID_ANY, "FirstGame", MY_SIZE_TXTCTRL_NUM(2));
    m_pTxtCtrlFirstGame->SetToolTip(_("Number of the first game (1, or f.e. 17 for session 2)"));
    m_pTxtCtrlFirstGame->SetMinMax(1, cfg::MAX_GAMES);
    m_pTxtCtrlFirstGame->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusRoundsSetSize, this);
    m_pTxtCtrlFirstGame->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterRoundsSetSize,     this);

    wxBoxSizer*     nrOfGamesSizer = new wxBoxSizer  (wxHORIZONTAL);
    nrOfGamesSizer->Add(nrOfRounds          ,0, wxALIGN_CENTER_VERTICAL);
    nrOfGamesSizer->Add(m_pTxtCtrlNrOfRounds,0);    nrOfGamesSizer->AddSpacer( 30 );
    nrOfGamesSizer->Add(nrOfGames           ,0, wxALIGN_CENTER_VERTICAL);
    nrOfGamesSizer->Add(m_pTxtCtrlNrOfGames ,0);    nrOfGamesSizer->AddSpacer( 30 );
    nrOfGamesSizer->Add(firstGame           ,0, wxALIGN_CENTER_VERTICAL);
    nrOfGamesSizer->Add(m_pTxtCtrlFirstGame ,0);


//nr of games per table = setSize:  wxStaticText + wxTextCtrl
    auto setSize = new wxStaticText(this, wxID_ANY, _("Games per table:"), MY_SIZE_STATIC_TEXT);
    m_pTxtCtrlSetSize = new MyTextCtrl(this, wxID_ANY, "GamesPerTable", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlSetSize->SetToolTip(_("Number of games per table"));
    m_pTxtCtrlSetSize->SetMinMax(1, cfg::MAX_GAMES);

    m_pTxtCtrlSetSize->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusRoundsSetSize, this);
    m_pTxtCtrlSetSize->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterRoundsSetSize,     this);

    wxBoxSizer* SetSizeSizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizeSizer->Add(setSize          , 0, wxALIGN_CENTER_VERTICAL);
    SetSizeSizer->Add(m_pTxtCtrlSetSize, 0);

//nr of groups:  wxStaticText + wxTextCtrl
    auto nrOfGroups = new wxStaticText(this, wxID_ANY, _("Groups: "), MY_SIZE_STATIC_TEXT);
    m_pTxtCtrlNrOfGroups = new MyTextCtrl(this, wxID_ANY,"Groups", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlNrOfGroups->SetToolTip(_("Number of groups that play the same games"));
    m_pTxtCtrlNrOfGroups->SetMinMax(1, cfg::MAX_GROUPS);
    m_pTxtCtrlNrOfGroups->Bind(wxEVT_KILL_FOCUS        , &SetupSchema::OnLostFocusNrOfGroups, this);
    m_pTxtCtrlNrOfGroups->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterNrOfGroups    , this);

    wxBoxSizer* nrOfGroupsSizer = new wxBoxSizer(wxHORIZONTAL);
    nrOfGroupsSizer->Add(nrOfGroups          , 0, wxALIGN_CENTER_VERTICAL);
    nrOfGroupsSizer->Add(m_pTxtCtrlNrOfGroups, 0);

// groupinfo
//1) wxStaticText+wxChoice(group)   wxButton(nextgroup) 
    m_pChoiceBoxGroup = new MY_CHOICE(this, _("Group:"), _("Info of the different groups"), Unique(CHOICE_GROUP));
    m_pChoiceBoxGroup->Bind(wxEVT_CHOICE, &SetupSchema::OnSelectGroup, this );

    auto nextGroup = new wxButton(this, wxID_ANY, _("++group"));
    nextGroup->SetToolTip(_("Info for the next group"));
    Bind(wxEVT_BUTTON, &SetupSchema::OnNextGroup, this, nextGroup->GetId() );

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
    m_pTxtCtrlPairs = new MyTextCtrl(this, wxID_ANY,"NrOfPairs", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
    m_pTxtCtrlPairs->SetMinMax(1, cfg::MAX_PAIRS_PER_GROUP);
    m_pTxtCtrlPairs->Bind(wxEVT_KILL_FOCUS,         &SetupSchema::OnLostFocusNrOfPairs, this );
    m_pTxtCtrlPairs->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupSchema::OnEnterPairs        , this );

    m_pChoiceBoxSchemas = new MyChoiceMC(this, _("Schema:"), _("Schema for this group"), Unique(CHOICE_SCHEMA));
    m_pChoiceBoxSchemas->Bind(wxEVT_CHOICE, &SetupSchema::OnSelectSchema, this );

    auto absent = new wxStaticText(this, wxID_ANY, _("Absent pair:  "));
    m_pTxtCtrlAbsent = new MyTextCtrl(this, wxID_ANY, "AbsentPair", MY_SIZE_TXTCTRL_NUM(2), wxTE_PROCESS_ENTER);
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
    vBox->AddSpacer( 30 );  vBox->Add(nrOfGamesSizer        , 0, wxALIGN_LEFT   );
    vBox->AddSpacer( 30 );  vBox->Add(SetSizeSizer          , 0, wxALIGN_LEFT   );
    vBox->AddSpacer( 30 );  vBox->Add(nrOfGroupsSizer       , 0, wxALIGN_LEFT   );
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
    wxString info;
    info = FMT(_(
                    "Rounds       :%2s    Games: %s    First game: %s\n"
                    "Games/table  :%2s\n"
                    "Groups       :%2s\n\n"
                ),
                m_pTxtCtrlNrOfRounds->GetValue(), m_pTxtCtrlNrOfGames->GetValue(), m_pTxtCtrlFirstGame->GetValue(),
                m_pTxtCtrlSetSize   ->GetValue(),
                m_pTxtCtrlNrOfGroups->GetValue()
            );
    prn::PrintLine(info);
    prn::PrintLine(_("group groupchars   pairs schema          absent pair\n"));
    //m_groupData;
    for (UINT group = 0; group < m_groupData.size(); ++group)
    {
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
    UINT            rounds  = wxAtoi(m_pTxtCtrlNrOfRounds->GetValue());
    INT_VECTOR      nameIds = schema::FindSchema(rounds, m_groupData[a_index].pairs, &schemas); if (nameIds.size()){;}

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
        wxBell();
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
        wxBell();
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
            wxBell();
            m_pTxtCtrlGroupChars->SetValue(m_groupData[grp].groupChars);    //restore original value
            return;
        }
    }

    m_groupData[grp].groupChars = grpChars;
}   // HandleGroupChars()
