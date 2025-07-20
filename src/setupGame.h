// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SETUPGAME_H_
#define _SETUPGAME_H_
#pragma once
#include <wx/valnum.h>

#include "baseframe.h"

class wxDirPickerCtrl;
class MyTextCtrlWithValidator;

class SetupGame : public Baseframe
{
public:
    explicit SetupGame(wxWindow* pParent, UINT pageId);
            ~SetupGame() override { ; }
    void     RefreshInfo() final;
    void     AutotestRequestMousePositions(MyTextFile* pFile) final;
    void     PrintPage() final;

protected:
    void OnOk    () final;
    void OnCancel() final;
    virtual void BackupData   () override final;

private:
    // all the controls in this setup page, assumed to be initialised in ctor.
    wxDirPickerCtrl*    m_pDirPicker;
    wxTextCtrl*         m_pTxtCtrlMatch;
    wxTextCtrl*         m_pTxtCtrlDescr;
    wxTextCtrl*         m_pTxtCtrlSession;
    wxCheckBox*         m_pChkBoxNeuberg;
    wxCheckBox*         m_pChkBoxWeightedMean;
    wxCheckBox*         m_pChkBoxFF;
    wxCheckBox*         m_pChkBoxClock;
    wxTextCtrl*         m_pTxtCtrlLinesPP;
    wxCheckBox*         m_pChkBoxGroupResult;
    wxCheckBox*         m_pChkBoxGlobalNames;
    bool                m_bButler;
    wxStaticText*       m_pStaticTxtMaxMean;
    MyTextCtrlWithValidator*    m_pTxtCtrlMaxMean;
    MyTextCtrlWithValidator*    m_pTxtCtrlMin;
    MyTextCtrlWithValidator*    m_pTxtCtrlMax;

    // vars to remember the original values
    wxString            m_sDirName;
    wxString            m_sActiveMatch;
    wxString            m_sDescription;
    UINT                m_iSession;
    UINT                m_iMinClub;
    UINT                m_iMaxClub;
    bool                m_bNeuberg;
    bool                m_bWeightedAvg;
    bool                m_bFF;
    bool                m_bClock;
    wxString            m_sMaxMean;
    UINT                m_iLinesPP;
    bool                m_bGroupResult;
    bool                m_bGlobalNames;

    void OnFocusLostMin(wxFocusEvent& event);
    void OnFocusLostMax(wxFocusEvent& event);
    void OnEnterMin(wxCommandEvent&);
    void OnEnterMax(wxCommandEvent&);
    void HandleMin();
    void HandleMax();
};

#endif
