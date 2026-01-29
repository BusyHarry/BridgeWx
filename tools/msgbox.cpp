// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
// will display a messagebox with first param as title and each next parameter as a separate line of text
//

//compile with UNICODE

#include <windows.h>

constexpr auto  bufSize         = 10000;
wchar_t         msg[bufSize]    = {0};

int wmain(int argc, wchar_t* argv[])
{
    const wchar_t* pTitle = (argc > 1) ? argv[1] : L"Missing title";
    for (int index = 2; index < argc; ++index)
    {
        if (index != 2) (void)wcscat_s(msg, bufSize, L"\n");
        (void)wcscat_s(msg, bufSize, argv[index]);
    }

    MessageBox(nullptr, msg, pTitle, 0 );
    return 0;
}
