// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _MY_LOG_H_
#define _MY_LOG_H_

class wxWindow;
class wxString;

/**
    - instantiate this class in your mainframe/dialog: MyLog m_myLog;
    - show/hide it via MyLog::Show() (initially its hidden)
    - set the logginglevel via MyLog::SetLevel()
    - log messages via the MyLogxxxx methods
    - use the other methods when needed
**/
class MyLog
{
public:
    enum class Level
    {
          LOG_MIN   = 0
        , LOG_Error = LOG_MIN   // a serious error, user must be informed about it
        , LOG_Warning           // user is normally informed about it but may be ignored
        , LOG_Message           // normal message (i.e. normal output of a non GUI app)
        , LOG_Info              // informational message (a.k.a. 'Verbose')
        , LOG_Debug             // disabled in release mode
        , LOG_Max   = LOG_Debug
    };

    MyLog();
    explicit MyLog(wxWindowID frameId, bool bCreateNow = true);
    explicit MyLog(bool bCreateNow, wxWindowID frameId = wxID_ANY);
    ~MyLog();

    // static functions to have easier/global access to info/methods
    static void         DoLog            (Level level, const wxString& msg);    // here the real logging is done
    static void         FontScale        (float scale);                         // scale the textsize of the logger for msg's hereafter
    static wxWindowID   GetId            ();                                    // get the id of the logwindow
    static bool         IsEnabled        (Level level);                         // true, if logging enabled for 'level'
    static bool         IsShown          ();                                    // return true if logwindow is visable
    static void         SetAppDebugging  ();                                    // this app is in debug-mode
    static void         SetCallbackOnHide(void(*pCallbackFun)());               // 'pCallbackFun' is called when window is hidden/closed
    static void         SetLevel         (Level level);                         // messages upto this id will be logged
    static void         SetMainFrame     (const wxWindow* pMainframe);          // sets the mainframe, so log window can be positioned alongside
    static void         SetScriptTesting ();                                    // this app is in scripttesting mode
    static void         Show             (bool bShow = true);                   // show/hide the logwindow

private:
    void Create();
};

#define MyLogWarning(fmt,...) MyLog::DoLog(MyLog::Level::LOG_Warning, wxString::Format(fmt, __VA_ARGS__))
#define MyLogMessage(fmt,...) MyLog::DoLog(MyLog::Level::LOG_Message, wxString::Format(fmt, __VA_ARGS__))
#define   MyLogDebug(fmt,...) MyLog::DoLog(MyLog::Level::LOG_Debug  , wxString::Format(fmt, __VA_ARGS__))
#define   MyLogError(fmt,...) MyLog::DoLog(MyLog::Level::LOG_Error  , wxString::Format(fmt, __VA_ARGS__))
#define    MyLogInfo(fmt,...) MyLog::DoLog(MyLog::Level::LOG_Info   , wxString::Format(fmt, __VA_ARGS__))

#endif // _MY_LOG_H_
