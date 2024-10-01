// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SETUPSCHEMA_H_
#define _SETUPSCHEMA_H_

#include "baseframe.h"

class wxWindow;
class MyChoice;

class SetupSchema : public Baseframe
{
public:
    explicit SetupSchema(wxWindow* pParent, UINT pageId);
            ~SetupSchema() override { ; }
    void     RefreshInfo() final;
    void     AutotestRequestMousePositions(MyTextFile* pFile) final;
    void     PrintPage() final;

protected:
    void OnOk                   () final;
    void OnCancel               () final;
    virtual void BackupData     () override final;

    void OnNextGroup            (wxCommandEvent&);
    void OnSelectGroup          (wxCommandEvent&);
    void OnSelectSchema         (wxCommandEvent&);
    void OnEnterRoundsSetSize   (wxCommandEvent&);
    void OnEnterPairs           (wxCommandEvent&);
    void OnEnterAbsent          (wxCommandEvent&);
    void OnEnterNrOfGroups      (wxCommandEvent&);
    void OnEnterGroupChars      (wxCommandEvent&);

    void OnLostFocusGroupChars  (wxFocusEvent&);
    void OnLostFocusNrOfPairs   (wxFocusEvent&);        // wxCommandEvent wxEVT_TEXT signals every char-change...
    void OnLostFocusAbsent      (wxFocusEvent&);
    void OnLostFocusGames       (wxFocusEvent&);
    void OnLostFocusNrOfGroups  (wxFocusEvent&);
    void OnLostFocusRoundsSetSize(wxFocusEvent&);

private:
    // all the controls in this setup page, assumed to be initialised in ctor.
    wxTextCtrl*                     m_pTxtCtrlNrOfGames;
    MyTextCtrl*                     m_pTxtCtrlNrOfRounds;
    MyTextCtrl*                     m_pTxtCtrlSetSize;
    MyTextCtrl*                     m_pTxtCtrlNrOfGroups;
    MY_CHOICE*                      m_pChoiceBoxGroup;
    MyChoiceMC*                     m_pChoiceBoxSchemas;
    MyTextCtrl*                     m_pTxtCtrlPairs;
    wxTextCtrl*                     m_pTxtCtrlGroupChars;
    MyTextCtrl*                     m_pTxtCtrlAbsent;
    MyTextCtrl*                     m_pTxtCtrlFirstGame;
    wxButton*                       m_pButtonNextGroup;
    std::vector<cfg::GROUP_DATA>    m_groupData;

    void RefreshInfoGroup(int index);   // init display for groupinfo
    void UpdateNrOfGames();             // if rounds/setSize changes, update readonly games
    void HandlePairs();                 // handle pair changes
    void HandleAbsent();                // handle absent changes
    void HandleNrOfGroups();            // handle groupcount changes
    void HandleGroupChars();            // handle groupchar changes
};

#endif // _SetupSchema_H_
