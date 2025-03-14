// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SCHEMA_DATA_H_
#define _SCHEMA_DATA_H_

#include "utils.h"
#pragma warning( push )
#pragma warning( disable : 4200 )   // zero/unsized array
typedef signed char SchemaDataType;
//typedef signed short SchemaDataType;  // use this if the schema's have pairnrs > 127
typedef struct SCHEMA_DATA
{
    #define SCHEMA_NAME_SIZE 28
    UINT rounds;     UINT pairs;
    UINT tables;     char name[SCHEMA_NAME_SIZE]; // having trouble with unfreed memory when using std::string for 'name'
   // todo: perhaps change 'data' to vector< vector<> > setdata [table][round]; vector< vector<> > pairdata [pair][round]
    SchemaDataType data[];
} SCHEMA_DATA;

#pragma warning( pop )

extern std::vector<const SCHEMA_DATA*> schemaTable;   // vector of pointers to schema's

#endif
