// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include "wx/defs.h"
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/dataview.h>
#include <wx/listctrl.h>
#include <wx/filepicker.h>

#include <wx/valtext.h>
#include <wx/valnum.h>
#include <wx/dcclient.h>
#include <wx/msgdlg.h>
#include <wx/valgen.h>
#include <wx/combobox.h>

#include "validators.h"
#include "setupGame.h"
#include "cfg.h"
#include "printer.h"

SetupGame::SetupGame(wxWindow* a_pParent, UINT a_pageId) :Baseframe(a_pParent, a_pageId)
{
    wxFlexGridSizer* fgs = new wxFlexGridSizer(10, 2, 9, 25);
    // match description
    wxStaticText* txtDescription = new wxStaticText(this, wxID_ANY, _("Description:"));
    txtDescription->SetToolTip(_("Short description for this match"));
    m_pTxtCtrlDescr = new wxTextCtrl(this, wxID_ANY, "GameDescription");
    m_pTxtCtrlDescr->SetMaxLength(cfg::MAX_DESCRIPTION);
    fgs->Add(txtDescription, 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(m_pTxtCtrlDescr, 1, wxEXPAND);

// max mean percentage/imps for session you have not played (in result over all sessions)
    m_bButler = !cfg::GetButler();  // init inverse of current setting, so RefreshInfo() will do initial setup!
    m_pStaticTxtMaxMean = new wxStaticText(this, wxID_ANY, "");
    m_pTxtCtrlMaxMean = new MyTextCtrlWithValidator(this, wxID_ANY, "MaxMean", MY_SIZE_TXTCTRL_NUM(5));
    m_pTxtCtrlMaxMean->Clear(); // remove 'title' from input..
    fgs->Add(m_pStaticTxtMaxMean, 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(m_pTxtCtrlMaxMean, 0);

// minmax count for club result
    wxStaticText* txtMinMaxClub = new wxStaticText(this, wxID_ANY, _("Min/Max clubpairs:"));

    //sorry, but validators don't work when min-value is non-zero
    // see https://github.com/wxWidgets/wxWidgets/issues/12968
    // so we check the values when storing them
    //20231201: seems to be resolved in 3.2.4, but only validation on loosing focus, not enter?????

    wxStaticText*   txtMin = new wxStaticText(this, wxID_ANY, _("min:") + "  ");
    m_pTxtCtrlMin = new MyTextCtrlWithValidator  (this, wxID_ANY, "Min", MY_SIZE_TXTCTRL_NUM(3), wxTE_PROCESS_ENTER);
    m_pTxtCtrlMin->SetMinMax(1, cfg::MAX_PAIRS);
    m_pTxtCtrlMin->Bind(wxEVT_KILL_FOCUS        , &SetupGame::OnFocusLostMin, this); //not needed because validator bug
    m_pTxtCtrlMin->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupGame::OnEnterMin    , this);
    wxStaticText*   txtMax = new wxStaticText(this, wxID_ANY, _("max:") + "  ");
    m_pTxtCtrlMax = new MyTextCtrlWithValidator  (this, wxID_ANY,"Max",  MY_SIZE_TXTCTRL_NUM(3), wxTE_PROCESS_ENTER);
    m_pTxtCtrlMax->SetMinMax(1, cfg::MAX_PAIRS);
    m_pTxtCtrlMax->Bind(wxEVT_KILL_FOCUS        , &SetupGame::OnFocusLostMax, this);
    m_pTxtCtrlMax->Bind(wxEVT_COMMAND_TEXT_ENTER, &SetupGame::OnEnterMax    , this);

    wxBoxSizer* minMaxSizer = new wxBoxSizer  (wxHORIZONTAL);
    minMaxSizer->Add(txtMin       , 0, wxALIGN_CENTER_VERTICAL);    // minimum size so txtctrl is adjacent
    minMaxSizer->Add(m_pTxtCtrlMin);
    minMaxSizer->AddSpacer(50);
    minMaxSizer->Add(txtMax       , 0, wxALIGN_CENTER_VERTICAL);
    minMaxSizer->Add(m_pTxtCtrlMax);

    txtMinMaxClub->SetToolTip(_("Minimun and maximum nr of pairs that count for the clubresult"));
    fgs->Add(txtMinMaxClub, 0, wxALIGN_CENTER_VERTICAL);
    fgs->Add(minMaxSizer, 1);

// Neuberg calculation for games not played bij all players or with some arbitration
    m_pChkBoxNeuberg = new wxCheckBox(this, wxID_ANY, _("Neuberg"));
//    m_pChkBoxNeuberg = new wxCheckBox(this, wxID_ANY, "Neuberg");
    m_pChkBoxNeuberg->SetToolTip(_("Neuberg calculation of sessionresult on adjusted scores"));

// use 'weighted mean' of the result of each match when calculating result of N sessions
    m_pChkBoxWeightedMean = new wxCheckBox(this, wxID_ANY, _("Weighted average"));
    m_pChkBoxWeightedMean->SetToolTip(_("Weighted average calculation in results over multiple sessions"));

    m_pChkBoxGroupResult = new wxCheckBox(this, wxID_ANY, _("Groupresult"));
    m_pChkBoxGroupResult->SetToolTip(_("Result for the group, sorted on pairnr, and results per game"));

    m_pChkBoxGlobalNames = new wxCheckBox(this, wxID_ANY, _("Global name database"));
    m_pChkBoxGlobalNames->SetToolTip(_("Use of a global name-database i.s.o names per match"));

    m_pChkBoxClock = new wxCheckBox(this, wxID_ANY, _("Clock"));
    m_pChkBoxClock->SetToolTip(_("Display a clock in the statusbar"));

    wxBoxSizer* checkerSizer = new wxBoxSizer(wxHORIZONTAL);
    checkerSizer->Add( m_pChkBoxNeuberg      ); checkerSizer->AddSpacer(20);
    checkerSizer->Add( m_pChkBoxWeightedMean ); checkerSizer->AddSpacer(20);
    checkerSizer->Add( m_pChkBoxGroupResult  ); checkerSizer->AddSpacer(20);
    checkerSizer->Add( m_pChkBoxGlobalNames  ); checkerSizer->AddSpacer(20);
    checkerSizer->Add( m_pChkBoxClock        );

    fgs->AddGrowableCol(1, 1);

    // action buttons: keep/cancel
    auto okCancel = CreateOkCancelButtons();

    // add all sizers to vertical sizer
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("General settings for this match "));
    vBox->Add(fgs,          1, wxALL | wxEXPAND      , MY_BORDERSIZE);
    vBox->Add(checkerSizer, 1, wxALL                 , MY_BORDERSIZE);  // was wxLEFT == border flag
    vBox->Add(okCancel,     0, wxALL | wxALIGN_CENTER, MY_BORDERSIZE);
                        //  ^-- make vertically unstretchable
                        //       ^-- no border and centre horizontally

    this->SetSizer(vBox);
//    Centre(); // ???
    RefreshInfo();      // populate the controls

    AUTOTEST_ADD_WINDOW(m_pTxtCtrlDescr      , "Description"    );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlMaxMean    , "MaxMean"        );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlMin        , "ClubMin"        );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlMax        , "ClubMax"        );
    AUTOTEST_ADD_WINDOW(m_pChkBoxNeuberg     , "Neuberg"        );
    AUTOTEST_ADD_WINDOW(m_pChkBoxWeightedMean, "WeightedAverage");
    AUTOTEST_ADD_WINDOW(m_pChkBoxGroupResult , "GroupResult"    );
    AUTOTEST_ADD_WINDOW(m_pChkBoxGlobalNames , "GlobalNames"    );
    AUTOTEST_ADD_WINDOW(m_pChkBoxClock       , "Clock"          );
    m_description = "SetupGame";
}   // SetupGame()

