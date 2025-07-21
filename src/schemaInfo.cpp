// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#define TESTING_IMPORT_EXPORT 0 /* if 1, we can test import/export schema's and create a 'new' NewSchemaData.cpp*/

#include <iostream>

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/translation.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>


#include "NewSchemaData.h"
#include "schemaInfo.h"
#include "MyLog.h"
#include "importExportSchema.h"
#include "baseframe.h"

#define   SET(round,table) (*m_pSchema).tableData[round][table].set

namespace schema
{
    const char* defaultSchema = "6multi14";
    const int   defaultId     = schema::GetId(defaultSchema);

    void FindSchema(UINT a_rounds, UINT a_pairs, INT_VECTOR& a_ids, wxArrayString* a_names)
    {
        a_ids.clear();
        if (a_names) a_names->clear();
        for (const auto& table : newSchemaTable)
        {
            if ((a_rounds == table.rounds) && (a_pairs == table.pairs))
            {
                auto id = &table - &newSchemaTable[0];  // index of schema
                a_ids.push_back(id);
                if (a_names) a_names->push_back(table.name);
            }
        }
    }   // FindSchema()

    int GetId(const wxString& a_name)
    {
        for (int ii = 0; ii < newSchemaTable.size(); ++ii)
        {
            if ( a_name == newSchemaTable[ii].name)
                return ii;
        }

        return ID_NONE;
    }   // GetId()

    wxString GetName(int a_id)
    {
        if (a_id >= 0 && a_id < newSchemaTable.size())
            return newSchemaTable[a_id].name;
        return wxEmptyString;
    }   // GetName()

    UINT GetMaxRound()
    {
        NewSchemaDataType maxRound = 0;
        for (const auto& table : newSchemaTable)
        {
            maxRound = std::max(maxRound, table.rounds);
        }

        return maxRound;
    }   // GetMaxRound()

    void GetSetInfo(int a_schemaId, UINT a_set, vGameInfo& a_info)
    {
        SchemaInfo si(a_schemaId);
        si.GetSetInfo(a_set, a_info);
    }   // GetSetInfo()

    void GetRoundInfo(int a_schemaId, UINT a_round, bool a_bGameOrder, vGameInfo& a_info)
    {
        SchemaInfo si(a_schemaId);
        si.GetRoundInfo( a_round, a_bGameOrder, a_info);
    }   // GetRoundInfo()

    bool ImportSchema(const wxString& a_file, bool a_bPreloading)
    {
        if (a_bPreloading)
        {   // 20250602: default size = 161
            if (newSchemaTable.capacity() < 255)
                newSchemaTable.reserve(255);
        }

        std::string     errorLine;
        NEW_SCHEMA      importSchema;
        bool bResult = importExportSchema::ImportSchemaNBB(a_file.ToStdString(), importSchema, errorLine);
        if (!bResult)
        {
            wxString errorL(errorLine);
            MyLogError(_("Error reading schema '%s' in line: '%s'"), a_file, errorL);
            MyMessageBox(errorL, _("Error"));
            return false;
        }
        // All ok, transform importdata to local data
        // All tables/pairs/sets are within limits!
        // Now check if its a new schema or a replacement of an existing one.
        auto dup = std::find(newSchemaTable.begin(), newSchemaTable.end(), importSchema);
        if (dup != newSchemaTable.end())
            *dup = importSchema;                    // update it
        else
        {   // add new schema
            if (a_bPreloading || newSchemaTable.size() < newSchemaTable.capacity())
                newSchemaTable.push_back(importSchema); // add the new schema
            else
            {   // INCREASE the reserve-value at the start of this function
                // On re-allocation we get exceptions if we have active schema-pointers
                wxString msg = _("Schematable is full, can't add new schemas");
                MyLogError("%s", msg);
                MyMessageBox(msg, _("Warning"));
            }
        }
        return true;
    }   // ImportSchema()

    bool ExportSchema(const wxString& a_schema, const wxString& a_file)
    {
        int id = GetId(a_schema);
        if (id == ID_NONE) return false;
        wxString fileName(a_file);
        if (fileName.empty())
        {
            wxDirDialog dlg(nullptr, FMT("%s '%s.asc'", _("Select a folder to store schema"), a_schema));
            if (wxID_OK != dlg.ShowModal()) return false;
            wxFileName name(dlg.GetPath(), a_schema, "asc");
            fileName = name.GetFullPath();
        }
        return importExportSchema::ExportSchemaNBB(fileName.ToStdString(), newSchemaTable[id]);
    }   // ExportSchema()

}   // end namespace schema

SchemaInfo::SchemaInfo()
{
    SetId(schema::ID_NONE);
}   // SchemaInfo()

