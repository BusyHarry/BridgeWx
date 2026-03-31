// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _BASEFRAME_H_
#pragma once
#define _BASEFRAME_H_

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#include <wx/frame.h>
#include <wx/textfile.h>
#include <wx/string.h>
#include <wx/combobox.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>

#include "utils.h"
#include "choicemc.h"
#include "mygrid.h"

void AutoTestBusy(wxWindow* pWin, const wxString& msg);
#define AUTOTEST_BUSY(msg)    AutoTestBusy(this,msg)  /*set busyflag for autotest and CallAfter() to reset flag*/

#if 1
    #define MY_CHOICE MyChoice      /* MyChoice --> wxChoice */
#else
    #define MY_CHOICE MyChoiceMC    /* MyChoice --> ChoiceMC */
#endif

class wxBoxSizer;
class MyTextFile;
class wxRadioBox;

#define MY_SIZE_TXTCTRL_NUM(count) wxDefaultPosition, {GetCharWidth()*(1+(count)),-1} /* default size for textCtrl with number input*/
#undef wxOK_DEFAULT                     /* is defined as 0x00, so has no effect when setting focus....*/
#undef wxYES_DEFAULT                    /* is defined as 0x00, so has no effect when setting focus....*/
auto constexpr wxOK_DEFAULT     = wxOK; /* this is default, so define it like its button id!*/
auto constexpr wxYES_DEFAULT    = wxYES;/* this is default, so define it like its button id!*/
auto constexpr MY_BORDERSIZE    = 10;

wxPoint GetStaticRectPosition();    // position for invisible static controls AHK
wxSize  GetStaticRectSize();        //     size for invisible static controls AHK

using  CellData = struct CellData { int row=0;int column=0; wxString newData;};

struct CellInfo             // info for derived class when a gridcell changes
{
    int         row;        // the row of the changing cell
    int         column;     // the column of the changing cell
    void*       pList;      // the pointer to the active grid/list
    wxString    oldData;    // the original value
    wxString    newData;    // the new value
    explicit CellInfo(void* a_pList=nullptr, int a_row=0,int a_column=0, const wxString& a_oldData=ES, const wxString& a_newData=ES)
      : row     (a_row)
      , column  (a_column)
      , pList   (a_pList)
      , oldData (a_oldData)
      , newData (a_newData)
    {}
};

enum CellChanges
{
    CELL_CHANGE_OK       = false,
    CELL_CHANGE_REJECTED = true
};

class AHKHelper
{   // (hidden) helper class for AHK2 to receive mouseclick so we can send it to wxChoice/wxComboBox which don't have a 'title'
public:
    AHKHelper(wxWindow* pParent, wxWindow* pTarget, const wxString& label);
    ~AHKHelper() = default;
    wxString GetStaticLabel() const;
private:
    wxStaticText    m_staticAhk;    // 'dummy' target to issue mouseclick for wxChoice/wxComboBox
    wxWindow*       m_pTarget;      // real window  to send mouseclicks to
};

class MywxChoice : public wxChoice, private AHKHelper
{   // wrapper around standard wxChoice to be able to get an identification for this control for autotesting
public:
    MywxChoice(wxWindow *parent, wxWindowID id, const wxString& label = ES);
   ~MywxChoice() override = default;
    wxString GetLabel() const final;
};

class MywxComboBox : public wxComboBox, private AHKHelper
{   // wrapper around standard wxComboBox to be able to get an identification for this control for autotesting
public:
    MywxComboBox(wxWindow *parent, wxWindowID id,
                    const wxString& ahkLabel= wxEmptyString,
                    const wxPoint&  pos     = wxDefaultPosition,
                    const wxSize&   size    = wxDefaultSize,
                    int             count   = 0,
                    const wxString choices[]= nullptr,
                    long            style   = 0
        );
    ~MywxComboBox() final = default;
    wxString GetLabel() const final;
};

class Baseframe : public wxPanel
{
public:
    #define AUTOTEST_ADD_WINDOW(pWindow,mousePosName) m_winNames.push_back({pWindow,mousePosName})
    Baseframe(wxWindow* pParent, UINT pageId);
    ~Baseframe() override = default;

    void            DeleteConfig() { delete m_pConfig; m_pConfig = 0; }
    virtual void    RefreshInfo() = 0;  // (re)populate the current info
    virtual void    PrintPage();        // derived class should print current page if applicable
    virtual void    BackupData()=0;     // called if active panel is hidden. You should save changed data!
    const wxString& GetDescription()const;   // get description of this frame
    virtual void    AutotestRequestMousePositions(MyTextFile* pFile);   // request to add usefull mousepositions for autotesting
    //
    // When a derived class has a MyGrid-member, it can notice its owner
    // through this callback, that the value in a cell is going to change.
    // Its the responsibylity of the owner class to do something with this.
    // If returned CELL_CHANGE_REJECTED, we cancel the change.
    // The default implementation shows a msgbox with all the data and accepts the change.
    //
    // param:"cellInfo", the grid, the row, column, olddata, newdata
    virtual bool OnCellChanging(const CellInfo& cellInfo);