void SetupGame::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    AutoTestAddWindowsNames(a_pFile, m_description);
}   // AutotestRequestMousePositions()

void SetupGame::RefreshInfo()
{
    m_sDescription  = cfg::GetDescription();
    m_iSession      = cfg::GetActiveSession();
    m_iMinClub      = cfg::GetMinClubcount();
    m_iMaxClub      = cfg::GetMaxClubcount();
    m_bNeuberg      = cfg::GetNeuberg();
    m_bWeightedAvg  = cfg::GetWeightedAvg();
    m_bClock        = cfg::GetClock();
    m_sMaxMean      = cfg::MaxMeanToString();
    m_bGroupResult  = cfg::GetGroupResult();
    m_bGlobalNames  = cfg::GetGlobalNameUse();

    if (cfg::GetButler() != m_bButler)
    {   // update/init butler dependent values
        m_bButler = !m_bButler;

        wxString    staticTxt;
        wxString    tipTxt;
        double      max;
        if (m_bButler)
        {
            staticTxt   = _("Maximum imps/game when absent:");
            tipTxt      = _("Maximum imps/game you get in the end/total result when not playing a session");
            max         = 5;
        }
        else
        {
            //xgettext:no-c-format
            staticTxt   = _("Maximum % when absent:");
            tipTxt      = _("Maximum % you get in the end/total result when not playing a session");
            max         = 100;
        }

        m_pStaticTxtMaxMean->SetLabel  (staticTxt);
        m_pStaticTxtMaxMean->SetToolTip(tipTxt   );
        // Allow floating point numbers from 0 to 100/5 with 2 decimal digits only
        m_pTxtCtrlMaxMean->SetMinMax(0, max, 2);
    }

    m_pTxtCtrlDescr         ->SetValue  ( m_sDescription       );
    m_pTxtCtrlMin           ->SetValue  ( U2String(m_iMinClub) );
    m_pTxtCtrlMax           ->SetValue  ( U2String(m_iMaxClub) );
    m_pChkBoxNeuberg        ->SetValue  ( m_bNeuberg           );
    m_pChkBoxWeightedMean   ->SetValue  ( m_bWeightedAvg       );
    m_pChkBoxClock          ->SetValue  ( m_bClock             );
    m_pTxtCtrlMaxMean       ->SetValue  ( m_sMaxMean           );
    m_pChkBoxGroupResult    ->SetValue  ( m_bGroupResult       );
    m_pChkBoxGlobalNames    ->SetValue  ( m_bGlobalNames       );

    HandleMin();  // update the rangechecks for min/max club
    HandleMax();
    Layout();
}   // RefreshInfo()

