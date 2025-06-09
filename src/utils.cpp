// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/string.h>
#include <wx/datetime.h>
#include <wx/msgdlg.h>

#include <wx/wxcrt.h>

#include <wx/filefn.h>
#include <wx/valgen.h>
#include <wx/ffile.h>

#include "utils.h"
#include "cfg.h"
#include "baseframe.h"

const wxString ES;  // EmptyString

int Unicode2Ascii(const wxString& a_inbuf, char* a_outbuf, int a_outsize, int a_codePage)
{
    int len = ::WideCharToMultiByte(
        a_codePage,             // codepage, default 437
        0,                      // flags
        a_inbuf.c_str(),        // input buffer
        -1,                     // convert upto '\0'
        a_outbuf,               //
        a_outsize, NULL, NULL); // max outputsize

    a_outbuf[a_outsize-1]=0;
    return len;     // converted chars +'\0'
}   // Unicode2Ascii()

wxString Ascii2Unicode(const char* a_inbuf, int a_length, int a_codePage)
{
    wxString out;
    if (a_inbuf == 0 || *a_inbuf == 0) return out;  // if no input, return empty string

    if (a_length == 0)  // zero-terminated string
        a_length = strlen(a_inbuf);

    for (int ii = 0 ; ii < a_length; ++ii)
    {
        char toConvert = a_inbuf[ii];
        if ( (toConvert >= ' ') && (toConvert <= '{') )
        {   // no conversion needed, just plain ascii
            out += toConvert;
            continue;
        }

        wchar_t outbuf[4];  //{0};
        int count = ::MultiByteToWideChar
        (
            a_codePage, // code page
            0,          // flags: 
            &toConvert, // input string
            1,          // its length (NUL-terminated if -1, else the number of chars to convert)
            outbuf,     // wide output buffer
            3           // size of output buffer in wchars: if 0, then function returns nr of wchars needed for output, inclusive '\0'
        );
        outbuf[count] = 0;
        out += outbuf;  // add character to output string
    }

    return out;
}   // Ascii2Unicode()

wxString U2String(UINT x)
{
    return FMT("%u", x);
}   // U2String()

wxString I2String(int x)
{
    return FMT("%d", x);
}   // I2String()

wxString L2String(long x)
{
    return FMT("%ld", x);
}   // L2String()

long RoundLong(long a,int b)    // rounding when deviding long by int
{
    if (b == 0)
        return 0;

    auto xx = (10*a)/b;
    auto xxx= (xx + (xx < 0? -5:5))/10;
    return xxx;
}   // RoundLong()

wxString LongToAscii2(long score)
{      // return "float" string to score as xxx.yy
    int beforeDp = (int)(score/100);
    if (beforeDp == 0 && score < 0)
        return FMT("-0.%02d",(int)(-score));
   return FMT("%d.%02d",beforeDp, (int)(labs(score) % 100));
}   // LongToAscii2()

wxString LongToAscii1(long score)
{      // return "float" string to score as xxx.y or xxx if input is multiple of 10
//    if (labs(score) % 10 != 0)
    if ((score % 10) != 0)
    {
        long beforeDp = score/10;
        if (beforeDp == 0 && score < 0)
        {
            return FMT("-0.%ld", -score);
        }
        return FMT("%ld.%ld", beforeDp, labs(score) % 10);
    }

    return FMT("%ld  ", score/10);    // replace .0 with 2 spaces
}   // LongToAscii1()

long  AsciiTolong( const wxString& a_string, ExpectedDecimalDigits a_longtype)
{
    StringBuf tmp(a_string);
    return AsciiTolong( tmp, a_longtype);
}   // AsciiTolong()

long AsciiTolong( StringBuf& a_buffer, ExpectedDecimalDigits a_longtype)
{   // Convert a string to a (rounded) long, ignore dp and expect max 2 digits after dp.
    // The index in the buffer denotes the starting position in the string.
    // At the end, the index points after the last used character.
    // type1: long with 1 digit after dp, type 2 has max 2 digits.
    bool            sign    = false;
    bool            dp      = false;
    long            result  = 0L;
    int             digits  = a_longtype == ExpectedDecimalDigits::DIGITS_1 ? 1 : 0;
    size_t          index   = a_buffer.index;
    const wxChar*   buf     = a_buffer.string.wc_str();   // need a zero ended array

    while (wxIsspace(buf[index])) ++index;    // skip whitespace
    if (buf[index] == '-')                    // check for optional sign
    {
        ++index;
        sign = true;
    }
    else if (buf[index] == '+') ++index;

    for ( ; ; ++index)                      // handle all digits
    {   if (buf[index] == '.')              // dp found
        {   dp = true;
            continue;
        }
        if (!wxIsdigit(buf[index])) break;  // non-digit: stopp
        if (dp)
        {    if (digits++ >= 3)             // consume all digits we don't want
                continue;
        }
        result = result*10 + (buf[index] - '0');
    }
    while (digits++ < 3)                // force "3" digits behind comma
    {
        result *= 10;
    }
    result = (result+5)/10;             // rounding
    if (sign)
    {
        result = -result;
    }

    a_buffer.index=index;               //update to 'unused' position
    return result;
}   // AsciiToLong()

int Ascii1ToInt( StringBuf& a_buffer )
{   // convert "float string" with 1 optional decimal digit to int and update "used chars" index

    return AsciiTolong( a_buffer, ExpectedDecimalDigits::DIGITS_1 )/10;
}   // Ascii1ToInt()

