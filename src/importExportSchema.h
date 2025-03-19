// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _IMPORTEXPORTSCHEMA_H_
#define _IMPORTEXPORTSCHEMA_H_

#include "NewSchemaData.h"
namespace import
{
    /*
    * Read a textfile containing a schema.
    * The format is the same as the export from the Dutch NBB scoring program.
    */
    bool ReadFileSchemaDataNBB(const std::string& a_file, NEW_SCHEMA& a_schemaData, std::string& a_line);
}   // namespace import
#endif
