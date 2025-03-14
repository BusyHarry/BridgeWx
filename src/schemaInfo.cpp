// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/string.h>
#include <wx/arrstr.h>

#include "schemaData.h"
#include "schemaInfo.h"
#include "MyLog.h"
#include "importExportSchema.h"

#define   SET(round,table) m_pSetData[((table)-1)*(m_rounds)+round-1]
#define TABLE(round,pair)  m_pPairData[((pair)-1)*(m_rounds)+round-1]

namespace schema
{
    const char* defaultSchema = "6multi14";
    const int   defaultId     = schema::GetId(defaultSchema);

    INT_VECTOR FindSchema(UINT a_rounds, UINT a_pairs, wxArrayString* a_names)
    {
        INT_VECTOR result;

        if (a_names) a_names->clear();
        for (int ii = 0; ii < schemaTable.size(); ++ii)
        {
            if ( (a_rounds == schemaTable[ii]->rounds) && (a_pairs == schemaTable[ii]->pairs) )
            {
                result.push_back(ii);
                if (a_names) a_names->push_back(schemaTable[ii]->name);
            }
        }

        return result;
    }

    int GetId(const wxString& a_name)
    {
        for (int ii = 0; ii < schemaTable.size(); ++ii)
        {
            if ( a_name == schemaTable[ii]->name)
                return ii;
        }

        return ID_NONE;
    }   // GetId()

    wxString GetName(int a_id)
    {
        if (a_id >= 0 && a_id < schemaTable.size())
            return schemaTable[a_id]->name;

        return wxEmptyString;
    }   // GetName()

