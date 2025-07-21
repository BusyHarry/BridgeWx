// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/dataview.h>
#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/button.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valnum.h>
#include "wx/radiobox.h"
#include "wx/checkbox.h"
#include "wx/combobox.h"
#include "wx/regex.h"
#include <wx/filepicker.h>
#include <wx/generic/stattextg.h>
#if wxMAJOR_VERSION == 3 && wxMINOR_VERSION < 3
#include <wx/msw/subwin.h>
#endif
#include <wx/radiobut.h>

#include "cfg.h"
#include "baseframe.h"
#include "mygrid.h"

static const wxWindow* GetTopLevelWindow(const wxWindow* a_pWindow)
{
    while (a_pWindow)
    {
        wxWindow* parent = a_pWindow->GetParent();
        if (parent == nullptr)
            break;
        a_pWindow = parent;
    }

    return a_pWindow;
}   // GetTopLevelWindow()

#define STATIC_SIZE 20
static wxPoint spPosition={1,1};
wxSize GetStaticRectSize()
{   // position/size for invisible static controls
    return {STATIC_SIZE, STATIC_SIZE};
}   // GetStaticRectSize()

wxPoint GetStaticRectPosition()
{   // position/size for invisible static controls
    spPosition.x += STATIC_SIZE;
    if (spPosition.x > 600)
    {
        spPosition.x  = 0; 
        spPosition.y += STATIC_SIZE;
    }
    return spPosition;
}   // GetStaticRectPosition()

Baseframe::Baseframe(wxWindow* a_pParent, UINT a_pageId)
    : wxPanel(a_pParent)
    , m_pParent             { a_pParent }
    , m_pConfig             { this      }
    , m_pTxtCtrlSearchBox   { 0         }
    , m_iCurrentConfigHash  { -1        }
    , m_pageId              { a_pageId  }
{
    m_bIsScriptTesting = cfg::IsScriptTesting();
}   // Baseframe()

Baseframe::~Baseframe()
{
}   // ~Baseframe()

const wxString& Baseframe::GetDescription() const
{
    return m_description;
}   // GetDescription()

bool Baseframe::OnCellChanging(const CellInfo& a_cellInfo)
{   //default implementation, derived class should implement this
    if (a_cellInfo.oldData != a_cellInfo.newData)
    {
        wxString msg;
        msg.Printf(_("Baseframe:: row %d, column %d changes from <%s> to <%s>")
                            , a_cellInfo.row
                            , a_cellInfo.column
                            , a_cellInfo.oldData
                            , a_cellInfo.newData
                );
        MyMessageBox(msg);
    }

    return CELL_CHANGE_OK;   // ok, by default we accept the change
}   // OnCellChanging()

wxBoxSizer* Baseframe::CreateOkCancelButtons()
{
    // action buttons: keep/cancel for derived classes to use
    auto ok = new wxButton(this, wxID_ANY, Unique(_("Save")));
    ok->SetToolTip(_("save changed data"));
    auto cancel = new wxButton(this, wxID_ANY, Unique(_("Undo")));
    cancel->SetToolTip(_("undo changes"));

    // sizer for the default implementation of the Ok/Cancel buttons
    wxBoxSizer* pButtonOkCancelSizer = new wxBoxSizer(wxHORIZONTAL);
    pButtonOkCancelSizer->Add(    ok, 0 /* unstretchable */ , wxRIGHT, 20); // only right border so buttons don't clash
    pButtonOkCancelSizer->Add(cancel, 0                                  ); // caller handles all other borders/alignments

    ok    ->Bind(wxEVT_BUTTON, &Baseframe::OnOk    , this);//, ok    ->GetId() );
    cancel->Bind(wxEVT_BUTTON, &Baseframe::OnCancel, this);//, cancel->GetId() );
    AUTOTEST_ADD_WINDOW(ok    , "Ok"    );
    AUTOTEST_ADD_WINDOW(cancel, "Cancel");
    return pButtonOkCancelSizer;    // add this one to your own sizer(s)
}   // CreateOkCancelButtons()

void Baseframe::OnOk(wxCommandEvent&)
{
    AUTOTEST_BUSY("ok");
    MyLogDebug("OnOk()");
    BusyBox();
    OnOk();
}   // OnOk()

