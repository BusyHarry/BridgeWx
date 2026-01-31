// Copyright(c) 2026-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if not defined _SQLITE_H
#define _SQLITE_H

#include "fileio.h"

namespace sql
{
    #include "interfaces.h"
    bool ExistSession(UINT session);

}   // end namespace db

#endif
