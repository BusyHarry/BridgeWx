// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _IMPORTEXPORTSCHEMA_H_
#define _IMPORTEXPORTSCHEMA_H_

#include <string>
#include <vector>

namespace import
{
    struct tableInfo
    {
        int pairNS;
        int pairEW;
        int set;
    };

    struct Schema
    {   // info as read from a file
        int pairs   = 0;
        int tables  = 0;
        int sets    = 0;
        int rounds  = 0;
        int dummy   = 0;
        std::vector<std::vector<tableInfo>> info;   // info[round][table].xxx
        std::string schemaName;
    };

    /*
    * Read a textfile containing a schema.
    * The format is the same as the export from the Dutch NBB scoring program.
    */
    bool ReadFileSchemaDataNBB(const std::string& a_file, Schema& a_schemaData, std::string& a_line);
}   // namespace import
#endif