void Baseframe::OnOk()
{
    // derived class should implement this
}   // OnOk()

void Baseframe::OnCancel(wxCommandEvent&)
{
    AUTOTEST_BUSY("cancel");
    MyLogDebug(_("OnCancel()"));
    BusyBox();
    OnCancel();
}   // OnCancel()

void Baseframe::OnCancel()
{
    // derived class should implement this
}   // OnCancel()

void Baseframe::PrintPage()
{
    AUTOTEST_BUSY("printPage");
    LogMessage("Baseframe::PrintPage()");   // derived class should implement this...
}   // PrintPage()

wxString Baseframe::Unique(const wxString& a_name)
{   // autohotkey cannot distinguish between buttons with same name (like "ok")
    return m_bIsScriptTesting ? FMT("%s%u", a_name, m_pageId - ID_MENU_SETUP_FIRST) : a_name;
}   // Unique()

#define TXT_BORDER_SIZE 10  /* border between static text and its control*/
// TXT_CTRL_SIZER: how to add Sstatic txt in hSizer: fixed size, right border of TXT_BORDER_SIZE and txt centered vertically
#define TXT_CTRL_SIZER   1, wxRIGHT | wxALIGN_CENTER_VERTICAL, TXT_BORDER_SIZE /* sizer info for static txt in combo's*/

