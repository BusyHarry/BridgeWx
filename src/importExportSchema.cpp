﻿// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <iostream>
#include <fstream>
#include <sstream>

#include "importExportSchema.h"

static bool GetNonEmptyLine(std::ifstream& a_file, std::string& a_line)
{   // return true if we have a none empty line, false if eof
    for(;;)
    {
        if (a_file.eof()) return false;
        std::getline(a_file, a_line);
        a_line.erase(0, a_line.find_first_not_of(" \t\n")); // ltrim()
        if (a_line.empty()) continue;
        if (a_line[0] == ';') continue;
        break;
    }
    return true;
}   // GetNonEmptyLine()

// sanity checks
#define MAX_PAIRS   127
#define MAX_TABLES  20
#define MAX_SETS    10
#define MAX_ROUNDS  10

#define CHECK_VALUE(value,max) if ((value) < 1 || (value) > (max)) return false

namespace import
{
    /*
    * Next is the code for importing a (new) schema.
    * This is the same as the export from the Dutch NBB scoring program.
    * Empty lines and lines starting with ';' are ignored: comments
    * Syntax:

    8 4 5 5 0                   ; pairs tables rounds sets schema-type (0=pair schema, 1=individual schema)
    1-2 1 3-4 2 5-6 3 7-8 4     ; round 1: table 1 to N: NS-EW set    : 0-0 0 -> no play at this table in this round
    1-6 2 5-3 1 7-2 3 8-4 5     ; round 2: table 1 to N: NS-EW set
    1-5 4 3-6 5 4-7 1 2-8 2     ; etc
    1-4 3 3-2 4 7-5 2 6-8 1 
    1-7 5 6-4 4 2-5 5 8-3 3 
    #<name>Short Howell 8</name>    ; descriptive name for this schema
    */

    bool ReadFileSchemaDataNBB(const std::string& a_file, NEW_SCHEMA& a_schemaData, std::string& a_line)
    {
        a_schemaData.tableData.clear();
        a_schemaData.name.clear();
        a_line.clear();
        std::ifstream file(a_file);
        if (!file.is_open()) return false;
        if (!GetNonEmptyLine(file, a_line)) return false;
        std::stringstream definition(a_line);
        definition >> a_schemaData.pairs >> a_schemaData.tables >> a_schemaData.rounds >> a_schemaData.sets >> a_schemaData.schemaType;
        CHECK_VALUE(a_schemaData.pairs , MAX_PAIRS );
        CHECK_VALUE(a_schemaData.tables, MAX_TABLES);
        CHECK_VALUE(a_schemaData.sets  , MAX_SETS  );
        CHECK_VALUE(a_schemaData.rounds, MAX_ROUNDS);
        if (a_schemaData.schemaType != 0)
            return false;       // we can only use pair-schemas
        a_schemaData.tableData.resize(1);    // dummy round 0
        for (int round = 1; round <= a_schemaData.rounds; ++round)
        {
            if (!GetNonEmptyLine(file, a_line)) return false;
            std::stringstream      ss(a_line);
            std::vector<NewTableInfo> roundData;
            roundData.resize(1);    // dummy table 0
            for (int table = 1; table <= a_schemaData.tables; ++table)
            {
                NewTableInfo tableData;
                char dash; // for the inbetween char '-'
                ss >> tableData.pairNS >> dash >> tableData.pairEW >> tableData.set;
                if (tableData.pairNS == 0 && tableData.pairEW == 0 && tableData.set == 0)
                {
                    ;   // no play at this table in this round, so don't check.....
                }
                else
                {
                    CHECK_VALUE(tableData.pairNS, a_schemaData.pairs);
                    CHECK_VALUE(tableData.pairEW, a_schemaData.pairs);
                    CHECK_VALUE(tableData.set   , a_schemaData.sets );
                }
                roundData.push_back(tableData);
            }
            a_schemaData.tableData.push_back(roundData);
        }   // all rounds/tables handled, now get the name
        if (!GetNonEmptyLine(file, a_line)) return false;
        // line should contain: #<name>Short Howell 8</name>    ; descriptive name for this schema
        std::string     startName = "#<name>";
        const size_t    startLen  = startName.size();
        size_t          start     = a_line.find(startName);
        if (std::string::npos == start) return false;
        start += startLen;
        //    std::cout << "line='"  << line << "\n" << "start =" << start ;
        size_t end = a_line.find("</name>", start);
        if (std::string::npos == end) return false;
        a_schemaData.name = a_line.substr(start, end - start);
        //    std::cout << ", end=" << end << ", name='" << a_schemaData.schemaName << "'\n";
        file.close();
        return true;
    }   // ReadFileSchemaDataNBB()

}   // namespace import

int main()
{
    NEW_SCHEMA schema;
    std::string errorLine;
    bool bResult = import::ReadFileSchemaDataNBB("schema.txt", schema, errorLine);
    if (!bResult)
        std::cout << "error in line: '" << errorLine << "'\n";
    // Print the results
    std::cout << "\npairs: " << schema.pairs << ", tables: " << schema.tables << ", rounds: " << schema.rounds << ", sets: " << schema.sets << ", schemaType: " << schema.schemaType << '\n';
    std::cout << "schema name: '" << schema.name << "'\n" ;
    for (const auto& roundData : schema.tableData)
    {
        for (const auto& tableData : roundData)
        {
            std::cout << tableData.pairNS << "-" << tableData.pairEW << " " << tableData.set << " | ";
        }
        std::cout << std::endl;
    }
    return 0;
}   // main()
