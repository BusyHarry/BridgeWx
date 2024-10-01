// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

/*
* remove lines from input (first argument) which:
*  - are empty
*  - start with '#'
*  - contain "[Creat|Revis]ion-Date: "
* output to console, so you have to redirect/pipe the output to use it.
*/

//compile with UNICODE

#include <windows.h>
#include <stdio.h>

int wmain(int argc, const wchar_t* argv[])
{
    if (argc < 2)
    {
        fputs("Missing argument (file to strip)", stderr);
        exit(1);
    }

    FILE* fpIn;
    auto error  = _wfopen_s(&fpIn, argv[1], L"rt");
    if (fpIn == nullptr)
    {
        fputs("Can't open inputfile, exiting", stderr);
        exit(1);
    }

    #define BUFSIZE 2000
    char buf[BUFSIZE+1];
    do
    {
        if ( nullptr == fgets(buf, BUFSIZE, fpIn ))
            break;      // end of file
        char dummy;
        if (buf[0] == '\n')
            continue;   // ignore empty lines
        if ( 1 == sscanf_s(buf, " #%c", &dummy, static_cast<unsigned int>(sizeof(dummy))))
            continue;   // ignore lines starting with '#'
        if (nullptr != strstr(buf, "ion-Date: "))
            continue;   // ignore lines that contain "ion-Date: "
        fputs(buf, stdout); // a wanted line, so output it to console
    } while (1);

    fclose(fpIn);
    return 0;
}   // wmain()