bool SchemaInfo::SetId(int a_schemaId)
{
    m_tableSize = newSchemaTable.size();
    m_id = a_schemaId;  // limit checked in Init()
    Init();
    return m_bSchemaInitOk;
}   // SetId()

bool SchemaInfo::GetTableRoundInfo(UINT a_table, UINT a_round, schema::GameInfo& info) const
{
    if (!IsOk()) return false;
    assert(a_round > 0 && a_round <= m_rounds);
    assert(a_table > 0 && a_table <= m_tables);

    bool                bResult = true;
    NewSchemaDataType   set;
    set = SET(a_round, a_table);
    info.round  = a_round;
    info.set    = set;
    info.pairs  = GetPairs(a_round, a_table);
    if (info.pairs.ns == 0)   // 0: table not played this round
        bResult = false;

    return bResult;
}   // GetTableRoundInfo()

void SchemaInfo::GetRoundInfo( UINT a_round, bool a_bGameOrder,  schema::vGameInfo& a_info)
{
    a_info.clear();
    if (!IsOk()) return;    // no valid schema
    assert(a_round > 0 && a_round <= m_rounds);
    schema::GameInfo gi;
    for (UINT table = 1; table <= m_tables; ++table)
    {
        if (GetTableRoundInfo(table, a_round, gi))
            a_info.push_back(gi);
    }

    //now sort the data
    // here in the lambda its easier to sort also on NS...
    std::sort(a_info.begin(), a_info.end(),
        [a_bGameOrder] (schema::GameInfo const& left,schema:: GameInfo const& right)
        {
            if (a_bGameOrder)
            {
                if (left.set != right.set) return left.set < right.set;
            }

            return left.pairs.ns < right.pairs.ns;
        }
    );
}   // GetRoundInfo()

void SchemaInfo::GetSetInfo(UINT a_set, schema::vGameInfo& a_info) const
{
    a_info.clear();
    NewSchemaDataType theSet = static_cast<NewSchemaDataType> (a_set);
    for (UINT round = 1; round <= m_rounds; ++round)
    {
        for (UINT table = 1; table <= m_tables; ++table)
        {
            NewSchemaDataType  set = SET(round, table);
            if (set == theSet)
            {   // found requested set, store data
                schema::GameInfo gi;
                gi.round        = round;
                gi.set          = set;
                gi.pairs        = GetPairs(round, table);
                if (gi.pairs.ns != 0)   // 0: table not played this round
                    a_info.push_back(gi);
            }
        }
    }
    // now sort on NS
    std::sort(a_info.begin(), a_info.end());
}   // GetSetInfo()

SchemaInfo::SchemaInfo(int a_schemaId)
{
    SetId(a_schemaId);
}   // SchemaInfo()

SchemaInfo::SchemaInfo(const wxString& name)
{
    SetId(schema::GetId(name));
}   // SchemaInfo()

void SchemaInfo::Init()
{
    if ((m_id < 0) || (m_id >= m_tableSize))
    {
        m_rounds        = m_tables = m_pairs = 0;
        m_bSchemaInitOk = false;
        m_name          = "???";
        m_pSchema       = nullptr;
    }
    else
    {
        m_pSchema       = &newSchemaTable[m_id];
        m_rounds        = m_pSchema->rounds;
        m_pairs         = m_pSchema->pairs;
        m_tables        = m_pSchema->tables;  
        m_name          = m_pSchema->name;
        m_bSchemaInitOk = true;
    }
}   // Init()

schema::NS_EW SchemaInfo::GetPairs(UINT a_round, UINT a_table) const
{
    schema::NS_EW pairs;

    assert (a_round && a_round <= m_rounds);
    assert (a_table && a_table <= m_tables);
    pairs.ns = m_pSchema->tableData[a_round][a_table].pairNS;
    pairs.ew = m_pSchema->tableData[a_round][a_table].pairEW;
    return pairs;
}   // GetPairs()

UINT SchemaInfo::GetSet( UINT table, UINT round) const
{
    if ( (table == 0) || !IsOk() ) return 0;     // no schema or rest-table at uneven number of pairs!!
 
    assert(table <= m_tables);
    assert(round && round <= m_rounds);
    return m_pSchema->tableData[round][table].set;
}   // GetSet()

UINT SchemaInfo::GetBorrowTable(UINT table, UINT round) const
{
    if ( (table == 0) || !IsOk() ) return 0;    // no schema or no play at when table == 0
    assert(         table <= m_tables);
    assert(round && round <= m_rounds);

    return m_pSchema->tableData[round][table].setFromTable;
}   // GetBorrowTable()

