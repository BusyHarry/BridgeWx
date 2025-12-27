// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _DATABASE_H_
#define  _DATABASE_H_

#include "fileio.h"

namespace db
{
    #include "interfaces.h"
    bool ExistSession(UINT session);
}   // end namespace db

#endif
