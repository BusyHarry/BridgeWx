// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SETUPPRINTER_H_
#define _SETUPPRINTER_H_

#include "baseframe.h"

class wxWindow;
class MyTextCtrlWithValidator;

class SetupPrinter : public Baseframe
{
public: 

    explicit SetupPrinter(wxWindow* pParent, UINT pageId);
            ~SetupPrinter() override;
    void     RefreshInfo() final;
    void     AutotestRequestMousePositions(MyTextFile* a_pFile) final;  // create needed mousepositions
    void     PrintPage() final;
protected:
    void OnOk                   () final;
    void OnCancel               () final;
    virtual void BackupData     () override final;
private:

    MyTextCtrlWithValidator* m_pTxtCtrlLinesPP;
    MY_CHOICE*          m_choiceBoxPrn;
    wxCheckBox*         m_pChkBoxFF;
    wxCheckBox*         m_pChkBoxRemote;
    wxButton*           m_pBtnFont;
    wxBoxSizer*         m_mOkCancel;
};

#endif