UINT SchemaInfo::GetTable( UINT pair, UINT round) const
{
    if (!IsOk()) return 0;
    assert(pair  && pair  <= m_pairs );
    assert(round && round <= m_rounds);

    auto roundInfo = m_pSchema->tableData[round];
    for (UINT table = 1; table <= m_tables; ++table)
    {
        if (roundInfo[table].pairNS == pair || roundInfo[table].pairEW == pair)
            return table;
    }
    return 0;
}   // GetTable()

bool SchemaInfo::IsNs( UINT pair, UINT round) const
{
    if (!IsOk()) return false;
    assert(pair  && pair  <= m_pairs );
    assert(round && round <= m_rounds);

    auto roundInfo = m_pSchema->tableData[round];
//    return std::any_of(roundInfo.begin(), roundInfo.end(), [pair](const auto& info) {return info.pairNS == pair;});
    for (const auto& gameInfo : roundInfo)
    {
        if (gameInfo.pairNS == pair)
            return true;
        if (gameInfo.pairEW == pair)
            return false;
    }
    return false;
}   // IsNs()

UINT SchemaInfo::GetOpponent(UINT pair, UINT round) const
{
    UINT opponent;
    UINT table = GetTable( pair, round);
    for (opponent=1; opponent <= m_pairs; ++opponent)
    {
        if (GetTable( opponent, round) == table)
        {
            if (opponent != pair)
                break;
        }
    }
    return opponent > m_pairs ? 0 : opponent;
}   // GetOpponent()

bool SchemaInfo::AreOpponents(UINT a_pair1, UINT a_pair2)
{
    for (UINT round = 1; round <= m_rounds; ++round)
    {
        if ( GetOpponent(a_pair1, round) == a_pair2)
            return true;
    }
    return false;
}   // AreOpponents()

#if TESTING_IMPORT_EXPORT
//// testing new schema setup
#include <fstream>
static const auto nl('\n');
void DoConvertActive2NewSchemaData(const SchemaInfo& oldSchema, NEW_SCHEMA& newSchema)
{
    UINT rounds = oldSchema.Rounds();
    UINT tables = oldSchema.Tables();
    UINT pairs  = oldSchema.Pairs();

    NewSchemaDataType sets = 0;
    const auto size1((size_t)1);
    newSchema.rounds = rounds;
    newSchema.pairs  = pairs;
    newSchema.tables = tables;
    newSchema.name   = oldSchema.GetName().ToStdString();
    newSchema.tableData.resize(rounds+size1);    // 1 based
    for (UINT round = 1; round <= rounds; ++round)
    {
        newSchema.tableData[round].resize(tables+size1); // 1 based
        for (UINT table = 1; table <= tables; ++table)
        {
            auto pairInfo = oldSchema.GetPairs(round, table);
            NewTableInfo tblInfo;
            tblInfo.pairNS      = pairInfo.ns;
            tblInfo.pairEW      = pairInfo.ew;
            tblInfo.set         = oldSchema.GetSet(table, round);
            tblInfo.setFromTable = oldSchema.GetBorrowTable(table, round);
            newSchema.tableData[round][table] = tblInfo;
            sets = std::max(sets, tblInfo.set);
        }   // tables
    }   // rounds
    newSchema.sets = sets;
}   // DoConvertActive2NewSchemaData()