#include "names.h"
void SetupGame::BackupData()
{
    UINT minClub = wxAtoi(m_pTxtCtrlMin->GetValue());
    UINT maxClub = wxAtoi(m_pTxtCtrlMax->GetValue());

    cfg::ValidateMinMaxClub(minClub, maxClub);
    cfg::MinMaxClubWrite(minClub, maxClub);

    cfg::SetDescription     (m_pTxtCtrlDescr        ->GetValue());
    cfg::SetNeuberg         (m_pChkBoxNeuberg       ->GetValue());
    cfg::SetWeightedAvg     (m_pChkBoxWeightedMean  ->GetValue());
    cfg::SetClock           (m_pChkBoxClock         ->GetValue());
    cfg::SetMaxMean         (m_pTxtCtrlMaxMean      ->GetValue());
    cfg::SetGroupResult     (m_pChkBoxGroupResult   ->GetValue());
    bool bNewGlobalNameUse = m_pChkBoxGlobalNames   ->GetValue();
    if (cfg::GetGlobalNameUse() != bNewGlobalNameUse)
    {
        cfg::SetGlobalNameUse(bNewGlobalNameUse);
        if (names::GetNumberOfGlobalPairs())
        {
            MyMessageBox(_("Changing from/to globalname use, while there are already pairnames!"), _("Warning"));
        }
    }
}   // BackupData()

