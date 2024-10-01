// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SCHEMA_DATA_H_
#define _SCHEMA_DATA_H_

#include "utils.h"
#pragma warning( push )
#pragma warning( disable : 4200 )   // zero/unsized array
typedef struct SCHEMA_DATA
{
    UINT rounds;     UINT pairs;
    UINT tables;     const char* name;
    signed char data[];
} SCHEMA_DATA;

#pragma warning( pop )

extern const struct SCHEMA_DATA* const schemaTable[];   // zero terminated table of pointers to schema's
extern const int SCHEMA_NUM_ENTRIES;

#endif
