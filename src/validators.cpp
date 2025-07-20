// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/grid.h>

#include "validators.h"

///***************** begin of implementation for MyGridCellEditorWithValidator ********************
MyGridCellEditorWithValidator::MyGridCellEditorWithValidator(double a_min, double a_max, int a_precision) : wxGridCellTextEditor()
{
    m_validator.SetMinMax(a_min, a_max, a_precision);
}   // MyGridCellEditorWithValidator()

bool MyGridCellEditorWithValidator::IsAcceptedKey(wxKeyEvent& a_event)
{   // called by grid-editor to check if a key is allowed to start the editor
    return m_validator.IsValidStartKey(a_event.GetUnicodeKey());
}   // MyGridCellEditorWithValidator::IsAcceptedKey()

void MyGridCellEditorWithValidator::BeginEdit(int a_row, int a_col, wxGrid* a_pGrid)
{   // only now we can get the attached wxTextCtrl!
    m_validator.SetTextCtrl(Text());
    wxGridCellTextEditor::BeginEdit(a_row, a_col, a_pGrid);
}   // MyGridCellEditorWithValidator::BeginEdit()

///***************** end of implementation for MyGridCellEditorWithValidator ********************

///******************* begin of implementation for MyTextCtrlWithValidator *******************************
MyTextCtrlWithValidator::MyTextCtrlWithValidator(wxWindow *a_parent, wxWindowID a_id, const wxString& a_value,
    const wxPoint& a_pos, const wxSize& a_size, long a_style)
    : wxTextCtrl(a_parent, a_id, a_value, a_pos, a_size, a_style | wxTE_PROCESS_ENTER)
// we need the wxTE_PROCESS_ENTER else we don't get the '\n' char at all!
{
    if ( !(a_style & wxTE_PROCESS_ENTER) )                      // derived class does not want/need enter-event
        this->Bind(wxEVT_TEXT_ENTER,[](wxCommandEvent& ){;});   // dummy handler to consume event (prevent bell)
    m_validator.SetTextCtrl(this);
}   // MyTextCtrlWithValidator()
///******************* end of implementation for MyTextCtrlWithValidator *******************************

///**************** begin of implementation for MyValidator  ***************************
MyValidator::~MyValidator()
{
    if ( m_bBindDone )
    {
        bool b1 = m_pTextCtrl->Unbind(wxEVT_CHAR      , &MyValidator::OnChar     , this);
        bool b2 = m_pTextCtrl->Unbind(wxEVT_KILL_FOCUS, &MyValidator::OnLostFocus, this);
        MY_UNUSED(b1);
        MY_UNUSED(b2);
        m_bBindDone = false;
    }
}   // MyValidator::~MyValidator()

bool MyValidator::IsValidStartKey(wxChar a_key)
{   // first key does not arrive via OnChar() but via IsAcceptedKey() when we have a grid-editor
    bool bValid = true;
    do
    {
        if ( !m_bValidate                                 ) break;
        if ( a_key == '-' && m_bSignOk                    ) break;
        if ( m_bIsFloat && (a_key == '.' || a_key == ',') ) break;
        if ( wxIsdigit(a_key) && (a_key - '0') <= m_dMax  ) break;
        if ( IsExtraChar(a_key)                           ) break;
        if ( a_key == ' '                                 ) break;
        bValid = false;
    } while ( 0 );
    if ( !bValid )
        RingBell();
    return bValid;
}   //  MyValidator::IsValidStartKey()

bool MyValidator::IsExtraChar(wxChar a_chr)
{
    return wxString::npos != m_sStartChars.Find(a_chr);
}   // MyValidator::IsExtraChar()

void MyValidator::OnLostFocus(wxFocusEvent& a_event)
{
    a_event.Skip();     // MUST be done...... else strange things happen
    if ( !DoValidate(true) )
        RingBell();
}   // MyValidator::OnLostFocus()

