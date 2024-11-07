﻿// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/filepicker.h>
#include <wx/valnum.h>
#include <wx/combobox.h>

#include "cfg.h"
#include "setupNewMatch.h"
#include "fileio.h"
#include "printer.h"

SetupNewMatch::SetupNewMatch(wxWindow* a_pParent, UINT a_pageId) : Baseframe(a_pParent, a_pageId)
{
    wxStaticText* txtDir = new wxStaticText(this, wxID_ANY, _("Folder for matchdata:   "));
    long style = 0;

    style |= wxDIRP_USE_TEXTCTRL;   // manual enter directory here, or use the 'browse' button
                                    //      style |= wxDIRP_DIR_MUST_EXIST;
                                    //      style |= wxDIRP_CHANGE_DIR;
                                    //      style |= wxDIRP_SMALL;

    m_pDirPicker = new wxDirPickerCtrl(this, wxID_ANY,
        ES, _("Choose folder to save matchdata."),
        wxDefaultPosition, wxDefaultSize,
        style, wxDefaultValidator, "MatchFolder");  // name is for AHK2
    m_pDirPicker->Bind(wxEVT_DIRPICKER_CHANGED, [this](wxFileDirPickerEvent&){UpdateSelection();});

    txtDir->SetToolTip(_("Choose here the folder where you want to store the matchdata."));
// the name of the match
    auto txtMatch = new wxStaticText(this, wxID_ANY, _("Matchname:"));
    m_pComboBoxMatch = new MywxComboBox(this, wxID_ANY, "MatchName", wxDefaultPosition, {15 * GetCharWidth(),-1}, 0, NULL, wxCB_SORT);
    m_pComboBoxMatch->SetToolTip(_("Name of the (new) match"));
// the session of the match
    auto txtSession = new wxStaticText(this, wxID_ANY, _("Session:"));
    m_pTxtCtrlSession = new MyTextCtrl(this, wxID_ANY, "Session", MY_SIZE_TXTCTRL_NUM(2));
    m_pTxtCtrlSession->SetToolTip(_("The number of the session, 0 if the match exist of one session"));
    m_pTxtCtrlSession->SetMinMax(0, cfg::MAX_SESSIONS);
// database type
    auto txtDbType = new wxStaticText(this, wxID_ANY, _("Database type:"));
    m_pTxtCtrlDbType = new MyTextCtrl(this, wxID_ANY, "DbaseType",MY_SIZE_TXTCTRL_NUM(4), wxTE_READONLY);

    // now add all of the above in sets of 2 to a flexgridsizer
    wxFlexGridSizer* fgs = new wxFlexGridSizer(4 /*rows*/, 2 /*columns*/, 9 /*v-gap*/, 25 /*h-gap*/);
    fgs->Add(txtDir    , 0, wxALIGN_CENTER_VERTICAL);   fgs->Add(m_pDirPicker,      1, wxEXPAND);
    fgs->Add(txtMatch  , 0, wxALIGN_CENTER_VERTICAL);   fgs->Add(m_pComboBoxMatch,  1          );
    fgs->Add(txtSession, 0, wxALIGN_CENTER_VERTICAL);   fgs->Add(m_pTxtCtrlSession, 0          );
    fgs->Add(txtDbType , 0, wxALIGN_CENTER_VERTICAL);   fgs->Add(m_pTxtCtrlDbType , 0          );

    fgs->AddGrowableCol(1, 1);  // column 1 should grow horizontally if possible

// action buttons: keep/undo
    auto okCancel = CreateOkCancelButtons();

// add all sizers to vertical sizer
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Setup for the active match:"));
    vBox->AddSpacer( 30 );
    vBox->Add(fgs,         1, wxALL | wxEXPAND         , MY_BORDERSIZE);
    vBox->Add(okCancel,    0, wxALL | wxALIGN_CENTER   , MY_BORDERSIZE);

    SetSizer(vBox);
    RefreshInfo();      // populate the controls

    wxButton *pButton = dynamic_cast<wxButton*>(m_pDirPicker->GetPickerCtrl());
    if (pButton)
    {
        //pButton->SetLabel("Custom browse string");
        AUTOTEST_ADD_WINDOW(pButton      , "BrowsButton");
    }
    AUTOTEST_ADD_WINDOW(m_pDirPicker     , "Folder"    );
    AUTOTEST_ADD_WINDOW(m_pComboBoxMatch , "Name"      );
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlSession, "Session"   );
    m_description = "Match";
}   // SetupNewMatch()