/*
Format Description: formatting time/date representation
%a Abbreviated name of the day of the week.
%A Full name of the day of the week.
%b Abbreviated month name.
%B Full name of the month.
%c Preferred date and time (UTC) representation for the current locale.
%C Century number (year/100) as a 2-digit integer.
%d Day of the month as a decimal number (range 01 to 31).
%e Day of the month as a decimal number (range 1 to 31).
%F ISO 8601 date format (equivalent to %Y-%m-%d ).
%G ISO 8601 week-based year with century as a decimal number. The 4-digit year corresponds to the ISO week number (see %V ). It has the same format and value as %Y , except that if the ISO week number belongs to the previous or next year, that year is used instead.
%g Like %G , but without the century, that is, with a 2-digit year (00-99).
%H Hour as a decimal number with a 24-hour clock (range 00 to 23). See also %k .
%I Hour as a decimal number with a 12-hour clock (range 01 to 12). See also %l .
%j Day of the year as a decimal number (range 001 to 366).
%k Hour as a decimal number with a 24-hour clock (range 0 to 23). See also %H .
%l Hour as a decimal number with a 12-hour clock (range 1 to 12). See also %I .
%m Month as a decimal number (range 01 to 12).
%n Month as a decimal number (range 1 to 12).
%M Minute as a decimal number (range 00 to 59).
%p Either "AM" or "PM" according to the given time value. Noon is treated as "PM" and midnight as "AM".
%P Like %p but lowercase ("am" or "pm").
%s Number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
%S Seconds as a decimal number (range 00 to 59).
%u Day of the week as a decimal number (range 1 to 7), where Monday is 1. See also %w .
%V ISO 8601 week number of the current year as a decimal number (range 01 to 53), where week 1 is the first week with at least 4 days in the new year (that is, the first Thursday).
%w Day of the week as a decimal number (range 0 to 6), where Sunday is 0. See also %u .
%x Preferred date format for the current locale without the time.
%X Preferred time format for the current locale without the date.
%y Year as a decimal number without the century (range 00 to 99).
%Y Year as a decimal number including the century.
%z The +hhmm or -hhmm numeric time zone (that is, the hour and minute offset from UTC).
%Z Time zone name or abbreviation.

*/
wxString GetDate()
{
    if (cfg::IsScriptTesting()) return __DATE__AUTO;
    return wxDateTime::Now().Format("%A %x");   //%d %B %G");             //zaterdag 12 augustus 1923
}   // GetDate()

wxString GetTime()
{
    if (cfg::IsScriptTesting()) return __TIME__AUTO;
    return wxDateTime::Now().FormatTime();              //16:44:03 or 04:44:03 PM
}   // GetTime()

wxString GetDateTime()
{
    if (cfg::IsScriptTesting()) return __DAY__AUTO + " " + __DATE__AUTO + " "  __TIME__AUTO;
    //xgettext:TRANSLATORS: Set order of types to your country order 
    return wxDateTime::Now().Format(_("%A %B %d, %G %X"));    //zaterdag 12 augustus 2023 16:44:03
}   // GetDateTime()

UINT MyGetFilesize(const wxString& a_file)
{
    union mysize{wxULongLong ull;UINT u;mysize(){ull=0;}};
    
    mysize size;
    size.ull = wxFileName::GetSize(a_file);
    if (size.ull == wxInvalidSize)
        return UINT_MAX;
    return size.u;
#if 0
    wxStructStat buf;
    int result = wxStat(a_file, &buf);
    if (result == 0)
        return buf.st_size;
    return -1;
#endif
}   // MyGetFilesize()

bool WriteFileBinairy(const wxString& a_file, void* a_buffer, UINT a_bufSize)
{
    bool bOk = true;
    wxString sError;

    wxFFile fn;
    fn.Open(a_file, "wb");
    if (a_bufSize != fn.Write(a_buffer, a_bufSize))
    {
        bOk = false;
        sError = _(": write error");
    }

    fn.Close();

    if (!bOk)
    {
        LogMessage(a_file+sError);
        MyMessageBox(a_file+_(": error writing to file"));
    }

    return bOk;
}   // WriteFileBinairy()

bool ReadFileBinairy(const wxString& a_file, void* a_buffer, UINT a_bufSize)
{
    bool bOk = true;
    wxString sError;
    wxFFile fn;

    if (!wxFileExists(a_file))
    {   // file does not exist yet, ///// do not: create and zero it (else Open() will give a popup error...)
        memset(a_buffer, 0, a_bufSize);
        return false; //don't create file if not existing yet
#if 0
        fn.Open(a_file,"wb");
        if (a_bufSize != fn.Write(a_buffer, a_bufSize))
        {
            bOk = false;
            sError = _(": write error");
        }

        fn.Close();
#endif
    }

    fn.Open(a_file,"rb");
    if (!fn.IsOpened())
    {
        bOk = false;
    }
    else
    {
        auto fileSize = fn.Length();
        if (fileSize != a_bufSize)
        {
            bOk = false;
            sError = _(": wrong size");
        }
        else
        {
            if (a_bufSize != fn.Read(a_buffer, a_bufSize))
            {
                bOk = false;
                sError = _(": readerror");
            }
        }
    }

    if (!bOk)
    {
        LogError(a_file+sError);
        MyMessageBox(a_file+_(": error reading file"));
    }
    return bOk;
}   // ReadFileBinairy()

wxString BoolToString(bool a_bValue)
{
    return a_bValue ? _("yes") : _("no");
}   // BoolToString()