    wxString Unique(const wxString& name) const;    // if scripttesting, append class_id to common names
protected:
    wxWindow*    m_pParent;
    void         OnOk_          (const wxCommandEvent&);        // eventhandler for 'Ok' button, calls OnOk()
    virtual void OnOk           ();                             // default method for 'Ok' button for derived classes
    void         OnCancel_      (const wxCommandEvent&);        // eventhandler for 'Cancel' button, calls OnCancel()
    virtual void OnCancel       ();                             // default method for 'Cancel' button for derived classes
    virtual void DoSearch       (wxString&);                    // handler for 'any' search in derived class
    using pEventHandler = void (Baseframe::*)(wxCommandEvent &);
    #define EVT_CMD_HANDLER(handler) (pEventHandler)handler
    wxRadioBox* CreateRadioBox(const wxString& title, const wxArrayString& choices, pEventHandler pHandler, const wxString& autoName = "Radio");
    wxBoxSizer* CreateOkCancelButtons();                        // as it says
    wxBoxSizer* CreateSearchBox();                              // creates a static txt, textctl and search button in horizontal-boxSizer, caling DoSearch() when activated
    bool        AutotestAddMousePos(MyTextFile* pFile, const wxWindow* pWindow, const wxString& positionName) const; //add center of window as click-point for autotest
    void        AutoTestAddGridInfo(MyTextFile* pFile, const wxString& pageName, const MyGrid::GridInfo& gridInfo) const;

    const wxWindow* GetSearchWindow() const {return m_pTxtCtrlSearchBox;}
    struct WinAndName
    {   // store a window/name at construction time to get it added to AUTOTEST positions later on
        WinAndName(const wxWindow* a_pWindow, const wxString& a_mousePosName, const wxString& a_winTitle="")
            : pWindow       (a_pWindow)
            , mousePosName  (a_mousePosName)
            , winTitle      (a_winTitle)
        {}
        const wxWindow* pWindow = nullptr;
        wxString        mousePosName;
        wxString        winTitle;
    };
    std::vector<WinAndName> m_winNames;
    void AutoTestAddWindowsNames(MyTextFile* pFile, const wxString& pageName) const;     // add the values from m_mousePosNames to the position-file
    wxString            m_description;                          // describe this frame
    bool                m_bIsScriptTesting;                     // true if autotest active
private:
    void                OnSearch(const wxCommandEvent&);        // local handler for the search-button

    Baseframe*          m_pConfig            = nullptr;
    wxTextCtrl*         m_pTxtCtrlSearchBox  = nullptr;
    int                 m_iCurrentConfigHash = -1;              // last known config hash
    UINT                m_pageId;                               // identification of the actual page, added to names of common buttons etc when autotesting
};

#define MyAdd(item,...) Add(item->GetSizer(), __VA_ARGS__)

/*
* A class with a wxChoice box and a static text control, combined in a horizontal sizer
* More easy to declare and use
* Bind(): just as 'normal' because the class is derived from the standard wxChoice
* sizer->Add(instance): use sizer->MyAdd(instance,...), or sizer->Add(instance->GetSizer(),...)
*/
class MyChoice : public MywxChoice
{
public:
    MyChoice(wxWindow* parent, const wxString& staticText, const wxString& tooltip, const wxString& ahkLabel=ES);
    ~MyChoice() final = default;
    /*
    *     the Init() methods: initialize with numbers from 1 to count or the supplied strings.
    *     Set selection to 'selection'
    */
    void Init(UINT count                  , UINT selection = 0U, UINT offset = 0U);
    void Init(const wxArrayString& choices, UINT selection = 0U);
    void Init(const wxArrayString& choices, const wxString& selection );
    wxBoxSizer* GetSizer(){ return m_pBoxSizer; }
private:
    wxBoxSizer* m_pBoxSizer;
};

/*
* Same as class MyChoice, but with multiple columns i.o. one very large when you have 'many' choices
*/
class MyChoiceMC : public ChoiceMC
{
public:
    MyChoiceMC(wxWindow* parent, const wxString& title, const wxString& tooltip, const wxString& textCtrlTitle);
    ~MyChoiceMC() override = default;
    /*
    *     the Init() methods are buildin
    */
    wxBoxSizer* GetSizer(){ return m_pBoxSizer; }
private:
    wxBoxSizer* m_pBoxSizer;
};

// wxMessageBox has quirks: different fontsize/color if you have more lines
int MyMessageBox(const wxString& message, const wxString& caption = ES, long style=wxOK|wxCENTER, const wxPoint& position = wxDefaultPosition);

void BusyBox(const wxString& message = _("Busy..."), int milisecondsShow = 800, const wxPoint& position = wxDefaultPosition);

class MyTextFile : public wxTextFile
{   // wrapper around wxTextFile with buildin CP437 conversion for read/write
public:
    enum AccessType
    {
          READ
        , WRITE
        , READ_WRITE
    };
    MyTextFile() = default;
    explicit MyTextFile(const wxString& filename, AccessType access = READ, wxTextFileType textType = wxTextFileType_Dos);
    ~MyTextFile() final;
    void MyCreate(const wxString& filename, AccessType access = READ, wxTextFileType textType= wxTextFileType_Dos);
    bool IsOk() const;
    void  Flush();
private:
    AccessType      m_access    = READ;
    wxTextFileType  m_textType  = wxTextFileType_Dos;
    wxString        m_fileName;
    int             m_error     = -1;
    bool            m_bOk       = false;
};

int MyGetSingleChoiceIndex(const wxString& message, const wxString& caption, const wxArrayString& names, wxWindow* pParent = nullptr, int selection = 0);

#endif
