// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/string.h>
#include "utils.h"
#include "choiceMC.h"

#define TEST 0        /* for testing*/
#define IF_TEST if(TEST)

#define MAX_SIZE_COMBO  (20*m_popupCharWidth)
#define MIN_SIZE_COLUMN m_popupCharWidth

class PopupChoiceMC : public wxListView, public wxComboPopup
{
public:
    explicit            PopupChoiceMC       (wxWindow* a_pParent);
    virtual void        Init                () wxOVERRIDE;
    virtual bool        Create              (wxWindow* parent) wxOVERRIDE;
    virtual wxWindow*   GetControl          () wxOVERRIDE;
    virtual void        SetStringValue      (const wxString& sel) wxOVERRIDE;
    virtual wxString    GetStringValue      () const wxOVERRIDE;

    wxString            GetStringSelection  () const;
    bool                SetStringSelection  (const wxString& sel);
    int                 GetSelection        () const;
    bool                SetSelection        (int a_selection);  // return true if new selection within limits
    void                Append              (const wxString& selstr);

protected:
private:
    void OnMouseMove (wxMouseEvent& event);
    void OnMouseClick(wxMouseEvent& event);
    void ItemActivated();           // actions when item activated by mouse or ENTER key

    int         m_value;            // current item index
    int         m_itemHere;         // hot item in popup: mouse-over
    wxWindow*   m_pTargetWindow;    // event target
    wxWindowID  m_mainWinId;        // windows id of main control, needed for choice-event
};  // end PopupChoiceMC

/////////// implementation ////////////////////
PopupChoiceMC::PopupChoiceMC(wxWindow* a_pParent) : wxListView(), wxComboPopup()
{
    m_pTargetWindow = a_pParent;
    m_mainWinId     = a_pParent->GetId();
    Init();
}   // PopupChoiceMC()

void PopupChoiceMC::Init()
{
    m_value         = -1;
    m_itemHere      = -1; // hot item in list
}   // Init()

bool PopupChoiceMC::Create( wxWindow* parent )
{
    m_parent = parent;

    bool bResult = wxListView::Create(parent, wxID_ANY,
        wxPoint(0,0), wxDefaultSize, wxLC_LIST | wxLC_SINGLE_SEL | wxLC_ALIGN_LEFT | wxSIMPLE_BORDER );
    Bind(wxEVT_LEFT_DOWN  , &PopupChoiceMC::OnMouseClick, this);
    Bind(wxEVT_MOTION     , &PopupChoiceMC::OnMouseMove , this);
#if 0 
    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED,[this](wxListEvent & evt)
    {   // show mouse/cursor movements
        evt.Skip();
        // long pos = evt.GetIndex();
        // MyLogDebug(_("ChoiceMC: hover over item %ld"), pos);
    });
#endif
    Bind(wxEVT_COMMAND_LIST_ITEM_ACTIVATED,[this](wxListEvent& evt)
        {   // needed if ENTER key is used for activating selection
            evt.Skip();
            m_itemHere = evt.GetIndex();    // 'simulate' mousemove result
            ItemActivated();
        });
   
    return bResult;
}   // Create()

[[maybe_unused]] wxWindow* PopupChoiceMC::GetControl() { return this; }

[[maybe_unused]] void PopupChoiceMC::SetStringValue( const wxString& sel )
{
    (void)SetStringSelection(sel);
}   // SetStringValue()

bool PopupChoiceMC::SetStringSelection(const wxString& sel)
{
    int index = wxListView::FindItem(-1,sel);
    if ( index >= 0 && index < GetItemCount() )
    {
        wxListView::Select(index);
        m_value = index;
        return true;
    }
    return false;
}   // SetStringSelection()

wxString PopupChoiceMC::GetStringValue() const
{
    if ( m_value >= 0 )
        return wxListView::GetItemText(m_value);
    return wxEmptyString;
}   // GetStringValue()

wxString PopupChoiceMC::GetStringSelection() const {return GetStringValue();}
int PopupChoiceMC::GetSelection() const {return m_value;}

//
// Utilities for item manipulation
//
void PopupChoiceMC::Append( const wxString& selstr )
{
    wxListView::InsertItem(GetItemCount(),selstr);
}   // Append()

