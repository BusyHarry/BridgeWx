// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _UTILS_H_
#define _UTILS_H
#pragma once

#include <wx/log.h>

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

struct StringBuf
{   // buffer with a string and an index from where to start in the string
    // A function using this can update the index upto the position chars are handled
    StringBuf() {index = 0;}
    explicit StringBuf(const wxString& a_string, size_t a_index = 0){index = a_index; string = a_string;}
    size_t      index;
    wxString    string;
};

enum class ExpectedDecimalDigits
{
      DIGITS_1 = 1
    , DIGITS_2 = 2
};

long     RoundLong   (long a,int b);            // rounding when deviding long by int
wxString LongToAscii2(long score);              // return "float" string to score as xxx.yy
wxString LongToAscii1(long score);              // return "float" string to score as xxx.y or xxx if input is multiple of 10
int      Ascii1ToInt (StringBuf& stringBuffer); // convert "float" with 1 expected decimal digit to int and update "used chars" index
long     AsciiTolong (StringBuf& stringBuffer,ExpectedDecimalDigits longtype);                                  // "float" string to long
long     AsciiTolong (const wxString& string, ExpectedDecimalDigits longtype=ExpectedDecimalDigits::DIGITS_1);  // "float" string to long

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
