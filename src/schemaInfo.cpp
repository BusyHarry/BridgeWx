// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#define TESTING_IMPORT_EXPORT 0 /* if 1, we can test import/export schema's and create a 'new' NewSchemaData.cpp*/

#include <iostream>

#include <wx/string.h>
#include <wx/arrstr.h>

#include "NewSchemaData.h"
#include "schemaInfo.h"
#include "MyLog.h"
#include "importExportSchema.h"

#define   SET(round,table) (*m_pSchema).tableData[round][table].set

namespace schema
{
    const char* defaultSchema = "6multi14";
    const int   defaultId     = schema::GetId(defaultSchema);

    INT_VECTOR FindSchema(UINT a_rounds, UINT a_pairs, wxArrayString* a_names)
    {
        INT_VECTOR result;

        if (a_names) a_names->clear();
        for (int ii = 0; ii < newSchemaTable.size(); ++ii)
        {
            if ((a_rounds == newSchemaTable[ii]->rounds) && (a_pairs == newSchemaTable[ii]->pairs))
            {
                result.push_back(ii);
                if (a_names) a_names->push_back(newSchemaTable[ii]->name);
            }
        }

        return result;
    }

    int GetId(const wxString& a_name)
    {
        for (int ii = 0; ii < newSchemaTable.size(); ++ii)
        {
            if ( a_name == newSchemaTable[ii]->name)
                return ii;
        }

        return ID_NONE;
    }   // GetId()

    wxString GetName(int a_id)
    {
        if (a_id >= 0 && a_id < newSchemaTable.size())
            return newSchemaTable[a_id]->name;
        return wxEmptyString;
    }   // GetName()

    UINT GetMaxRound()
    {
        UINT maxRound = 0;
        for (int ii = 0; ii < newSchemaTable.size(); ++ii)
        {
            if ( maxRound < newSchemaTable[ii]->rounds )
            {
                maxRound = newSchemaTable[ii]->rounds;
            }
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

static class ImportCleanup
{
public:
    ~ImportCleanup()
    {
        for (auto it : m_toClean)
        {
#ifdef _DEBUG
            std::cout << "destructing imported schema: <" << it->name << ">\n";
#endif
            delete it;
        }
    }
    void Add(NEW_SCHEMA* pToClean){m_toClean.push_back(pToClean);}
private:
    std::vector<NEW_SCHEMA*> m_toClean;
} myCleanup;

bool ImportSchema(const wxString& a_file, bool a_bDeleteDup)
{
    std::string     errorLine;
    NEW_SCHEMA*     pImportSchema = new NEW_SCHEMA;
    bool bResult = import::ReadFileSchemaDataNBB(a_file.ToStdString(), *pImportSchema, errorLine);
    if (!bResult)
    {
        wxString errorL(errorLine);
        MyLogError("Error reading schema '%s' in line: '%s'", a_file, errorL);
//        MyMessageBox(errorL, _("Error"));
        delete pImportSchema;
        return false;
    }
    // All ok, transform importdata to local data
    // All tables/pairs/sets are within limits!
    if (a_bDeleteDup)
    {   // delete duplicates when importing manually
        auto duplicate = std::find_if(newSchemaTable.begin(), newSchemaTable.end(), [pImportSchema](const auto left) {return left->name == pImportSchema->name;});
        if (duplicate != newSchemaTable.end())
            newSchemaTable.erase(duplicate);
    }
    newSchemaTable.push_back(pImportSchema);  // we have a new schema!
    myCleanup.Add(pImportSchema); // free data at program-exit
    return true;
}   // ImportSchema()

}   // end namespace schema

SchemaInfo::SchemaInfo()
{
    SetId(schema::ID_NONE);
}   // SchemaInfo()

bool SchemaInfo::SetId(int a_schemaId)
{
    m_tableSize = newSchemaTable.size();
    m_id = a_schemaId;
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
        m_pSchema       = newSchemaTable[m_id];
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

#if TESTING_IMPORT_EXPORT
//// testing new schema setup
#include <fstream>
static const auto nl('\n');
static void ExportSchemaNBB(const NEW_SCHEMA& schema, std::ostream& os)
{
    wxString tmp = FMT("%u %u %u %u %u\n",schema.pairs, schema.tables, schema.rounds, schema.sets, schema.schemaType);
    os << tmp;

    for (UINT round = 1; round <= schema.rounds; ++round)
    {
        for (UINT table = 1; table <= schema.tables; ++table)
        {
            tmp = FMT("%2u-%2u %u ", schema.tableData[round][table].pairNS, schema.tableData[round][table].pairEW, schema.tableData[round][table].set);
            os << tmp;
        }
        os << nl;
    }
    os << "#<name>" << schema.name << "</name>\n" << std::endl;
}   // ExportSchemaNbb()

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
    fp << "\nstd::vector<const NEW_SCHEMA*> newSchemaTable\n{\n";
    char comma{' '};
    for (const auto& schema : schemas)
    {
        fp << "    " << comma << " &" << schema << nl;
        comma = ',';
    }
    fp << "};\n";
}   // CreateNewSchemaDataCpp()

#endif //TESTING_IMPORT_EXPORT

namespace schema
{
    void DebuggingSchemaData()
    {   // for testing only
#if TESTING_IMPORT_EXPORT
        SchemaInfo oldSchema("mpx NBB '93");
        //    oldSchema.SetId(0);    // first schema
        NEW_SCHEMA newSchema;
        DoConvertActive2NewSchemaData(oldSchema, newSchema);

        std::filebuf fb;
        fb.open("f:/schemas.txt", std::ios::out);
        std::ostream fp(&fb);
        ExportSchemaNBB(newSchema, fp);
        ExportSchemaNBB(newSchema, std::cout);
        CreateNewSchemaDataCpp();

        auto schema0 = newSchemaTable[0]; schema0;
#endif
    }   // DebuggingSchemaData()

}   // namespace schema