bool PopupChoiceMC::SetSelection(int a_selection)  // return true if new selection within limits
{
    if ( a_selection >= 0 && a_selection < GetItemCount())
    {
        m_value = a_selection;
        return true;
    }
    return false;
}   // SetSelection()

void PopupChoiceMC::OnMouseMove(wxMouseEvent& event)
{
    // Move selection to cursor if it is inside the popup
    int resFlags;
    int itemHere = HitTest(event.GetPosition(),resFlags);
    if ( itemHere >= 0 && itemHere != m_itemHere)
    {   // prevent multiple events for same selection
        wxListView::Select(itemHere, true);
        m_itemHere = itemHere;
    }
    event.Skip();
}   // OnMouseMove()

void PopupChoiceMC::OnMouseClick(wxMouseEvent& WXUNUSED(event))
{
    ItemActivated();
}   // OnMouseClick()

void PopupChoiceMC::ItemActivated()
{ // On mouse left/ENTER, set the value, send event and close the popup
    m_value = m_itemHere;   // result from mousemove/ENTER
    wxCommandEvent event(wxEVT_CHOICE, m_mainWinId);
    event.SetInt(m_value);
    event.SetString(GetStringValue());
    m_pTargetWindow->HandleWindowEvent(event);  // now send event to our creator
    Dismiss();                                  // and remove popup
    MyLogDebug(_("ChoiceMC: item %i geactiveerd"), m_value);
}   // ItemActivated()

ChoiceMC::ChoiceMC(wxWindow* a_pParent, const wxString& a_textCtrlTitle) : wxComboCtrl()
{
    SetTextCtrlStyle(wxCB_READONLY/*wxTE_READONLY*/);// remark: wxTE_READONLY ONLY possible BEFORE creation!  (wxCB_READONLY)
    Create(a_pParent, wxID_ANY, a_textCtrlTitle);    //HOW TO SIZE AT RUN-TIME? -> SetMinSize({300,-1});
    // setting textstyle AFTER Create() results in txtctrl fully occupying its parent window: no white border
    SetTextCtrlStyle(wxTE_LEFT);
    UseAltPopupWindow();    // Make sure we use popup that allows focusing the listview.
    SelectNone();           // nothing selected in wxTextEntry

    m_pPopup = new PopupChoiceMC(this);
    m_pPopup->InsertColumn(0, "", wxLIST_FORMAT_LEFT, -1);

    SetPopupControl(m_pPopup);
    m_bAutoSize = true; //size of choice-entries determine column-size and editctrl-size

    m_pTxtctrl = GetTextCtrl();
    ResetTextctrlSize();
    m_pTxtctrl->Clear();   // remove label-text from entry...
    m_pPopup->SetFont(m_pTxtctrl->GetFont());       // at least in windows, the wxListView does NOT use its parent fontsize! But some(?) global defined one.
    m_pTxtctrl->Bind(wxEVT_LEFT_DOWN , [this](wxMouseEvent&)
            {   // on mouseclick in editctrl, show popup or do nothing...
                if (m_timerPopupKillFocus.IsRunning())
                {   // apparently(!) popup receives killfocus before we get the mouse
                    IF_TEST MyLogDebug(_("ChoiceMC->mouseclick(timer running)"));
                    m_timerPopupKillFocus.Stop();
                }
                else
                {   // assume first click in editctrl
                    AutoTestBusyMC(true);   // set busy for MC, so AHK can wait for popup
                    IF_TEST MyLogDebug(_("ChoiceMC->mouseclick(show)"));
                    //Disable(); //can't click anymore
                    Popup();
                }
            });
    m_pTxtctrl->Bind(wxEVT_SET_FOCUS,[this](wxFocusEvent&){});      // eat all focus events: don't select anything, don't show cursor
    m_pTxtctrl->SetBackgroundColour({220,220,220});                 // now it looks like wxChoice
    //GetButton()->SetBackgroundColour({220,220,220});  // GetButton() ALWAYS nullptr
    //SetBackgroundColour({220,220,220});               // doesn't work for button to set background of mainwindow
    m_currentColumnWidth    = 0;
    m_nrOfColumns           = 0;
    m_popupCharWidth        = m_pPopup->GetCharWidth();
    m_popupCharHeight       = m_pPopup->GetCharHeight();
    m_popupMaxWidth         = 27*m_popupCharWidth;      // arbitrary, but we allow atleast 2 columns before scrolling...

    m_pPopup->SetColumnWidth(0, m_popupCharWidth*6);    // default 6 chars wide
    SetNumberOfRows(MC_DEFAULT_NR_OF_ROWS);
    m_textMinSize = GetMargins().x + GetButtonSize().GetX();    // minimum size of combobox if no text in it
    m_pPopup->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent&){m_timerPopupKillFocus.StartOnce(35);});
    
    IF_TEST MyLogDebug(_("ChoiceMC() comboMinSize=%i, charWidth=%i"), m_textMinSize, m_popupCharWidth);
    ShowSizes(0);
}   // ChoiceMC()