wxBoxSizer* Baseframe::CreateSearchBox()
{//| wxALIGN_CENTER_VERTICAL
    auto hSearchBox     = new wxBoxSizer  (wxHORIZONTAL);
    m_pTxtCtrlSearchBox = new wxTextCtrl  (this, ID_BASEFRAME_SEARCH, Unique("SearchEntry"  ), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_pTxtCtrlSearchBox->Clear();   // remove label-text from entry, but keep window-title for autotest...
    auto pButton        = new wxButton    (this, ID_BASEFRAME_SEARCH, Unique(_("Search"     )));
    hSearchBox->Add(      new wxGenericStaticText(this, wxID_ANY    , _("Text to search:")), TXT_CTRL_SIZER);
    hSearchBox->Add(m_pTxtCtrlSearchBox , 0, wxRIGHT, 20);
    hSearchBox->Add(pButton             , 0);

    Bind(wxEVT_BUTTON,      &Baseframe::OnSearch,  this, ID_BASEFRAME_SEARCH);
    Bind(wxEVT_TEXT_ENTER,  &Baseframe::OnSearch,  this, ID_BASEFRAME_SEARCH);
    AUTOTEST_ADD_WINDOW(m_pTxtCtrlSearchBox,"TextSearchEntry" );
    AUTOTEST_ADD_WINDOW(pButton            ,"TextSearchButton");
    return hSearchBox;
}   // CreateSearchBox()

void Baseframe::OnSearch(wxCommandEvent& )
{
    AUTOTEST_BUSY("search");
    wxString theString = m_pTxtCtrlSearchBox->GetValue();
    //BusyBox();    // V3.3.0+ --> too much flicker
    DoSearch(theString);
}   // OnSearch()

/* virtual */ void Baseframe::DoSearch(wxString&)
{
}

wxRadioBox* Baseframe::CreateRadioBox(const wxString& a_title, const wxArrayString& a_choices, pEventHandler a_pHandler, const wxString& a_autoName)
{
    auto* pRadio = new wxRadioBox(this, wxID_ANY, a_title, wxDefaultPosition, wxDefaultSize, a_choices, 1);
    pRadio->Bind(wxEVT_RADIOBOX, a_pHandler, this);
    AUTOTEST_ADD_WINDOW(pRadio, a_autoName);
    return pRadio;
}   // CreateRadioBox()

/*virtual*/ void Baseframe::AutotestRequestMousePositions(MyTextFile* /*pFile*/)    // request to add usefull mousepositions for autotesting
{
    // default do nothing
}

static void AutotestAddPos(MyTextFile* a_pFile, const wxWindow* a_pWindow, wxPoint a_sTopLeft, int a_width, int a_height, const wxString& a_varName)
{
    wxPoint     sAppWin    = GetTopLevelWindow(a_pWindow)->GetScreenPosition();
    wxPoint     sWinCenter = a_sTopLeft + wxPoint(a_width/2, a_height/2);
    wxPoint     wWinCenter = sWinCenter - sAppWin;

    a_pFile->AddLine(FMT("%-30s := [%4i,%4i] ; sCenter: [%4i,%4i], sWinTL: [%4i,%4i], sWinBR: [%4i,%4i]"
        , a_varName
        , wWinCenter.x            , wWinCenter.y
        , sWinCenter.x            , sWinCenter.y
        , a_sTopLeft.x            , a_sTopLeft.y
        , a_sTopLeft.x + a_width-1, a_sTopLeft.y + a_height-1
    ));
}   // AutotestAddPos()

void AutotestAddLabel(MyTextFile* a_pFile, const wxString& a_label, const wxString& a_labelValue)
{
    if (a_labelValue.IsEmpty()) return;
    wxString labelName = a_label + "_L";
    a_pFile->AddLine(FMT("%-30s := \"%s\"", labelName, a_labelValue));
}   // AutotestAddLabel()

static void AutotestAddWindowPos(MyTextFile* a_pFile, const wxWindow* a_pWindow, const wxString& a_positionName)
{
    wxRect  winRect     = a_pWindow->GetScreenRect();
    wxPoint sWinTL      = a_pWindow->GetScreenPosition();  // top-left of window
    AutotestAddPos(a_pFile, a_pWindow, sWinTL, winRect.width, winRect.height, a_positionName);
}   //  AutotestAddWindowPos()

bool Baseframe::AutotestAddMousePos(MyTextFile* a_pFile, const wxWindow* a_pWindow, const wxString& a_positionName)
{   // s* vars are Screen positions, w* vars are appwindow-relative positions
    class MyRadioBox: public wxRadioBox
    {   // helper class, only to get at the msw-whnd inside the wxRadiobox
    public:
#if wxMAJOR_VERSION >= 3 && wxMINOR_VERSION >= 3
        HWND GetHwnd_(UINT index) const {return m_radioButtons[index]->GetHWND();};
#else
        HWND GetHwnd_(UINT index) const {return m_radioButtons->Get(index);}
#endif
    };
    assert(a_pWindow);  // stupid C6011 warning: possible use of nullptr
    auto pRadio = static_cast<const MyRadioBox*>(dynamic_cast<const wxRadioBox*> (a_pWindow));
    if (pRadio)
    {
        UINT rCount = pRadio->GetCount();
        for (UINT count = 0; count < rCount; ++count)
        {
            RECT        rect;  // left,top,right,bottom
            HWND        hwnd    = pRadio->GetHwnd_(count);
            bool        bResult = ::GetWindowRect(hwnd, &rect);  (void)bResult;// window in screen coördinates
            wxString    nameI   = FMT("%s%u", a_positionName, count);
            AutotestAddPos  (a_pFile, a_pWindow, wxPoint(rect.left,rect.top), rect.right-rect.left, rect.bottom-rect.top,nameI);
            AutotestAddLabel(a_pFile, nameI, pRadio->GetString(count));
        }
    }
    else
    {   // 'normal' window:  text or button or ...
        AutotestAddWindowPos(a_pFile, a_pWindow, a_positionName);   // mousepos info
        // next lines will try to get a 'title' of this window so we can access it by name i.s.o. mousepositions 
        auto pChoiceMC = dynamic_cast<const MyChoiceMC*> (a_pWindow);
        if (pChoiceMC) AutotestAddLabel(a_pFile, a_positionName, pChoiceMC->GetTextCtrl()->GetLabel());
        else
        {
            auto pMyChoice = dynamic_cast<const MyChoice*> (a_pWindow);
            if (pMyChoice) AutotestAddLabel(a_pFile, a_positionName, pMyChoice->GetLabel());
            else 
            {
                auto pMywxComboBox = dynamic_cast<const MywxComboBox*> (a_pWindow);
                if (pMywxComboBox)
                    AutotestAddLabel(a_pFile, a_positionName, pMywxComboBox->GetLabel());
                else
                {
                    auto pDirPicker = dynamic_cast<const wxDirPickerCtrl*> (a_pWindow);
                    if (pDirPicker)
                        AutotestAddLabel(a_pFile, a_positionName, pDirPicker->GetName());
                    else // 'normal' window: wxButton/wxTextCtrl/wxStaticText/wxCheckBox....
                        AutotestAddLabel(a_pFile, a_positionName, a_pWindow->GetLabel());
                }
            }
        }
    }

    return true;
}   // AutotestAddMousePos()

void Baseframe::AutoTestAddWindowsNames(MyTextFile* a_pFile, const wxString& a_pageName)
{
    for (const auto& window : m_winNames)
    {
        AutotestAddMousePos(a_pFile, window.pWindow, a_pageName + "_" + window.mousePosName);
    }
}  // AutoTestAddWindowsNames()

void Baseframe::AutoTestAddGridInfo(MyTextFile* a_pFile, const wxString& a_pageName, const MyGrid::GridInfo& a_gridInfo)
{   // add info about all collumns of first row
    AutotestAddMousePos(a_pFile, a_gridInfo.pGridWindow, a_pageName + "_Grid");
    UINT colNr = 0;
    UINT colNrAuto = 0;
    auto pGrid = (MyGrid*)a_gridInfo.pGridWindow;
    for (const auto& col : a_gridInfo.collumnInfo)
    {
        wxPoint wTL     = {col.x + a_gridInfo.rowLabelSize, col.y + a_gridInfo.colLabelSize};
        wxPoint wBR     = {wTL.x + col.width              , wTL.y + col.height             };
        wxPoint sTL     = a_gridInfo.pGridWindow->ClientToScreen(wTL);
        wxPoint sBR     = a_gridInfo.pGridWindow->ClientToScreen(wBR);
        wxPoint sSize   = sBR - sTL;
        wxString name = FMT("%s_GridCol%u", a_pageName, colNr);
        AutotestAddPos(a_pFile, a_gridInfo.pGridWindow, sTL, sSize.x, sSize.y, name);
        if ( (a_gridInfo.ColumnLabelAutotest.size() > colNr) && pGrid->IsColShown(colNr) )
        {   // columnlabel defenitions
            ++colNrAuto;    // one based column values for non-hidden columns
            wxString columnId = FMT("%s_Column_%s", a_pageName, a_gridInfo.ColumnLabelAutotest[colNr]);
            columnId.Replace(" ", "_");
            columnId.Replace("-", "_");
            columnId.Replace("%", "procent");
            a_pFile->AddLine(FMT("%-30s := %u", columnId, colNrAuto));
        }
        ++colNr;
    }
}   // AutoTestAddGridInfo()

/*****************************************************************/


/*********************************************************/
MyChoice::MyChoice(wxWindow* a_parent, const wxString& a_staticText, const wxString& a_tooltip, const wxString& a_ahkLabel)
    : MywxChoice(a_parent, wxID_ANY, a_ahkLabel)
{
    SetToolTip(a_tooltip);
    m_pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    if (!a_staticText.IsEmpty())
    {
        wxStaticText* pTitle= new wxStaticText(a_parent, wxID_ANY, a_staticText);
        m_pBoxSizer->Add(pTitle, TXT_CTRL_SIZER);
    }
    m_pBoxSizer->Add(this, 0);
}   // MyChoice()

void MyChoice::Init(UINT a_count, UINT a_selection, UINT a_offset)
{
    Clear();
    for (UINT count = 1; count <= a_count; ++count)
    {
        Append(U2String(count+a_offset));
    }
    
    SetSelection(a_selection);
}   // Init()

void MyChoice::Init(const wxArrayString& a_choices, UINT a_selection)
{
    Set(a_choices);
    SetSelection(a_selection);
}   // Init()

void MyChoice::Init(const wxArrayString& a_choices, const wxString& a_selection)
{
    Set(a_choices);
    SetStringSelection(a_selection);
}   // Init()

MyChoiceMC::MyChoiceMC(wxWindow* a_parent, const wxString& a_staticText, const wxString& a_tooltip, const wxString& a_windowTitle)
    :ChoiceMC(a_parent, a_windowTitle)
{
    SetToolTip(a_tooltip);
    m_pBoxSizer = new wxBoxSizer(wxHORIZONTAL);

    if (!a_staticText.IsEmpty())
    {
        wxStaticText* pTitle= new wxStaticText(a_parent, wxID_ANY, a_staticText);
        m_pBoxSizer->Add(pTitle, TXT_CTRL_SIZER);
    }
    m_pBoxSizer->Add(this  , 0);
}   // MyChoiceMC()

MyChoiceMC::~MyChoiceMC()
{
    ;
}   // ~MyChoiceMC()

wxSize GetSize4EditBox(wxString& msg)    // remark: 'msg' will be adapted
{
    // GetTextExtent()   gives size for text only: {0,0} if empty
    // GetSizeFromText() gives size for text AND margins
    // GetMargins()      gives {0,-1}
    // need a bit extra: 'x', else size is just a little bit too small: textwrap in textctrl
    wxTextCtrl  txtCtrl(GetMainframe(), wxID_ANY);  // txtCtrl uses fontsize from mainframe, this is already scaled to wanted value
    wxSize      size    = {0,0};
    wxSize      margin  = txtCtrl.GetSizeFromText("xx") - txtCtrl.GetTextExtent("x");

    msg = "\n" + msg + "\n";                    // bit extra space on top and bottem
    wxArrayString array = wxSplit(msg, '\n');
    msg.clear();
    for (const auto& it : array)
    {
        wxString tmp = "     " + it;  // bit extra space on left and right side
        wxSize   sz  = txtCtrl.GetTextExtent(tmp +  "     ");  // gives {0,0} if empty...
        size.x = std::max(size.x, sz.x);
        size.y += sz.y;
        tmp.Trim(true);
        msg += tmp + "\n";
    }
    msg.RemoveLast();   // last '\n' would gives one line too many in textbox
    return size+margin;
}   // GetSize4EditBox()

#define ADD_BUTTON_TO_MSGBOX(style,flag,label,pSizer,dialog)\
if (style & flag)                                           \
{                                                           \
    auto pButton = new wxButton(&dialog, wxID_ANY, label);  \
    pButton->SetFont(pButton->GetFont().Scale(0.8f));       \
    pButton->Bind(wxEVT_BUTTON, [&dialog](const wxCommandEvent&){dialog.EndModal(flag);}); \
    pSizer->Add(pButton, 0, wxRIGHT, 5);                    \
    if (style & flag##_DEFAULT) pButton->SetFocus();        \
}
#include <wx/app.h>

float GetScale()
{   // calculate a scale from the fontsize of the mainframe and a default txtCtrl
    auto pMain = GetMainframe();    // this guarantees MY main window
    if (nullptr == pMain) return 1.0;
    wxTextCtrl  txtCtrl;
    auto        mainSize    = pMain-> GetFont().GetFractionalPointSize();
    auto        defaultSize = txtCtrl.GetFont().GetFractionalPointSize();
    return (float)(mainSize/defaultSize);
}   // GetScale()

int MyMessageBox(const wxString& message, const wxString& caption, long style, const wxPoint& position)
{
    auto pMain = GetMainframe();    // this guarantees MY main window
    if (pMain == nullptr)
    {
        return wxMessageBox(message, caption, style, 0, position.x, position.y);
    }

    float scale = GetScale();
    wxDialog dialog(pMain, wxID_ANY, caption, position);    // no parent --> problems when BusyBox() closes...
    dialog.SetFont(dialog.GetFont().Scale(scale));          // error? dialog does NOT take fontsize of parent...
    auto        pEditSizer  = new wxBoxSizer( wxHORIZONTAL );
    wxString    msg         = message;
    wxSize      size        = GetSize4EditBox(msg);
    wxTextCtrl* txtCtrl     = new wxTextCtrl(&dialog, wxID_ANY, msg, {0,0}, size, wxBORDER_NONE|wxTE_MULTILINE | wxTE_READONLY);

    txtCtrl->SetScrollbar(wxVERTICAL,0,0,0);
    pEditSizer->Add(txtCtrl, 1);    // '1': possibly grow horizontally

    auto hButtonSizer = new wxBoxSizer( wxHORIZONTAL );             // add wanted buttons
    ADD_BUTTON_TO_MSGBOX(style, wxOK    ,_("Ok"      ), hButtonSizer, dialog)
    ADD_BUTTON_TO_MSGBOX(style, wxYES   ,_("Yes"     ), hButtonSizer, dialog)
    ADD_BUTTON_TO_MSGBOX(style, wxNO    ,_("No"      ), hButtonSizer, dialog)
    ADD_BUTTON_TO_MSGBOX(style, wxCANCEL,_("Cancel"  ), hButtonSizer, dialog)

    wxBoxSizer *vBox = new wxBoxSizer( wxVERTICAL );
    vBox->Add(pEditSizer  , 1, wxEXPAND);
    vBox->Add(hButtonSizer, 0, wxALIGN_RIGHT | wxALL, 8);
    dialog.SetSizerAndFit(vBox);

    if (position == wxDefaultPosition)
    {   // center on main-window
        //if (style & wxCENTER ){;} else {;}	// not implemeted (yet)
        wxPoint     mainPos = pMain->GetPosition();
        wxSize      mainSize= pMain->GetSize();
        wxSize      mySize  = dialog.GetSize();
        wxPoint     myPos;
        myPos.x = mainPos.x + (mainSize.x - mySize.x)/2;
        myPos.y = mainPos.y + (mainSize.y - mySize.y)/2;
        dialog.SetPosition(myPos);
    }

    int result = dialog.ShowModal();
    return result;
}   // MyMessageBox()

// display a message during some time and selfdestruct afterwards (like a tooltip)
void BusyBox(const wxString& message, int milisecondsShow, const wxPoint& position)
{
    class bb 
    {
    public:
        ~bb(){}
        bb(const wxString& message, int milisecondsShow, const wxPoint& position)
        {
            auto    pTxtCtrl = new wxTextCtrl();
            wxSize  size     = pTxtCtrl->GetSizeFromText(message);
            float   scale    = GetScale();
            size            *= scale;
            size.x          += pTxtCtrl->GetCharWidth()/2;      // add 'half' char at end to get some symmetry!
            wxPoint pos      = position;

            if (pos == wxDefaultPosition)
            {
                pos = wxGetMousePosition();
                pos.x -= size.x/2;    // center at mousetip;
                pos.y -= 2*size.y;    // move to above mousetip;
            }

            m_pBusyFrame = new wxFrame(nullptr, wxID_ANY, ES, pos, size, 0);
            pTxtCtrl    ->Create(m_pBusyFrame , wxID_ANY, ES, wxDefaultPosition, size, wxTE_READONLY);
            pTxtCtrl->SetFont(pTxtCtrl->GetFont().Scale(scale));    // use same fontscaling as my mainframe.
            pTxtCtrl->AppendText(message);
            m_pBusyFrame->Show();                   // show popup msg and start timer
            wxApp::GetMainTopWindow()->SetFocus();  // focus back to mainframe: prevent crash on dtor

            m_busyTimer.Bind(wxEVT_TIMER,[this](const wxTimerEvent& )
                {
                    m_raiseTimer.Stop();
                    m_pBusyFrame->Close();
                    m_pBusyFrame->Destroy();
                    m_pBusyFrame = nullptr;
                    delete this;
                });
            m_busyTimer.StartOnce(milisecondsShow);

            m_raiseTimer.Bind(wxEVT_TIMER,
                [this](const wxTimerEvent&)
                {   // Simulate topwindow, without being one
                    if (m_pBusyFrame) m_pBusyFrame->Raise();   // bring to top, if hidden...
                });
            m_raiseTimer.Start(100);
        }
    private:
        bb(){m_pBusyFrame = nullptr;}
        bb(const bb &)      {m_pBusyFrame = nullptr;}  // complaint of cppcheck
        void operator=(bb&) {m_pBusyFrame = nullptr;}  // complaint of cppcheck
        wxFrame*        m_pBusyFrame;
        wxTimer         m_busyTimer;
        wxTimer         m_raiseTimer;
        wxBusyCursor    m_busyCursor;
    };

    if (!cfg::IsScriptTesting())
        new bb(message, milisecondsShow, position);
}   // BusyBox()

/****************** MyTextFile **********************/
MyTextFile::MyTextFile()
{
    m_error     = -1;
    m_access    = READ;
    m_bOk       = false;
    m_textType  = wxTextFileType_Dos;
}   // MyTextFile()

MyTextFile::MyTextFile(const wxString& a_filename, AccessType a_access, wxTextFileType a_textType)
{
    MyCreate(a_filename, a_access, a_textType);
}   // MyTextFile()

void MyTextFile::MyCreate(const wxString& a_filename, AccessType a_access, wxTextFileType a_textType)
{
    m_access    = a_access;
    m_textType  = a_textType;
    m_fileName  = a_filename;
    m_error     = 0;

    Close();    // if opened previous, it will reinitialize/reset the class
    m_bOk = Open(m_fileName, wxCSConv(wxFONTENCODING_CP437));
    if (m_access != READ)
    {
        if (m_bOk)
        {
            if ( m_access == WRITE) Clear(); // empty existing file
        }
        else
        {
            m_bOk = Create();    // create file if it does not yet exists
            if (m_bOk)
                m_bOk = Open(m_fileName);
        }
    }
}   // MyCreate()

bool MyTextFile::IsOk()
{
    return m_bOk;
}   // IsOk()

MyTextFile::~MyTextFile()
{
    Flush();
}   // ~MyTextFile()

void MyTextFile::Flush()
{
    if (IsOpened() && ( m_access != READ) )
        m_bOk = Write(m_textType, wxCSConv(wxFONTENCODING_CP437));  // write changes to disk
}   // Flush()

/****************** end MyTextFile **********************/
#include <wx/uiaction.h>
AHKHelper::AHKHelper(wxWindow* a_pParent, wxWindow* a_pTarget, const  wxString& a_label)
{
    m_pTarget = a_pTarget;
    m_staticAhk.Hide();
    m_staticAhk.Create(a_pParent, wxID_ANY, a_label, GetStaticRectPosition(), GetStaticRectSize());
    m_staticAhk.Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& )
                        {
                            // SetFocus()/SetFocusFromKbd(): this SEEMS to work, but 'sometimes' OS thinks focus is somewhere else...
                            wxUIActionSimulator sim;        // this ALWAYS works!
                            wxRect sr = m_pTarget->GetScreenRect();
                            sim.MouseMove (sr.x+5,sr.y+5);
                            sim.MouseClick();               // 'physical' click...
                        }
                    );
}   // AHKHelper()