static void CreateNewSchemaDataCppOld()
{
    std::filebuf fb;
    fb.open ("f:/NewSchemaDataOld.cpp",std::ios::out | std::ios::trunc);
    std::ostream fp(&fb);
    const unsigned char bom[] = {0xEF,0xBB,0xBF,0};
    fp << bom <<
        "// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.\n"
        "// Distributed under the MIT License (http://opensource.org/licenses/MIT)\n"
        "\n"
        "#include \"NewSchemaData.h\"\n";

    SchemaInfo oldSchema;
    std::vector<std::string> schemas;
    size_t tblSize = newSchemaTable.size();
    for (UINT index = 0; index < tblSize; ++index)
    {
        oldSchema.SetId(index);
        NEW_SCHEMA newSchema;
        DoConvertActive2NewSchemaData(oldSchema, newSchema);
        std::string sanitizedName = '_' + newSchema.name;
//        auto check = [](char chr) -> bool {return chr == ' ' || chr == '\'' || chr == '.' || chr == '-';};
//        std::replace_if(sanitizedName.begin(), sanitizedName.end(), check, '_');
        for (auto& chr : sanitizedName) { if (chr == ' ' || chr == '\'' || chr == '.' || chr == '-') chr = '_'; }
        schemas.push_back(sanitizedName);
        fp  << "\nstatic NEW_SCHEMA " << sanitizedName << "\n{ "
            << newSchema.pairs      <<  " /*pairs*/, "
            << newSchema.tables     <<  " /*tables*/, "
            << newSchema.rounds     <<  " /*rounds*/, "
            << newSchema.sets       <<  " /*sets*/, "
            << newSchema.schemaType <<  " /*schema type: 0=pair schema, 1=individual schema*/, \""
            << newSchema.name       <<  "\" /*schemaName*/,\n"
                                        "  {\n"
                                        "      {/*dummy round 0*/}          //NS,EW,SET,SetFromTable\n";
        for (UINT round = 1; round <= newSchema.rounds; ++round)
        {
            fp << FMT("    , {/*r%-2u*/ {/*dummy table 0*/}", round);
            for (UINT table = 1; table <= newSchema.tables; ++table)
            {
                fp << FMT( ", {%2u,%2u,%2u,%2u}"
                            , (UINT)newSchema.tableData[round][table].pairNS
                            , (UINT)newSchema.tableData[round][table].pairEW
                            , (UINT)newSchema.tableData[round][table].set
                            , (UINT)newSchema.tableData[round][table].setFromTable
                         );
            }   // tables
            fp << " }\n";
        }   // rounds
        fp << "  }\n};\n";
    }   // schema's

    // now create the vector of pointers to these schema's
    fp << "\nstd::vector<const NEW_SCHEMA*> newSchemaTableOld\n{\n";
    char comma{' '};
    for (const auto& schema : schemas)
    {
        fp << "    " << comma << " &" << schema << nl;
        comma = ',';
    }
    fp << "};\n";
}   // CreateNewSchemaDataCppOld()

static void CreateNewSchemaDataCpp()
{
    std::filebuf fb;
    fb.open ("f:/NewSchemaData.cpp",std::ios::out | std::ios::trunc);
    std::ostream fp(&fb);
    const unsigned char bom[] = {0xEF,0xBB,0xBF,0};
    fp << bom <<
        "// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.\n"
        "// Distributed under the MIT License (http://opensource.org/licenses/MIT)\n"
        "\n"
        "#include \"NewSchemaData.h\"\n";

    // now create the vector of schema's
    fp << "\nstd::vector<NEW_SCHEMA> newSchemaTable\n{\n";
    char comma{' '};

    SchemaInfo oldSchema;
    size_t tblSize = newSchemaTable.size();
    for (UINT index = 0; index < tblSize; ++index)
    {
        oldSchema.SetId(index);
        NEW_SCHEMA newSchema;
        DoConvertActive2NewSchemaData(oldSchema, newSchema);
        fp  << " " << comma << " { "
            << newSchema.pairs      <<  " /*pairs*/, "
            << newSchema.tables     <<  " /*tables*/, "
            << newSchema.rounds     <<  " /*rounds*/, "
            << newSchema.sets       <<  " /*sets*/, "
            << newSchema.schemaType <<  " /*schema type: 0=pair schema, 1=individual schema*/, \""
            << newSchema.name       <<  "\" /*schemaName*/,\n"
            "     {\n"
            "         {/*dummy round 0*/}          //NS,EW,SET,SetFromTable\n";
        for (UINT round = 1; round <= newSchema.rounds; ++round)
        {
            fp << FMT("       , {/*r%-2u*/ {/*dummy table 0*/}", round);
            for (UINT table = 1; table <= newSchema.tables; ++table)
            {
                fp << FMT( ", {%2u,%2u,%2u,%2u}"
                    , (UINT)newSchema.tableData[round][table].pairNS
                    , (UINT)newSchema.tableData[round][table].pairEW
                    , (UINT)newSchema.tableData[round][table].set
                    , (UINT)newSchema.tableData[round][table].setFromTable
                );
            }   // tables
            fp << "   }\n";
        }   // rounds
        fp << "     }\n   }\n";
        comma = ',';
    }   // schema's

    fp << "};\n";
}   // CreateNewSchemaDataCpp()

#endif //TESTING_IMPORT_EXPORT

namespace schema
{
    void DebuggingSchemaData()
    {   // for testing only
#if TESTING_IMPORT_EXPORT
        SchemaInfo oldSchema("5tin08");
        //    oldSchema.SetId(0);    // first schema
        NEW_SCHEMA newSchema;
        DoConvertActive2NewSchemaData(oldSchema, newSchema);
        importExportSchema::ExportSchemaNBB("f:/testschema1.asc", newSchema);
        importExportSchema::ExportSchemaNBB("f:/testschema2.asc", newSchemaTable[1]);
        CreateNewSchemaDataCpp();
        CreateNewSchemaDataCppOld();
#endif
    }   // DebuggingSchemaData()

}   // namespace schema
