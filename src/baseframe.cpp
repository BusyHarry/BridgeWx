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
        wxMessageBox(msg);
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
    hSearchBox->Add(      new wxStaticText(this, wxID_ANY           , _("Text to search:")), TXT_CTRL_SIZER);
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
    BusyBox();
    DoSearch(theString);
}   // OnSearch()

/* virtual */ void Baseframe::DoSearch(wxString&)
{
}

[[maybe_unused]] UINT Baseframe::ValidateNumValUINT( wxTextCtrl* a_pTextCtrl )
{
    UINT newVal = wxAtoi(a_pTextCtrl->GetValue());
    wxIntegerValidator<UINT>* pValidator = reinterpret_cast<wxIntegerValidator<UINT>*> (a_pTextCtrl->GetValidator());
    if (pValidator)
    {
        UINT orgVal = newVal;
        newVal = std::clamp(newVal, pValidator->GetMin(), pValidator->GetMax());
        if (newVal != orgVal)
        {   // value was out of range, correct it
            a_pTextCtrl->SetValue(FMT("%u", newVal));
            a_pTextCtrl->SetInsertionPointEnd();
        }
    }

    return newVal;
}   // ValidateNumValUINT()

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

#include "wx/msw/subwin.h"

bool Baseframe::AutotestAddMousePos(MyTextFile* a_pFile, const wxWindow* a_pWindow, const wxString& a_positionName)
{   // s* vars are Screen positions, w* vars are appwindow-relative positions
    class MyRadioBox: public wxRadioBox
    {   // helper class, only to get at the msw-whnd inside the wxRadiobox
    public:
        HWND GetHwnd_(UINT index) const {return m_radioButtons->Get(index);}
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
    for (const auto& col : a_gridInfo.collumnInfo)
    {
        wxPoint wTL     = {col.x + a_gridInfo.rowLabelSize, col.y + a_gridInfo.colLabelSize};
        wxPoint wBR     = {wTL.x + col.width              , wTL.y + col.height             };
        wxPoint sTL     = a_gridInfo.pGridWindow->ClientToScreen(wTL);
        wxPoint sBR     = a_gridInfo.pGridWindow->ClientToScreen(wBR);
        wxPoint sSize   = sBR - sTL;
        wxString name   = FMT("%s_GridCol%u", a_pageName, colNr++);
        AutotestAddPos(a_pFile, a_gridInfo.pGridWindow, sTL, sSize.x, sSize.y, name);
    }
}   // AutoTestAddGridInfo()

/*****************************************************************/

/***************** validator stuf **********************************/
void MyTextCtrl::OnLostFocus(wxFocusEvent& a_event)
{
    a_event.Skip();     // MUST be done...... else strange things happen
    if (!DoValidate(true))
    {
#if 0
        // this way, we get the focus back via the 'official' way...  
        CallAfter([this] { m_pParent->SetFocus(); } );
#endif
        wxBell();
    }
}   // OnLostFocus()

void MyTextCtrl::OnChar(wxKeyEvent& a_event)
{
    a_event.Skip();   // default: let system handle char
    wxChar chr;
#if wxUSE_UNICODE
    chr = a_event.GetUnicodeKey();
    if ( chr == WXK_NONE )
    {
        // It's a character without any Unicode equivalent at all, e.g. cursor
        // arrow or function key, we never filter those.
        return;
    }
#else
    chr = a_event.GetKeyCode();
    if ( chr > WXK_DELETE )
    {
        // Not a character either.
        return;
    }
#endif

    if ( chr == WXK_RETURN || chr == WXK_NUMPAD_ENTER )
    {
        if (!DoValidate())
        {
            if ( !wxValidator::IsSilent() )
                wxBell();
            a_event.Skip(false);    // Do not skip the event in this case, stop handling it here.
        }

        return;
    }

    if ( chr < WXK_SPACE || chr == WXK_DELETE )
    {
        // Allow ASCII control characters and Delete.
        return;
    }

    if ( a_event.GetModifiers() & ~wxMOD_SHIFT )
    {
        // Keys using modifiers other than Shift don't change the number, so
        // ignore them.
        return;
    }

    wxString tmp;
    int pos;
    GetCurrentValueAndInsertionPoint(tmp, pos);
    bool ok = (chr == '-') ? IsMinusOk(tmp, pos) : IsCharOk(tmp, pos, chr);
    if (ok)
    {
        tmp.insert(pos,1,chr);  // get 'new' string
        if (m_bIsFloat)
        {
            double xx = wxAtof(tmp);    //    perhaps: tmp.ToCDouble(&xx);
            if ( m_bSignOk && xx < m_dMin){ok=false;} // too small
            if (!m_bSignOk && xx > m_dMax){ok=false;} // too large

        }
        else
        {
            long xx = wxAtol(tmp);
            if ( m_bSignOk && xx < m_iMin){ok=false;} // too  small
            if (!m_bSignOk && xx > m_iMax){ok=false;} // too large
        }
    }
    if ( !ok )
    {
        if ( !wxValidator::IsSilent() )
            wxBell();

        // Do not skip the event in this case, stop handling it here.
        a_event.Skip(false);
    }
}   // OnChar()

static void UpdateDouble(MyTextCtrl* pText, const wxString& current, const double& val, int decimalPlaces )
{
    wxString result = FMT("%.*f", decimalPlaces, val);
    //perhaps: wxString res = wxString::FromCDouble(val, decimalPlaces);
    if (result != current)                   // show result in wanted format
        pText->wxTextCtrl::SetValue(result); // NB Bypass parent to prevent recursion
}   // UpdateDouble()

bool MyTextCtrl::DoValidate(bool a_bForceInRange)
{
    if (!m_bValidate) return true;          // no validator, so all ok
    const wxString currentVal = GetValue();
    if (currentVal.IsEmpty()) return true;  // empty, so all ok

    if (m_bIsFloat)
    {
        const double epsilon= 1e-5; // assume that min/max are 'much' larger then this...
        double xx = wxAtof(currentVal);//     perhaps: double yy;currentVal.ToCDouble(&yy);
        if ( xx >= m_dMin-epsilon && xx <= m_dMax+epsilon)
        {   // in range
            UpdateDouble(this, currentVal, xx, m_iDecimalPlaces);
            return true;
        }

        if (a_bForceInRange)
        {
            xx = std::clamp(xx, m_dMin, m_dMax);
            UpdateDouble( this, currentVal, xx, m_iDecimalPlaces);
        }
    }
    else
    {
        long xx = wxAtol(currentVal);
        if (xx >= m_iMin && xx <= m_iMax) return true;
        if (a_bForceInRange)
        {
            xx = std::clamp(xx, m_iMin, m_iMax);
            wxTextCtrl::SetValue(I2String(xx)); // NB Bypass parent to prevent recursion
        }
    }

    return false;
}   // DoValidate()

void MyTextCtrl::GetCurrentValueAndInsertionPoint(wxString& val, int& pos)
{
    val = GetValue();
    pos = GetInsertionPoint();

    long selFrom, selTo;
    GetSelection(&selFrom, &selTo);

    const long selLen = selTo - selFrom;
    if ( selLen )
    {
        // Remove selected text because pressing a key would make it disappear.
        val.erase(selFrom, selLen);

        // And adjust the insertion point to have correct position in the new
        // string.
        if ( pos > selFrom )
        {
            if ( pos >= selTo )
                pos -= selLen;
            else
                pos = selFrom;
        }
    }
}   // GetCurrentValueAndInsertionPoint()

bool MyTextCtrl::IsMinusOk(const wxString& val, int pos) const
{
    // We need to know if we accept negative numbers at all.
    if ( !m_bSignOk )
        return false;

    // Minus is only ever accepted in the beginning of the string.
    if ( pos != 0 )
        return false;

    // And then only if there is no existing minus sign there.
    if ( !val.empty() && val[0] == '-' )
        return false;

    // Notice that entering '-' can make our value invalid, for example if
    // we're limited to -5..15 range and the current value is 12, then the
    // new value would be (invalid) -12. We consider it better to let the
    // user do this because perhaps he is going to press Delete key next to
    // make it -2 and forcing him to delete 1 first would be unnatural.
    //
    // TODO: It would be nice to indicate that the current control contents
    //       is invalid (if it's indeed going to be the case) once
    //       wxValidator supports doing this non-intrusively.
    return true;
}   // IsMinusOk()

bool MyTextCtrl::IsCharOk(const wxString& val, int pos, wxChar chr)
{
    if (!m_bIsFloat) return ( chr >= '0' && chr <= '9' );
    //    const wxChar separator = wxNumberFormatter::GetDecimalSeparator();
#define separator '.'
    if ( chr == separator )
    {
        if ( val.find(separator) != wxString::npos )
        {
            // There is already a decimal separator, can't insert another one.
            return false;
        }

        // Prepending a separator before the minus sign isn't allowed.
        if ( pos == 0 && !val.empty() && val[0] == '-' )
            return false;

        // Otherwise always accept it, adding a decimal separator doesn't
        // change the number value and, in particular, can't make it invalid.
        // OTOH the checks below might not pass because strings like "." or
        // "-." are not valid numbers so parsing them would fail, hence we need
        // to treat it specially here.
        return true;
    }

    // Must be a digit then.
    if ( chr < '0' || chr > '9')
        return false;

    // Check whether the value we'd obtain if we accepted this key passes some
    // basic checks.
    //    const wxString newval(GetValueAfterInsertingChar(val, pos, chr));
    wxString newval(val); newval.insert(pos,1,chr);
#if 0
    LongestValueType value;
    if ( !FromString(newval, &value) )
        return false;
#endif
    // Also check that it doesn't have too many decimal digits.
    const size_t posSep = newval.find(separator);
    if ( posSep != wxString::npos && newval.Len() - posSep - 1 > static_cast<size_t>(m_iDecimalPlaces) )
        return false;

    // Note that we do _not_ check if it's in range here, see the comment in
    // wxIntegerValidatorBase::IsCharOk().
    return true;
}   // IsCharOk()

void MyTextCtrl::SetMinMax(long a_min, long a_max)
{
    if (!m_bValidate)
    {   // TRY to get the event first..
        CallAfter([this] {Bind(wxEVT_CHAR      , &MyTextCtrl::OnChar     , this);});
        CallAfter([this] {Bind(wxEVT_KILL_FOCUS, &MyTextCtrl::OnLostFocus, this);});
    }

    if ( a_min > a_max ) std::swap( a_min, a_max );
    m_iMin      = a_min;
    m_iMax      = a_max;
    m_bIsFloat  = false;
    m_bValidate = true;
    m_bSignOk   = (m_iMin < 0) ;

    (void) DoValidate();
}   // SetMinMax()

void MyTextCtrl::SetMinMax(double a_min, double a_max, int a_decimalPlaces)
{
    if (!m_bValidate)
    {   // TRY to get the event first..
        CallAfter([this] {Bind(wxEVT_CHAR      , &MyTextCtrl::OnChar     , this);});
        CallAfter([this] {Bind(wxEVT_KILL_FOCUS, &MyTextCtrl::OnLostFocus, this);});
    }

    if ( a_min > a_max ) std::swap( a_min, a_max );
    m_bSignOk           = (a_min < 0) ;
    m_dMin              = a_min;
    m_dMax              = a_max;
    m_iDecimalPlaces    = std::min(std::abs(a_decimalPlaces), 5);   // max 5 digits after dp
    m_bIsFloat          = true;
    m_bValidate         = true;

    (void) DoValidate();
}   // SetMinMax()

//**************** end of MyValidator ***************************

//**************** begin of MyTextCtrl ***************************
// MyTextCtrl has its own integer/float validator, acting on charInput, enterKey and killfocus
MyTextCtrl::MyTextCtrl(wxWindow *parent, wxWindowID id, const wxString& value,
                        const wxPoint& pos, const wxSize& size, long style
                      ): wxTextCtrl(parent, id, value, pos, size, style | wxTE_PROCESS_ENTER, wxDefaultValidator)
// we need the wxTE_PROCESS_ENTER else we don't get the \n char at all!
{
    if (!(style & wxTE_PROCESS_ENTER)) // derived class does not want/need enter-event
        this->Bind(wxEVT_TEXT_ENTER,[](wxCommandEvent& ){;});   // dummy handler to consume event (prevent bell)

    // validator stuf
    m_dMin              = 0.0;
    m_dMax              = 0.0;
    m_dResult           = 0.0;
    m_iDecimalPlaces    = 0;
    m_iMin              = 0;
    m_iMax              = 0;
    m_bIsFloat          = false;
    m_bValidate         = false;
    m_bSignOk           = false;
}   // MyTextCtrl()

void MyTextCtrl::SetValue(const wxString& a_value)
{
    static bool bValidating = false;
    wxTextCtrl::SetValue(a_value);
    m_sValOriginal = a_value;
    if (!bValidating)
    {
        bValidating = true;
        DoValidate(true); // could call SetValue() if ok, prevent recursion
    }
    bValidating = false;
}   // SetValue()

//**************** end of MyTextCtrl ***************************

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

wxSize GetSize4EditBox(wxString& msg)   // remark: 'msg' will be adapted
{
    // GetTextExtent()   gives size for text only: {0,0} if empty
    // GetSizeFromText() gives size for text AND margins
    // GetMargins()      gives {0,-1}
    // need a bit extra: 'x', else size is just a little bit too small: textwrap in textctrl
    wxTextCtrl  txtCtrl;
    wxSize      size    = {0,0};
    wxSize      margin  = txtCtrl.GetSizeFromText("xx") - txtCtrl.GetTextExtent("x");

    msg = "\n" + msg + "\n";                    // bit extra space on top and bottem
    wxArrayString array = wxSplit(msg, '\n');
    msg.clear();
    for (const auto& it : array)
    {
        wxString tmp = "     " + it + "     ";  // bit extra space on left and right side
        wxSize   sz  = txtCtrl.GetTextExtent(tmp);  // gives {0,0} if empty...
        size.x = std::max(size.x, sz.x);
        size.y += sz.y;
        msg += tmp + "\n";
    }

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

int MyMessageBox(const wxString& message, const wxString& caption, long style, const wxPoint& position)
{
//    auto pMain = wxApp::GetMainTopWindow();
    auto pMain = GetMainframe();    // this guarantees MY main window
    if (pMain == nullptr)
    {
        return wxMessageBox(message, caption, style, 0, position.x, position.y);
    }

    wxDialog dialog(pMain, wxID_ANY, caption, position);    // no parent --> problems when BusyBox() closes...

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
            size.x          += pTxtCtrl->GetCharWidth()/2;      // add 'half' char at end to get some symmetry!
            wxPoint pos      = position;

            if (pos == wxDefaultPosition)
            {
                pos = wxGetMousePosition();
                pos.x -= size.x/2;    // center at mousetip;
                pos.y -= 2*size.y;    // move to above mousetip;
            }

            m_pBusyFrame = new wxFrame(nullptr, wxID_ANY, ES, pos, size, 0);
            pTxtCtrl    ->Create(m_pBusyFrame , wxID_ANY, message, wxDefaultPosition, size, wxTE_READONLY);
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