AHKHelper::~AHKHelper(){}

wxString AHKHelper::GetStaticLabel() const
{
    return m_staticAhk.GetLabel();
}   // GetStaticLabel()

MywxChoice::MywxChoice(wxWindow* a_pParent, wxWindowID a_id, const wxString& a_label)
: wxChoice (a_pParent, a_id)
, AHKHelper(a_pParent, this, a_label)
{
}   // MywxChoice()

MywxChoice::~MywxChoice()
{
}

wxString MywxChoice::GetLabel() const
{
    return AHKHelper::GetStaticLabel();
}   // GetLabel()

MywxComboBox::MywxComboBox(wxWindow* a_pParent, wxWindowID a_id,
    const wxString& a_ahkLabel, const wxPoint& pos, const wxSize& size, int count, const wxString choices[], long style)
     : wxComboBox(a_pParent, a_id, ES, pos, size, count, choices, style)
     , AHKHelper(a_pParent, this, a_ahkLabel)
{
}   // MywxComboBox()

MywxComboBox::~MywxComboBox()
{
}

wxString MywxComboBox::GetLabel() const
{
    return AHKHelper::GetStaticLabel();
}   // GetLabel()

#include <wx/choicdlg.h>
int MyGetSingleChoiceIndex(const wxString& a_message, const wxString& a_caption, const wxArrayString& a_names, wxWindow* a_pParent, int a_selection)
{   // replacement for wxGetSingleChoiceIndex() to get a fontsize equal to its parent
    wxSingleChoiceDialog dialog;
    if (a_pParent) dialog.SetFont(a_pParent->GetFont());    // get parent sized font in the dialog
    dialog.Create(0, a_message, a_caption, a_names);
    dialog.SetSelection(a_selection);
    return dialog.ShowModal() == wxID_OK ? dialog.GetSelection() : -1;
}   // MyGetSingleChoiceIndex()
