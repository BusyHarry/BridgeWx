// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _NEWSCHEMADATA_H_
#define _NEWSCHEMADATA_H_

#include <vector>
#include <string>

typedef unsigned short NewSchemaDataType;
struct NewTableInfo
{
    NewSchemaDataType pairNS         = 0;
    NewSchemaDataType pairEW         = 0;
    NewSchemaDataType set            = 0;
    NewSchemaDataType setFromTable   = 0;    // if  nonzero -> borrow set from this table;
};

class NEW_SCHEMA
{
public:
    NewSchemaDataType    pairs  = 0;
    NewSchemaDataType    tables = 0;
    NewSchemaDataType    sets   = 0;
    NewSchemaDataType    rounds = 0;
    NewSchemaDataType    schemaType  = 0;   // 0=pair schema, 1=individual schema
    std::string name;
    std::vector< std::vector<NewTableInfo> > tableData; // [round][table] NB 1-based: round 0 and table 0 are dummies!
    static const NewSchemaDataType SCHEMA_DELETED = 9999;
    inline bool operator == (const NEW_SCHEMA& rhs) const
    {
        return     pairs   == rhs.pairs
                && tables  == rhs.tables
                && sets    == rhs.sets
                && rounds  == rhs.rounds
                && name    == rhs.name
                // schemaType ALWAYS 0
                ;
    }
    inline bool operator != (const NEW_SCHEMA& rhs) const { return !(*this == rhs); }
};

extern std::vector<NEW_SCHEMA> newSchemaTable;   // vector of schema's

#endif
