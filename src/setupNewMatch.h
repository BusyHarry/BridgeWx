// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SETUPNEWMATCH_H_
#define _SETUPNEWMATCH_H_

#include "baseframe.h"

class wxWindow;
class wxDirPickerCtrl;
class wxTextCtrl;

class SetupNewMatch : public Baseframe
{
public: 

    explicit SetupNewMatch(wxWindow* pParent, UINT pageId);
            ~SetupNewMatch() override;
    void     RefreshInfo() final;
    void     AutotestRequestMousePositions(MyTextFile* a_pFile) final;  // create needed mousepositions
    void     PrintPage() final;
protected:
    void UpdateSelection    ();
    virtual void OnOk       () final;              // handler for 'Ok' button
    virtual void OnCancel   () final;              // handler for 'Cancel' button
    virtual void BackupData () override final;

private:
    wxDirPickerCtrl*    m_pDirPicker;
    MyTextCtrl*         m_pTxtCtrlSession;
    MywxComboBox*       m_pComboBoxMatch;
    MyTextCtrl*         m_pTxtCtrlDbType;
};

#endif
