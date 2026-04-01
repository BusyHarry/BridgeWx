// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
// will output to console and file <.\buildDate.h> a string like:
// static const char* buildDate = "zaterdag 20 juli 2024 @ 16:07:36";

// locale to use when no commandline argument is given
// #define DEFAULT_LOCALE "en-UK"
#define DEFAULT_LOCALE GetSystemLocale()

#include <Windows.h>
#include <ctime>
#include <iostream>
#include <io.h>

/*
Format Description: formatting time/date representation
%a Abbreviated name of the day of the week.
%A Full name of the day of the week.
%b Abbreviated month name.
%B Full name of the month.
%T equivalent to "%H:%M:%S" (the ISO 8601 time format)
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

static char build[256];

void DateTime1()
{   //"vrijdag 19 juli 2024 @ 20:51:51";
    // change the names, if you want/need another language
    static const char* days[]=
    {"zondag", "maandag", "dinsdag", "woensdag", "donderdag", "vrijdag", "zaterdag"};
    static const char* months[]=
    {"januari","februari","maart","april","mei","juni","juli","augustus","september","oktober","november","december"};
    auto tm = std::time(0);   // get time now
    auto pNow = std::localtime(&tm);
    sprintf(build,"static const char* buildDate = \"%s %i %s %i @ %02i:%02i:%02i\";"
        , days[pNow->tm_wday]
        , pNow->tm_mday
        , months[pNow->tm_mon]
        , 1900+pNow->tm_year
        , pNow->tm_hour
        , pNow->tm_min
        , pNow->tm_sec
    );
}   // DateTime1()

static const char* GetSystemLocale()
{
    static char cLocaleName[LOCALE_NAME_MAX_LENGTH];    // locale name in 8bit chars

    cLocaleName[0]=0;                       // clear name
    LCID lcid = GetThreadLocale();          // get the locale id
    wchar_t wName[LOCALE_NAME_MAX_LENGTH];  // get it in wchar's
    if (LCIDToLocaleName(lcid, wName, LOCALE_NAME_MAX_LENGTH, 0) == 0)
        return cLocaleName;
    int nCodePage = GetACP();               // active codepage
    int len = ::WideCharToMultiByte(        // convert to a char buffer
        nCodePage,                          // codepage
        0,                                  // flags
        wName,                              // input buffer
        -1,                                 // convert upto '\0'
        cLocaleName,                        // char output buffer
        sizeof(cLocaleName), NULL, NULL);   // max outputsize

    cLocaleName[LOCALE_NAME_MAX_LENGTH-1]=0;

    return cLocaleName;
}   // GetSystemLocale()

void DateTime2(const char* pLocale)
{
    //"vrijdag 9 juli 2024 @ 20:51:51";
    std::time_t tm = std::time(nullptr);
    std::locale::global(std::locale(pLocale));
    std::strftime(build, sizeof(build), "static const char* buildDate = \"%A, %x @ %X\";", std::localtime(&tm));
    /*
    * For non-latin languages we need to do something like:
    *   static const wchar_t* buildDate = L"xyz";
    *   wchar_t buildw[256];
    *   auto len= wcsftime( buildw, sizeof(buildw)/sizeof(wchar_t), L"%A %e %B %Y @ %T", std::localtime(&tm));
    * but how to get it in the build.....
    * Or perhaps: add .utf8 to locale and in application: wxString::FromUtf8(build)
    */
}   //DateTime2()

const char compiletime[] ="<buildtime: " __DATE__ ", " __TIME__ ">";
int main(int argc, const char* argv[])
{
    char buf[1000]={0};
    if (argc > 1) sprintf(buf, ", now using <%s>", argv[1]);
    fprintf(stderr, "%s [locale], default <%s>%s\n", argv[0], DEFAULT_LOCALE, buf);
    fprintf(stderr, "Generates a string/file containing the buildtime in utf8.\n\n");

    compiletime;
    const char* utf8    = ".utf8";
    char*       pBuf    = nullptr;
    auto        pLocale = (argc > 1) ? argv[1] : DEFAULT_LOCALE;

    if (strstr(pLocale, utf8) == nullptr)
    {   // add .utf8 for locale, so we can handle non-latin languages
        pBuf = new char[strlen(pLocale)+strlen(utf8)+1];
        strcpy(pBuf, pLocale);
        strcat(pBuf, utf8);
        pLocale = pBuf;
    }
//    DateTime1();
//    printf("%s", build);    // console output
//    puts("");

    DateTime2(pLocale);
    printf("%s", build);    // console output

    if (_isatty(_fileno(stdout)))
    {   // only create file, if no output redirection
        char cmd[256];
        sprintf(cmd, "echo %s > buildDate.h", build);
        system((const char*)cmd);
    }

    delete[] pBuf;
    return 0;
}   // main()
