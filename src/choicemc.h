// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _CHOICEMC_H_
#define _CHOICEMC_H_

#include <wx/listctrl.h>
#include <wx/combo.h>

void AutoTestBusyMC(bool bSetCheck = true);

using UINT = unsigned int;

class PopupChoiceMC;
class wxTextCtrl;

class ChoiceMC : public wxComboCtrl
{
public:
    explicit    ChoiceMC            (wxWindow* pParent, const wxString& textCtrlTitle);
               ~ChoiceMC            () override = default;
    bool        SetSelection        (int selection);  // return true if new selection within limits
    bool        SetStringSelection  (const wxString& selection);
    void        Append              (const wxString& choice);
    wxString    GetStringSelection  () const override;
    int         GetSelection        () const;
    void        Clear               ();         // remove all content
    void        Init                (UINT count, UINT selection = 0U, UINT offset = 0U);
    void        Init                (const wxArrayString& choices, UINT selection = 0U);
    void        Init                (const wxArrayString& choices, const wxString& selection );
    void        Set                 (const wxArrayString& choices);
    long        InsertItem          (long index, const wxString& choice);
    void        SetMaxNumberOfRows  (UINT numberOfRows);    // nr of rows in choice
    void        SetColumnWidthInChars(UINT chars);
    void        SetAutoColumnWidth  (bool bAuto);
    void        CheckAutoSize       (const wxString& choice);
    void        SetPopupWidth       (int size);
    void        SetMaxPopupWidth    (int maxWidth);
    UINT        GetCount            () const;       // nr of items in popup
protected:
    bool        AnimateShow( const wxRect& rect, int flags ) wxOVERRIDE;

private:
    void ResetTextctrlSize();               // set max-size of combo if empty
    void ExpandNrOfRows(UINT itemNumber, bool bInit=false); // expand nr of rows if itemNumber > current rowcount
    static constexpr auto MC_DEFAULT_NR_OF_ROWS = 4;
    PopupChoiceMC*  m_pPopup;
    wxTextCtrl*     m_pTxtctrl;
    int             m_popupCharWidth;
    int             m_popupCharHeight;
    int             m_popupMaxWidth;
    bool            m_bAutoSize;
    int             m_currentColumnWidth = 0;
    UINT            m_numberOfRows       = 0;
    UINT            m_maxNumberOfRows    = 0;
    int             m_popupWidth         = 0;
    int             m_textMinSize;      // size of borders and button of combo-editbox
    UINT            m_nrOfColumns        = 0;
    wxTimer         m_timerPopupKillFocus;
};

#endif // _CHOICEMC_H_