    UINT GetMaxRound()
    {
        UINT maxRound = 0;
        for (int ii = 0; ii < schemaTable.size(); ++ii)
        {
            if ( maxRound < schemaTable[ii]->rounds )
            {
                maxRound = schemaTable[ii]->rounds;
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
    ~ImportCleanup(){for (auto it : m_toClean){delete[] it;}}
    void Add(char* pToClean){m_toClean.push_back(pToClean);}
private:
    std::vector<char*> m_toClean;
} myCleanup;

bool ImportSchema(const wxString& a_file, bool a_bDeleteDup)
{
    std::string     errorLine;
    import::Schema  importSchema;
    bool bResult = import::ReadFileSchemaDataNBB(a_file.ToStdString(), importSchema, errorLine);
    if (!bResult)
    {
        wxString errorL(errorLine);
        MyLogError("Error reading schema '%s' in line: '%s'", a_file, errorL);
//        MyMessageBox(errorL, _("Error"));
        return false;
    }
    // All ok, transform importdata to local data
    // All tables/pairs/sets are within limits!
    auto sSize= sizeof(SCHEMA_DATA) + sizeof(SchemaDataType)*importSchema.rounds*((size_t)importSchema.pairs+importSchema.tables);
    char* tmp = new char [sSize]();
    myCleanup.Add(tmp); // free data at program-exit
    SCHEMA_DATA* schema = reinterpret_cast<SCHEMA_DATA*>(tmp);
    schema->pairs = importSchema.pairs;
    auto m_rounds = schema->rounds = importSchema.rounds;
    auto tables   = schema->tables = importSchema.tables;
    // having trouble with unfreed memory when using std::string for a name...
    strncpy(schema->name, importSchema.schemaName.c_str(), SCHEMA_NAME_SIZE-1);
    schema->name[SCHEMA_NAME_SIZE-1] = 0;
    auto* m_pSetData    = schema->data;
    auto* m_pPairData   = m_pSetData + (size_t)m_rounds*tables;

    for ( UINT round = 1; round <= m_rounds; ++round)
    {   // round 0 is a dummy
        for (UINT table = 1; table <= tables; ++table)
        {   // pair 0 is a dummy
            const auto& info = importSchema.info[round][table];
//            auto setOffset          = ((table)-1)*(m_rounds)+round-1; setOffset;
//            auto tableNsOffset      = ((info.pairNS)-1)*(m_rounds)+round-1;tableNsOffset;
//            auto tableEwOffset      = ((info.pairEW)-1)*(m_rounds)+round-1;tableEwOffset;
            if (info.pairNS == 0)  continue;    // this table has no players this round
            SET  (round,table)      = info.set;
            TABLE(round,info.pairNS)= table;
            TABLE(round,info.pairEW)= -(int)table;
            // TODO: check if 'set' is in use on a lower table -> make it 'borrow from'
        }   // table
    }   // round

    if (a_bDeleteDup)
    {   // delete duplicates when importing manually
        auto duplicate = std::find_if(schemaTable.begin(), schemaTable.end(), [schema](const auto left) {return 0 == strcmp(left->name, schema->name);});
        if (duplicate != schemaTable.end())
            schemaTable.erase(duplicate);
    }
    schemaTable.push_back(schema);  // we have a new schema!
    return true;
}   // ImportSchema()

}   // end namespace schema

SchemaInfo::SchemaInfo()
{
    SetId(schema::ID_NONE);
}   // SchemaInfo()

bool SchemaInfo::SetId(int a_schemaId)
{
    m_id = a_schemaId;
    Init();
    return m_bSchemaInitOk;
}   // SetId()

bool SchemaInfo::GetTableRoundInfo(UINT a_table, UINT a_round, schema::GameInfo& info) const
{
    if (!IsOk()) return false;
    assert(a_round > 0 && a_round <= m_rounds);
    assert(a_table > 0 && a_table <= m_tables);

    bool         bResult = true;
    SchemaDataType  set;

    set = SET(a_round, a_table);
    if (set < 0) set = SET(a_round, -set);
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
    SchemaDataType theSet = static_cast<SchemaDataType> (a_set);
    for (UINT round = 1; round <= m_rounds; ++round)
    {
        for (UINT table = 1; table <= m_tables; ++table)
        {
            SchemaDataType  set = SET(round, table);
            if (set < 0) set = SET(round, -set);
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
    if ((m_id < 0) || (m_id >= schemaTable.size()))
    {
        m_rounds        = m_tables = m_pairs = 0;
        m_pSetData      = m_pPairData = nullptr;
        m_bSchemaInitOk = false;
        m_name          = "???";
    }
    else
    {
        m_rounds        = schemaTable[m_id]->rounds;
        m_pairs         = schemaTable[m_id]->pairs;
        m_tables        = schemaTable[m_id]->tables;  
        m_pSetData      = schemaTable[m_id]->data;
        m_pPairData     = m_pSetData + (size_t)m_rounds*m_tables;
        m_name          = schemaTable[m_id]->name;
        m_bSchemaInitOk = true;
    }
}   // Init()

schema::NS_EW SchemaInfo::GetPairs(UINT a_round, UINT a_table) const
{
    schema::NS_EW pairs;

    for (UINT pair = 1, count = 2; count && (pair <= m_pairs); ++pair)
    {
        auto table = TABLE(a_round,pair);
        if ( (UINT)abs(table) == a_table)
        {
            table > 0 ? pairs.ns = pair : pairs.ew = pair;
            --count;
        }
    }
    return pairs;
}   // GetPairs()

UINT SchemaInfo::GetSet( UINT table, UINT round) const
{
    if ( (table == 0) || !IsOk() ) return 0;     // no schema or rest-table at uneven number of pairs!!
 
    assert(table <= m_tables);
    assert(round && round <= m_rounds);
    int set = SET(round, table);
    if (set < 0 ) return (UINT)SET(round, -set);
    return (UINT)set;
}   // GetSet()

UINT SchemaInfo::GetBorrowTable(UINT table, UINT round) const
{
    if ( (table == 0) || !IsOk() ) return 0;    // no schema or no play at when table == 0
    assert(         table <= m_tables);
    assert(round && round <= m_rounds);

    int set = SET(round,table);
    if (set < 0 ) return (UINT)(-set);
    return 0;
}   // GetBorrowTable()

UINT SchemaInfo::GetTable( UINT pair, UINT round) const
{
    if (!IsOk()) return 0;
    assert(pair  && pair  <= m_pairs );
    assert(round && round <= m_rounds);

    return std::abs(TABLE(round, pair));
}   // GetTable()

bool SchemaInfo::IsNs( UINT pair, UINT round) const
{
    if (!IsOk()) return false;
    assert(pair  && pair  <= m_pairs );
    assert(round && round <= m_rounds);

    return TABLE(round, pair) > 0;
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