void SetupGame::PrintPage()
{
    wxString maxAbsent = FMT(_("Max %s absent"), m_bButler ? _("imps") : wxString("%"));
    bool bResult = prn::BeginPrint(_("Gamesettings page:\n")); MY_UNUSED(bResult);
    prn::PrintLine(FMT("%-20s: %s\n"   , _("Description"     ), m_pTxtCtrlDescr                     ->GetValue()));
    prn::PrintLine(FMT("%-20s: %s\n"   , maxAbsent            , m_pTxtCtrlMaxMean                   ->GetValue()));
    prn::PrintLine(FMT("%-20s: %s-%s\n", _("MinMax clubpairs"), m_pTxtCtrlMin->GetValue(), m_pTxtCtrlMax->GetValue()));
    prn::PrintLine(FMT("%-20s: %s\n"   , _("Neuberg"         ), BoolToString(m_pChkBoxNeuberg       ->GetValue())));
    prn::PrintLine(FMT("%-20s: %s\n"   , _("Weighted average"), BoolToString(m_pChkBoxWeightedMean  ->GetValue())));
    prn::PrintLine(FMT("%-20s: %s\n"   , _("Groupresult"     ), BoolToString(m_pChkBoxGroupResult   ->GetValue())));
    prn::PrintLine(FMT("%-20s: %s\n"   , _("Global names"    ), BoolToString(m_pChkBoxGlobalNames   ->GetValue())));
    prn::PrintLine(FMT("%-20s: %s\n"   , _("Clock"           ),BoolToString(m_pChkBoxClock          ->GetValue())));

    prn::PrintLine(cfg::GetCopyrightDateTime());
    prn::EndPrint();
}   // PrintPage()

void SetupGame::OnOk()
{
    BackupData();
    cfg::FLushConfigs();    // update diskfiles
    RefreshInfo();
}   // OnOk()

void SetupGame::OnCancel()
{
    RefreshInfo();  // restore original content
}   // OnCancel()

static int siMinClub=1;
static int siMaxClub=cfg::MAX_PAIRS;

void SetupGame::HandleMin()
{
    AUTOTEST_BUSY("min");
    // update the minimum value for maxclub
    int minClub = wxAtoi(m_pTxtCtrlMin->GetValue());
    siMinClub = minClub;
    m_pTxtCtrlMax->UpdateMin(minClub);
}   // HandleMin()

void SetupGame::HandleMax()
{
    AUTOTEST_BUSY("max");
    //update the maximum value for minclub
    int maxClub = wxAtoi(m_pTxtCtrlMax->GetValue());
    if (maxClub < siMinClub)
    {   // prevent recursion with too low value in control and then change focus
//        maxClub = siMinClub;
//        m_pTxtCtrlMax->ChangeValue(U2String(maxClub));
    }
    siMaxClub = maxClub;
    m_pTxtCtrlMin->UpdateMax(maxClub);
}   // HandleMax()

void SetupGame::OnFocusLostMin(wxFocusEvent& a_event)
{
    //LogMessage("SetupGame::OnFocusLostMin()");
    HandleMin();
    a_event.Skip();
}   // OnFocusLostMin()

void SetupGame::OnFocusLostMax(wxFocusEvent& a_event)
{
    //LogMessage("SetupGame::OnFocusLostMax()");
    HandleMax();
    a_event.Skip();
}   // OnFocusLostMax)()

void SetupGame::OnEnterMin(wxCommandEvent&)
{
    //LogMessage("SetupGame::OnEnterMin()");
    HandleMin();
}   // OnEnterMin()

void SetupGame::OnEnterMax(wxCommandEvent&)
{
    //LogMessage("SetupGame::OnEnterMax()");
    HandleMax();
}   // OnEnterMax()