ChoiceMC::~ChoiceMC() {}

bool ChoiceMC::SetSelection(int a_selection)
{   // setting size of combocontrol only gets active AFTER/DURING the next idle loop.
    // So we need a 'CallAfter' to be sure the text will fit...
    bool bResult = m_pPopup->SetSelection(a_selection);
    CallAfter([this, bResult]{SetValueByUser(bResult ? GetStringSelection() : ""); });    // show in editctrl
    CallAfter([this]{ShowSizes(3);});
    return bResult;
}   // SetSelection()

bool ChoiceMC::SetStringSelection(const wxString& a_sel)
{   // setting size of combocontrol only gets active AFTER/DURING the next idle loop.
    // So we need a 'CallAfter' to be sure the text will fit...
    bool bResult = m_pPopup->SetStringSelection(a_sel);
    SetValueByUser(bResult ? a_sel : "");   // show in editctrl
    CallAfter([this]{ShowSizes(4);});
    return bResult;
}   // SetStringSelection()

void ChoiceMC::ShowSizes(UINT a_id)
{
#if TEST == 1
    wxSize  s1      = GetSize();
    wxRect  txtRect = GetTextRect();
    wxSize  s2      = m_pTxtctrl->GetSize();
//    int     wi      = m_pTxtctrl->GetMaxWidth();

    MyLogDebug(_("ShowSizes(%i,%u): GetSize=%i, TxtCtlGetSize=%i, GetTextRect=%i")
                   , GetId(), a_id, s1.x      , s2.x            , txtRect.GetWidth());
#else
    (void)a_id;
#endif
}   // ShowSizes()

void ChoiceMC::CheckAutoSize(const wxString& a_choice)
{   // should be called BEFORE an item is added/inserted
    if (m_bAutoSize)
    {
        ShowSizes(1);
        int width = m_pPopup->GetTextExtent(a_choice).GetWidth() + MIN_SIZE_COLUMN;
        if (width > m_currentColumnWidth)
        {   // update size of txtctrl in combo and columnwidth in popup
            m_currentColumnWidth = width;
            m_pPopup->SetColumnWidth(0, m_currentColumnWidth);
            int txtSize = std::min(m_currentColumnWidth + m_textMinSize, MAX_SIZE_COMBO);
            m_pTxtctrl->SetSize({width  ,-1});  // NOW text WILL fit....
            SetMinSize         ({txtSize,-1});  // will only take effect AFTER next idle....
            IF_TEST MyLogDebug(_("CheckAutoSize(%i): SetMinSize: txt+offset=%i, minsize=%i"), GetId(), width, txtSize);
        }
        m_nrOfColumns = (m_pPopup->GetItemCount() + m_numberOfRows /* - 1*/) / m_numberOfRows;
        int actualWidth = m_nrOfColumns*m_currentColumnWidth;
        if ( actualWidth > m_popupWidth )
            SetPopupWidth(actualWidth);
        ShowSizes(2); CallAfter([this]{ShowSizes(20);});
    }
}   // CheckAutoSize()

void ChoiceMC::Append(const wxString& a_newChoice)
{
    CheckAutoSize(a_newChoice);
    m_pPopup->Append(a_newChoice);
}   // Append()

wxString ChoiceMC::GetStringSelection() const
{
    return m_pPopup->GetStringSelection();
}   // GetStringSelection()

int ChoiceMC::GetSelection() const
{
    return m_pPopup->GetSelection();
}   // GetSelection()