void MyValidator::OnChar(wxKeyEvent& a_event)
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
        if ( !DoValidate(true) )
        {
            RingBell();
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
    wxString    currentVal;
    int         pos;
    bool        bOk = true;
    do
    {   // just a dummy loop: easy 'jump' to end of it
        GetCurrentValueAndInsertionPoint(currentVal, pos);

        if ( chr == '-' && IsMinusOk(currentVal, pos) ) break;
        if ( IsExtraChar(chr) )
        {
            if ( pos == 0 && currentVal.IsEmpty() ) break;  // only allow special char at position 0
            bOk = false;
            break;
        }

        if ( pos && IsExtraChar(currentVal[0]) )
        {   // prevent extra entries if we already have an extra char!
            bOk = false;
            break;
        }

        bool bIsComma(false);
        if ( chr == ',' && m_bIsFloat ) { chr = '.'; bIsComma = true; }
        bOk = IsCharOk(currentVal, pos, chr);
        if ( !bOk ) break;

        currentVal.insert(pos, 1, chr);  // get 'new' string
        if ( bIsComma )
        {   // can't get EmuLateKeyPress() working :( --> wxKeyEvent keyEvent(wxEVT_KEY_DOWN); keyEvent.m_keyCode = keyEvent.m_rawCode = keyEvent.m_uniChar = '.'; m_pTextCtrl->EmulateKeyPress(keyEvent);
//            m_pTextCtrl->CallAfter([this, currentVal, pos] {m_pTextCtrl->ChangeValue(currentVal); m_pTextCtrl->SetInsertionPoint(pos + 1); });
            m_pTextCtrl->ChangeValue(currentVal); m_pTextCtrl->SetInsertionPoint(pos + 1);
            a_event.Skip(false);
            return;
        }
        double xx;
        currentVal.ToCDouble(&xx); // dp is ALWAYS '.' here
        if ( m_bSignOk   && xx < m_dMin ) { bOk = false; }  // too small: smaller then negative minimum
        if ( m_dMax >= 0 && xx > m_dMax ) { bOk = false; }  // too large: larger  then positive maximum
    } while ( 0 );

    if ( !bOk )
    {
        RingBell();
        // Do not skip the event in this case, stop handling it here.
        a_event.Skip(false);
    }
}   // MyValidator::OnChar()

void MyValidator::UpdateDouble(const wxString& a_current, const double& a_val)
{
    wxString result = wxString::FromCDouble(a_val, m_iPrecision); // we ALWAYS want a '.' as decimalpoint
    if ( result != a_current )              // show result in wanted format
        m_pTextCtrl->SetValue(result);      // NB Bypass parent to prevent recursion
}   // MyValidator::UpdateDouble()

bool MyValidator::DoValidate(bool a_bForceInRange)
{
    if ( !m_bValidate || nullptr == m_pTextCtrl ) return true;  // no validator, so all ok
    const wxString currentVal = m_pTextCtrl->GetValue();
    if ( currentVal.IsEmpty() ) return true;                    // empty, so all ok
    if ( m_sStartChars.Find(currentVal[0]) != wxString::npos )  // special start char
        return true;
    if ( m_bIsFloat )
    {
        const double epsilon= 1e-5; // assume that min/max are 'much' larger then this...
        double xx;
        currentVal.ToCDouble(&xx);   // c-locale: dp == '.'
        if ( xx >= m_dMin-epsilon && xx <= m_dMax+epsilon )
        {   // in range
            UpdateDouble(currentVal, xx);
            return true;
        }

        if ( a_bForceInRange )
        {
            xx = std::clamp(xx, m_dMin, m_dMax);
            UpdateDouble( currentVal, xx );
        }
    }
    else
    {
        double xx = (double)wxAtol(currentVal);
        if ( xx >= m_dMin && xx <= m_dMax ) return true;
        if ( a_bForceInRange )
        {
            xx = std::clamp(xx, m_dMin, m_dMax);
            m_pTextCtrl->SetValue(L2String((long)xx)); // NB Bypass parent to prevent recursion
        }
    }

    return false;
}   // MyValidator::DoValidate()

void MyValidator::GetCurrentValueAndInsertionPoint(wxString& a_val, int& a_pos)
{
    a_val = m_pTextCtrl->GetValue();
    a_pos = m_pTextCtrl->GetInsertionPoint();

    long selFrom, selTo;
    m_pTextCtrl->GetSelection(&selFrom, &selTo);

    const long selLen = selTo - selFrom;
    if ( selLen )
    {
        // Remove selected text because pressing a key would make it disappear.
        a_val.erase(selFrom, selLen);

        // And adjust the insertion point to have correct position in the new
        // string.
        if ( a_pos > selFrom )
        {
            if ( a_pos >= selTo )
                a_pos -= selLen;
            else
                a_pos = selFrom;
        }
    }
}   // MyValidator::GetCurrentValueAndInsertionPoint()