SetupNewMatch:: ~SetupNewMatch(){}

void SetupNewMatch::AutotestRequestMousePositions(MyTextFile* a_pFile)    // request to add usefull mousepositions for autotesting
{
    AutoTestAddWindowsNames(a_pFile, m_description);
}   // AutotestRequestMousePositions()

void SetupNewMatch::UpdateSelection()
{
    AUTOTEST_BUSY("matchPath");
    wxString match      = cfg::GetActiveMatch();
    wxString path       = m_pDirPicker->GetTextCtrlValue();
    wxString extension  = io::DatabaseTypeGet() == io::DB_ORG ? ".ini" : ".db";
    wxArrayString choices;
    wxString file = wxFindFirstFile(path + "/*" + extension);
    while ( !file.empty() )
    {
        file = file.AfterLast('/').AfterLast('\\'); // remove path
        if (file != cfg::GetBareMainIni())
        {
            file.Replace(extension, ES);            // remove extension
            choices.push_back(file);                // only filenamepart needed
        }
        file = wxFindNextFile();
    }

    m_pComboBoxMatch->Set(choices);
    m_pComboBoxMatch->SetStringSelection(match);
}   // UpdateSelection()

#include <wx/display.h>
void SetupNewMatch::RefreshInfo()
{
    m_pDirPicker     ->SetDirName( cfg::GetActiveMatchPath()          );
    m_pTxtCtrlSession->SetValue  ( U2String( cfg::GetActiveSession()) );
    cfg::FileExtension extension = io::DatabaseTypeGet() == io::DB_ORG ? cfg::EXT_MAIN_INI : cfg::EXT_DATABASE;
    wxFileName file = cfg:: ConstructFilename("", extension);
    m_pTxtCtrlDbType ->SetValue  (file.GetName());
    UpdateSelection();
    Layout();
}   // RefreshInfo()

void SetupNewMatch::BackupData()
{
    wxArrayString argv;      // 0=reserved, 1=match, 2=path, 3=session
    argv.push_back(ES);
    argv.push_back("-f" + m_pDirPicker     ->GetTextCtrlValue());
    argv.push_back("-w" + m_pComboBoxMatch ->GetValue());
    argv.push_back("-z" + m_pTxtCtrlSession->GetValue());

    cfg::HandleCommandline( argv, false );
}   // BackupData()

void SetupNewMatch::OnOk()
{
    BackupData();
    cfg::FLushConfigs();    // update diskfiles
    RefreshInfo();
}   // OnOk()

void SetupNewMatch::OnCancel()
{
    RefreshInfo();  // restore original content
    LogMessage(_("SetupNewMatch::Cancel()"));
}   // OnCancel()

void SetupNewMatch::PrintPage()
{
    bool bResult = prn::BeginPrint(_("New match page:\n")); MY_UNUSED(bResult);
    wxString info;
    info = FMT(_("Matchfolder  : %s\n"
                 "Match        : %s\n"
                 "Session      : %s\n"
                 "Databasetype : %s\n"),
                m_pDirPicker     ->GetTextCtrlValue(),
                m_pComboBoxMatch ->GetValue(),
                m_pTxtCtrlSession->GetValue(),
                m_pTxtCtrlDbType ->GetValue()
              );

    prn::PrintLine(info);
    prn::PrintLine(cfg::GetCopyrightDateTime());
    prn::EndPrint();
}   // PrintPage()
