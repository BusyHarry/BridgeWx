// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _MY_LOG_H_
#define _MY_LOG_H_

class MyLog  // instantiate this class in your mainframe/dialog: MyLog m_myLog;
{
public:
    MyLog();
    explicit MyLog(wxWindowID frameId, bool bCreateNow = true);
    explicit MyLog(bool bCreateNow, wxWindowID frameId = wxID_ANY);
    bool IsShown();
    void SetCallbackOnHide(void(*ptr)());
    ~MyLog();

private:
    void Create();
};

enum class MyLogLevel
{
      MyLOG_MIN   = 0
    , MyLOG_Error = MyLOG_MIN  // a serious error, user must be informed about it
    , MyLOG_Warning    // user is normally informed about it but may be ignored
    , MyLOG_Message    // normal message (i.e. normal output of a non GUI app)
    , MyLOG_Info       // informational message (a.k.a. 'Verbose')
    , MyLOG_Debug      // disabled in release mode
    , MyLOG_Max   = MyLOG_Debug
};

wxWindowID  MyLogGetId      ();                         // get the id of the logwindow
void        MyLogShow       (bool bShow = true);        // show/hide the logwindow
void        MyLogSetLevel   (MyLogLevel level);         // messages upto this id will be shown
bool        MyLogIsEnabled  (MyLogLevel level);         // true, if logging enabled for 'level'

void MyLogGeneral(MyLogLevel level, const wxString& msg);

#define MyLogWarning(fmt,...) MyLogGeneral(MyLogLevel::MyLOG_Warning, FMT(fmt,__VA_ARGS__))
#define MyLogMessage(fmt,...) MyLogGeneral(MyLogLevel::MyLOG_Message, FMT(fmt,__VA_ARGS__))
#define MyLogVerbose(fmt,...) MyLogGeneral(MyLogLevel::MyLOG_Message, FMT(fmt,__VA_ARGS__))
#define   MyLogDebug(fmt,...) MyLogGeneral(MyLogLevel::MyLOG_Debug  , FMT(fmt,__VA_ARGS__))
#define   MyLogError(fmt,...) MyLogGeneral(MyLogLevel::MyLOG_Error  , FMT(fmt,__VA_ARGS__))
#define    MyLogInfo(fmt,...) MyLogGeneral(MyLogLevel::MyLOG_Info   , FMT(fmt,__VA_ARGS__))

#endif // _MY_LOG_H_
