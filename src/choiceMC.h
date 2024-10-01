// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _CHOICEMC_H_
#define _CHOICEMC_H_

#include <wx/listctrl.h>
#include <wx/combo.h>

void AutoTestBusyMC(bool bSetCheck = true);

typedef unsigned int UINT;

class PopupChoiceMC;
class wxTextCtrl;

class ChoiceMC : public wxComboCtrl
{
public:
    explicit    ChoiceMC            (wxWindow* pParent, const wxString& textCtrlTitle);
    virtual     ~ChoiceMC           ();
    bool        SetSelection        (int selection);  // return true if new selection within limits
    bool        SetStringSelection  (const wxString& selection);
    void        Append              (const wxString& choice);
    wxString    GetStringSelection  () const;
    int         GetSelection        () const;
    void        Clear               ();         // remove all content
    void        Init                (UINT count, UINT selection = 0U, UINT offset = 0U);
    void        Init                (const wxArrayString& choices, UINT selection = 0U);
    void        Init                (const wxArrayString& choices, const wxString& selection );
    void        Set                 (const wxArrayString& choices);
    long        InsertItem          (long index, const wxString& choice);
    void        SetNumberOfRows     (UINT numberOfRows);    // nr of rows in choice
    void        SetColumnWidthInChars(UINT chars);
    void        SetAutoColumnWidth  (bool bAuto);
    void        CheckAutoSize       (const wxString& choice);
    void        SetPopupWidth       (int size);
    void        SetMaxPopupWidth    (int maxWidth);
    void        ShowSizes           (UINT id);  //show sizes from combo/text
    UINT        GetCount            ();         // nr of items in popup
protected:
    virtual bool AnimateShow( const wxRect& rect, int flags ) wxOVERRIDE;

private:
    #define MC_DEFAULT_NR_OF_ROWS 4
    PopupChoiceMC*  m_pPopup;
    wxTextCtrl*     m_pTxtctrl;
    int             m_popupCharWidth; 
    int             m_popupCharHeight;
    int             m_popupMaxWidth;
    bool            m_bAutoSize;
    int             m_currentColumnWidth;
    UINT            m_numberOfRows;
    int             m_popupWidth;
    int             m_textMinSize;      // size of borders and button of combo-editbox
    UINT            m_nrOfColumns;
    wxTimer         m_timerPopupKillFocus;
};

#endif // _CHOICEMC_H_