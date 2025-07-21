// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _VALIDATORS_H_
#define _VALIDATORS_H_

#include <wx/grid.h>
#include <wx/valnum.h>
#include <wx/textctrl.h>

#include "utils.h"

class MyValidator
{   // none/int/float validator for a wxTextCtrl
    // empty results are ok. Decimal point: both ',' and '.' are accepted, display is only '.'
public:
    MyValidator(){}
    MyValidator(wxTextCtrl* pTextCtrl) {SetTextCtrl(pTextCtrl);}
    MyValidator(wxTextCtrl* pTextCtrl, double min, double max, int precision = -1) {SetTextCtrl(pTextCtrl); SetMinMax(min, max, precision);}
    MyValidator(double min, double max, int precision = -1) {SetMinMax(min, max, precision);}
    virtual ~MyValidator(); // Unbind() events

    bool IsValidStartKey    (wxChar key);               // is 'key' allowed to start edit (grid-editor only)
    void SetExtraStartChars (const wxString& startChars) {m_sStartChars = startChars;} // additional valid start chars
    void SetMinMax          (double min, double max, int precision = -1);   // set values and (try to) Bind() events
    void SetTextCtrl        (wxTextCtrl* pTextCtrl);    // if 'pTextCtrl', then Bind() events if wanted
    void UpdateMax          (double max);               // update maximum and force value inrange
    void UpdateMin          (double min);               // update minimum and force value inrange
    void UpdatePrecision    (int precision);            // update the precision (why would you use this???)
protected:
    void OnChar             (wxKeyEvent&     event);                        // event handler to check for valid chars
    void OnLostFocus        (wxFocusEvent&   event);                        // event handler to force value in-range
private:
    bool DoValidate         (bool bForceInRange = false);                   // return true if accepted, possibly forcing result to be in-range
    void GetCurrentValueAndInsertionPoint(wxString& val, int& pos);         // retrieve current textvalue and insertion point from the txtCtrl
    bool IsCharOk           (const wxString& val, int pos, wxChar chr);     // is 'chr' acceptable at this position
    bool IsExtraChar        (wxChar chr);                                   // true, if 'chr' is in 'm_sStartChars'
    bool IsMinusOk          (const wxString& val, int pos) const;           // is '-' acceptable at this position
    void UpdateDouble       (const wxString& current, const double& val);   // get double in wanted format

    double      m_dMin              = 0.0;          // min/max values are stored as double
    double      m_dMax              = 0.0;
    int         m_iPrecision        = 0;            // nr of allowed digits after the decimal point
    bool        m_bIsFloat          = false;        // float or integer validator
    bool        m_bValidate         = false;        // true, if min/max is set
    bool        m_bSignOk           = false;        // true if '-' is an allowed char
    bool        m_bBindDone         = false;        // true if we have binded the wanted events, so validation can work
    wxTextCtrl* m_pTextCtrl         = nullptr;      // the textctrl we are working with
    wxString    m_sStartChars;                      // extra char(s) allowed to start edit: no chars further allowed
};  // class MyValidator

//// A GridEditor with validator of type MyValidator
class MyGridCellEditorWithValidator : public wxGridCellTextEditor
{
public:
    explicit MyGridCellEditorWithValidator(double min, double max, int precision = -1);
             MyGridCellEditorWithValidator() {}
            ~MyGridCellEditorWithValidator() {}
    virtual void    BeginEdit           (int row, int col, wxGrid* grid) override;  // needed to get attached wxTextCtrl
    virtual bool    IsAcceptedKey       (wxKeyEvent& event);                        // check if key can start edit
    void            SetMinMax           (double min, double max, int precision=-1) { m_validator.SetMinMax          (min, max, precision);} // init/update min/max/precision
    void            SetExtraStartChars  (const wxString& startChars )           { m_validator.SetExtraStartChars    (startChars); }         // additional valid start chars
    void            UpdateMin           (double min)                            { m_validator.UpdateMin             (min);}                 // update minimum and force value inrange
    void            UpdateMax           (double max)                            { m_validator.UpdateMax             (max);}                 // update maximum and force value inrange
    void            UpdatePrecision     (int precision)                         { m_validator.UpdatePrecision       (precision); }          // update precision and force value inrange
private:
    MyValidator m_validator;        // the 'real' validator
};  // class MyGridCellEditorWithValidator

// A TextCtrl with validator of type MyValidator
class MyTextCtrlWithValidator : public wxTextCtrl
{
public:
    MyTextCtrlWithValidator(wxWindow *parent, wxWindowID id,
        const wxString&       value = wxEmptyString,
        const wxPoint&          pos = wxDefaultPosition,
        const wxSize&          size = wxDefaultSize,
        long                  style = 0);
    void SetMinMax          (double min, double max, int precision=-1) { m_validator.SetMinMax(min, max,precision); }
    void SetExtraStartChars (const wxString& startChars )              { m_validator.SetExtraStartChars(startChars); }  // additional valid start chars
    void UpdateMin          (double min)                               { m_validator.UpdateMin(min); }                  // update minimum and force value inrange
    void UpdateMax          (double max)                               { m_validator.UpdateMax(max); }                  // update maximum and force value inrange
    void UpdatePrecision    (int precision)                            { m_validator.UpdatePrecision(precision); }      // update precision and force value inrange
    virtual ~MyTextCtrlWithValidator() {}
private:    // for validator
    MyValidator m_validator;
};  // class MyTextCtrlWithValidator

#endif