[[maybe_unused]] bool ChoiceMC::AnimateShow( const wxRect& rect, int WXUNUSED(flags) )
{
    // Customize(wxCC_NO_TEXT_AUTO_SELECT);   // DOES prevent selectAll, but still cursor present
    wxWindow* win = GetPopupWindow();
//    wxRect& pRect = (wxRect&)rect; pRect.x=100; pRect.y=100; // repos window....
    win->SetSize(rect);
    win->Raise();  // This is needed
    win->ShowWithEffect(wxSHOW_EFFECT_SLIDE_TO_BOTTOM /*wxSHOW_EFFECT_BLEND*/);
    CallAfter([](){AutoTestBusyMC(false);});    // tell AHK popup is there
    return true;
}   // AnimateShow()

void ChoiceMC::Clear()
{
    m_pPopup->DeleteAllItems();
    m_pPopup->Init();
    m_popupWidth        = 0;
    m_currentColumnWidth= 0;
    m_nrOfColumns       = 0;
    ResetTextctrlSize(); // Clear() SHOULD reset, but screen-layout could change.....
}   // Clear()

UINT ChoiceMC::GetCount()
{
    return m_pPopup->GetItemCount();
}   // GetCount()

void ChoiceMC::Init(UINT a_count, UINT a_selection, UINT a_offset)
{
    Clear();
    for (unsigned int count = 1; count <= a_count; ++count)
    {
        Append(FMT("%u", count + a_offset));
    }

    SetSelection(a_selection);
}   // Init()

void ChoiceMC::Set(const wxArrayString& a_items)
{
    Clear();
    for (const auto& it : a_items)
        Append(it);
}   // Set()

void ChoiceMC::Init(const wxArrayString& a_choices, UINT a_selection)
{
    Set(a_choices); //void Set(const wxArrayString& items)
    SetSelection(a_selection);
}   // Init()

void ChoiceMC::Init(const wxArrayString& a_choices, const wxString& a_selection)
{
    Set(a_choices);
    SetStringSelection(a_selection);
}   // Init()

long ChoiceMC::InsertItem(long a_index, const wxString& a_choice)
{
    CheckAutoSize(a_choice);
    return m_pPopup->InsertItem(a_index, a_choice);
}   // InsertItem()

void ChoiceMC::SetNumberOfRows(UINT a_numberOfRows)
{ 
//    m_pPopup->SetScrollbar();
    int borderSizeY     = wxSystemSettings::GetMetric( wxSYS_BORDER_Y , m_pPopup );
    int scrollBarHeight = wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y, m_pPopup );

    SetPopupMaxHeight((1+m_popupCharHeight)*a_numberOfRows + scrollBarHeight + 2*borderSizeY);
    m_numberOfRows = a_numberOfRows;
}   // SetNumberOfRows()

[[maybe_unused]] void ChoiceMC::SetColumnWidthInChars(UINT a_chars)
{
    m_pPopup->SetColumnWidth(0, m_popupCharWidth*a_chars);
}   // ChoiceMC()

[[maybe_unused]] void ChoiceMC::SetAutoColumnWidth(bool a_bAuto)
{
    m_bAutoSize = a_bAuto;
}   // SetAutoColumnWidth()

void ChoiceMC::SetPopupWidth(int a_size)
{

    if (m_nrOfColumns > 2)  // allow at least 2 columns
    {
        a_size = std::min(a_size, m_popupMaxWidth);
        if (a_size == m_popupMaxWidth)  // max size for popup: larger values will give scrollbar
            a_size -= m_popupMaxWidth % m_currentColumnWidth;   // need multiple of column-width!
    }
    if (a_size != m_popupWidth)
    {
        m_popupWidth = a_size;
        SetPopupMinWidth(m_popupWidth);
        IF_TEST MyLogDebug(_("ChoiceMC::SetPopupWidth() size=%i, cols=%u, breedte=%i"),a_size,m_nrOfColumns, m_currentColumnWidth);
    }
}   // SetPopupWidth()

[[maybe_unused]] void ChoiceMC::SetMaxPopupWidth(int a_maxWidth)
{
    m_popupMaxWidth = a_maxWidth;
}   // SetMaxPopupWidth()

void ChoiceMC::ResetTextctrlSize()
{
    wxSize width(5 * GetCharWidth(), -1);
    SetMinSize(width);
    SetMaxSize(width);
    SetPopupWidth(m_textMinSize);
}   // ResetTextctrlSize()