bool MyValidator::IsMinusOk(const wxString& a_val, int a_pos) const
{
    // We need to know if we accept negative numbers at all.
    if ( !m_bSignOk )
        return false;

    // Minus is only ever accepted in the beginning of the string, but ignore a single space!
    if ( a_pos == 1 && a_val[0] == ' ' && wxString::npos == a_val.Find('-') ) 
        return true;
    if ( a_pos )
        return false;

    // And then only if there is no existing minus sign there.
    if ( wxString::npos != a_val.Find('-') )
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
}   // MyValidator::IsMinusOk()

bool MyValidator::IsCharOk(const wxString& a_val, int a_pos, wxChar a_chr)
{
    if ( !m_bIsFloat ) return ( a_chr >= '0' && a_chr <= '9' );
    const wxChar separator =  '.' ;     // NB, the OnChar() transforms a ',' to a '.'
    if ( a_chr == separator )
    {
        if ( a_val.find(separator) != wxString::npos )
        {
            // There is already a decimal separator, can't insert another one.
            return false;
        }

        // Prepending a separator before the minus sign isn't allowed.
        if ( a_pos == 0 && !a_val.IsEmpty() && a_val[0] == '-' )
            return false;

        // Otherwise always accept it, adding a decimal separator doesn't
        // change the number value and, in particular, can't make it invalid.
        // OTOH the checks below might not pass because strings like "." or
        // "-." are not valid numbers so parsing them would fail, hence we need
        // to treat it specially here.
        return true;
    }

    // Must be a digit then.
    if ( a_chr < '0' || a_chr > '9' )
        return false;

    // Check whether the value we'd obtain if we accepted this key passes some
    // basic checks.
    //    const wxString newval(GetValueAfterInsertingChar(val, pos, chr));
    wxString newval(a_val); newval.insert(a_pos, 1, a_chr);
    // Also check that it doesn't have too many decimal digits.
    const size_t posSep = newval.find(separator);
    if ( posSep != wxString::npos && newval.Len() - posSep - 1 > static_cast<size_t>(m_iPrecision) )
        return false;

    // Note that we do _not_ check if it's in range here, see the comment in
    // wxIntegerValidatorBase::IsCharOk().
    return true;
}   // MyValidator::IsCharOk()

void MyValidator::SetMinMax(double a_min, double a_max, int a_precision)
{
    // Bind() for chars and lostfocus is setup when we get a textCtrl!
    if ( a_min > a_max ) std::swap( a_min, a_max );
    m_bSignOk           = (a_min < 0) ;
    m_dMin              = a_min;
    m_dMax              = a_max;
    if ( a_precision != -1 )
    {
        m_iPrecision = std::min(std::abs(a_precision), 5);   // max 5 digits after dp
        m_bIsFloat = true;
    }
    m_bValidate         = true;     // we want the validator
    SetTextCtrl(m_pTextCtrl);       // do Bind() if not done yet
}   // MyValidator::SetMinMax()

void MyValidator::UpdateMin(double a_min)
{
    m_dMin = a_min;
    DoValidate(true);
}   // MyValidator::UpdateMin()

void MyValidator::UpdateMax(double a_max)
{
    m_dMax = a_max;
    DoValidate(true);
}   // MyValidator::UpdateMax()

void MyValidator::UpdatePrecision(int a_precision)
{
    m_iPrecision = a_precision;
    DoValidate(true);
}   // MyValidator::UpdatePrecision()


void MyValidator::SetTextCtrl(wxTextCtrl* a_pTextCtrl)
{
    if ( m_bBindDone ) return;  // we can have only one textCtrl!
    m_pTextCtrl = a_pTextCtrl;
    if ( m_bValidate && nullptr != m_pTextCtrl )
    {   // TRY to get the event first..
        m_pTextCtrl->CallAfter([this] {m_pTextCtrl->Bind(wxEVT_CHAR      , &MyValidator::OnChar     , this);});
        m_pTextCtrl->CallAfter([this] {m_pTextCtrl->Bind(wxEVT_KILL_FOCUS, &MyValidator::OnLostFocus, this);});
        m_bBindDone = true;
        // DON't force a validate: let user change/correct value if wrong!
    }
}   // MyValidator::SetTextCtrl()

//**************** end of implementation for MyValidator  ***************************
