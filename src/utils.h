// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _UTILS_H_
#define _UTILS_H_

typedef unsigned int UINT;
#define FMT wxString::Format
extern const wxString ES;       // an Empty String
#include "mylog.h"

#define MY_UNUSED(x) do {} while(0 && (x) )

template <typename T>
bool IsInRange(const T& value, const T& low, const T& high)
{
    return (value >= low) && (value <= high);
}

bool EnableBell( bool bBell );  // enable/disable bellsound, return old value
void RingBell  ();              // ring the bell, if enabled

long     RoundLong   (long a,int b);            // rounding when deviding long by int

// convert a 0 terminated ascii string to unicode, using cp437
// if length == 0, then zero-termination expected
// default codepage is 437 (old pc characters)
wxString Ascii2Unicode(const char* a_inbuf, int length = 0, int codePage = 437);
int      Unicode2Ascii(const wxString& a_inbuf, char* a_outbuf, int a_sizeOut, int codePage = 437); // convert a unicode string to ascii in CP437

wxString GetDate();                             // get current date as: "zaterdag 12 augustus 2023"
wxString GetTime();                             // get current time as: "16:44:03"
wxString GetDateTime();                         // get current datetime as : GetDate()+GetTime()

#if 1   // ONLY my own stuf
#define LogMessage  MyLogMessage
#define LogWarning  MyLogWarning
#define   LogError  MyLogError
#define    LogInfo  MyLogInfo
#define   LogDebug  MyLogDebug
#define LogVerbose  MyLogInfo

#else   // my own stuf AND wx stuf
// log to wxLog and Mylog
#include <wx/log.h>
#define LogMessage(msg,...) do {wxLogMessage(msg, __VA_ARGS__); MyLogMessage(msg, __VA_ARGS__);} while (0)
#define LogWarning(msg,...) do {wxLogWarning(msg, __VA_ARGS__); MyLogWarning(msg, __VA_ARGS__);} while (0)
#define   LogError(msg,...) do {  wxLogError(msg, __VA_ARGS__);   MyLogError(msg, __VA_ARGS__);} while (0)
#define    LogInfo(msg,...) do {   wxLogInfo(msg, __VA_ARGS__);    MyLogInfo(msg, __VA_ARGS__);} while (0)
#define   LogDebug(msg,...) do {  wxLogDebug(msg, __VA_ARGS__);   MyLogDebug(msg, __VA_ARGS__);} while (0)
#define LogVerbose(msg,...) do {wxLogVerbose(msg, __VA_ARGS__);    MyLogInfo(msg, __VA_ARGS__);} while (0)
#endif

wxString U2String       (UINT x);  // (unsigned) int to string
wxString I2String       (int  x);  // int to string
wxString L2String       (long x);  // long to string
UINT MyGetFilesize      (const wxString& file);
bool ReadFileBinairy    (const wxString& file, void* buffer, UINT bufSize);
bool WriteFileBinairy   (const wxString& file, void* buffer, UINT bufSize);

wxString BoolToString   (bool bValue);
#endif
