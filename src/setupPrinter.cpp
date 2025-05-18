// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/combobox.h>
#include <wx/valnum.h>
#include <wx/filepicker.h>

#include "cfg.h"
#include "printer.h"
#include "SetupPrinter.h"

#define CHOICE_PRINTER "ChoicePrinter"

SetupPrinter::SetupPrinter(wxWindow* a_pParent, UINT a_pageId) : Baseframe(a_pParent, a_pageId)
{
    //printer setup
    m_choiceBoxPrn          = new MY_CHOICE (this, "", _("Printer choice, 'list' means 'print to file' with name list"), CHOICE_PRINTER);
    m_pTxtCtrlLinesPP       = new MyTextCtrl(this, wxID_ANY, "LinesPP", MY_SIZE_TXTCTRL_NUM(3));
    m_choiceBoxPrn->Bind(wxEVT_CHOICE, [this](auto&){AUTOTEST_BUSY("selectPrinter");});

    m_pTxtCtrlLinesPP->SetMinMax(1, 100);   // lines per page: 1-100
    // now add all of the above in sets of 2 to a flexgridsizer
    wxFlexGridSizer* fgs = new wxFlexGridSizer(2 /*rows*/, 2 /*columns*/, 9 /*v-gap*/, 25 /*h-gap*/);
    fgs->Add( new wxStaticText(this, wxID_ANY, _("Printer:"       )), 0, wxALIGN_CENTER_VERTICAL );   fgs->MyAdd( m_choiceBoxPrn    );
    fgs->Add( new wxStaticText(this, wxID_ANY, _("Lines per page:")), 0, wxALIGN_CENTER_VERTICAL );   fgs->  Add( m_pTxtCtrlLinesPP );

    m_pChkBoxFF = new wxCheckBox(this, wxID_ANY, _("FormFeed after each print command"));
    m_pChkBoxFF->SetToolTip(_("FF after each single printcommand i.s.o when the page is full"));

    m_pChkBoxRemote = new wxCheckBox(this, wxID_ANY, _("Network printers"));
    m_pChkBoxRemote->SetToolTip(_("Also show printers in the network\n\nBE CAREFULL: SYSTEEM CAN HANG!"));

    // put all of above below each other in a sizer: all occupying just the space they need
    wxBoxSizer* vBoxSettings= new wxBoxSizer(wxVERTICAL);
    vBoxSettings->Add(fgs            ,0 , wxALL | wxALIGN_LEFT  , MY_BORDERSIZE );
    vBoxSettings->Add(m_pChkBoxFF    ,0 , wxALL                 , MY_BORDERSIZE );
    vBoxSettings->Add(m_pChkBoxRemote,0 , wxALL                 , MY_BORDERSIZE );

    // button for font-selection
    m_pBtnFont = new wxButton(this, wxID_ANY, _("Font choice"));
    m_pBtnFont->SetToolTip(_("choice of another/larger/smaller font for the printer output"));
    m_pBtnFont->Bind(wxEVT_BUTTON, [](wxCommandEvent&){prn::SelectFont();});

    // action buttons: keep/cancel
    m_mOkCancel = CreateOkCancelButtons();

    auto hButtonSizer= new wxBoxSizer(wxHORIZONTAL);        // add all buttons into one sizer
    hButtonSizer->Add(m_pBtnFont , 0, wxALL, MY_BORDERSIZE);
    hButtonSizer->Add(m_mOkCancel, 0, wxALL, MY_BORDERSIZE);

    // add all sizers to vertical sizer
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this,_("Printer settings"));
    vBox->AddSpacer( 30 );
    vBox->Add(vBoxSettings, 1                                       );  // take as much space as there is, so ok/cancel is at the bottom
    vBox->Add(hButtonSizer, 0, wxALL | wxALIGN_CENTER, MY_BORDERSIZE);
    //vBox->Add(okCancel  , 0, wxALL | wxALIGN_CENTER, MY_BORDERSIZE);

    SetSizer(vBox);
//    Centre(); // ???
    RefreshInfo();      // populate the controls

    AUTOTEST_ADD_WINDOW(m_pTxtCtrlLinesPP, "LinesPP"      );
    AUTOTEST_ADD_WINDOW(m_pChkBoxFF      , "FormFeed"     );
    AUTOTEST_ADD_WINDOW(m_pChkBoxRemote  , "Remote"       );
    AUTOTEST_ADD_WINDOW(m_pBtnFont       , "Font"         );
    AUTOTEST_ADD_WINDOW(m_choiceBoxPrn   , CHOICE_PRINTER );
    m_description = "Printer";
}   // SetupPrinter()

SetupPrinter::~SetupPrinter(){}   // ~SetupPrinter()

void SetupPrinter::RefreshInfo()
{
    m_pTxtCtrlLinesPP->SetValue( U2String( cfg::GetLinesPerPage()) );
    m_pChkBoxFF      ->SetValue( cfg::GetFF()                      );
    m_pChkBoxRemote  ->SetValue( cfg::GetNetworkPrinting()         );

    wxArrayString printers;
    prn::EnumeratePrinters(printers);
    // available OS printers
    printers.Insert(cfg::GetFilePrinterName(), 0 );    // first choice always 'print to file'

    m_choiceBoxPrn->Set(printers);
    m_choiceBoxPrn->SetStringSelection(cfg::GetPrinterName());
    Layout();
}   // RefreshInfo()

void SetupPrinter::BackupData()
{
    cfg::SetLinesPerPage    (wxAtoi(m_pTxtCtrlLinesPP->GetValue())          );
    cfg::SetFF              (m_pChkBoxFF             ->GetValue()           ); 
    cfg::SetNetworkPrinting (m_pChkBoxRemote         ->GetValue()           );
    cfg::SetPrinterName     (m_choiceBoxPrn          ->GetStringSelection() );
}   // BackupData()

void SetupPrinter::OnOk()
{
    LogMessage(_("SetupPrinter::OnOk()"));
    BackupData();
    cfg::FLushConfigs();    // update diskfiles
    RefreshInfo();
//    Layout();
}   // OnOk()

void SetupPrinter::OnCancel()
{
    LogMessage(_("SetupPrinter::OnCancel()"));
    RefreshInfo();  // restore original content
}   // OnCancel()

void SetupPrinter::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    AutoTestAddWindowsNames(a_pFile, m_description);
}   // AutotestRequestMousePositions()

void SetupPrinter::PrintPage()
{
    bool bResult = prn::BeginPrint(_("Printersettings page:\n")); MY_UNUSED(bResult);
    prn::PrintLine(FMT("%-16s: %s\n", _("Printer"       ), m_choiceBoxPrn               ->GetStringSelection()));
    prn::PrintLine(FMT("%-16s: %s\n", _("Lines/page"    ), m_pTxtCtrlLinesPP            ->GetValue()));
    prn::PrintLine(FMT("%-16s: %s\n", _("Formfeed"      ), BoolToString(m_pChkBoxFF     ->GetValue())));
    prn::PrintLine(FMT("%-16s: %s\n", _("Networkprinter"), BoolToString(m_pChkBoxRemote ->GetValue())));

    prn::PrintLine(cfg::GetCopyrightDateTime());
    prn::EndPrint();
}   // PrintPage()
