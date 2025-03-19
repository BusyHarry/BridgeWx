// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _SCHEMAINFO_H_
#define _SCHEMAINFO_H_

#include "utils.h"

#define INT_VECTOR std::vector<int>
class wxArrayString;
class wxString;

namespace schema
{
    constexpr auto      ID_NONE = -1;             // id for not found or not initialized
    extern const char*  defaultSchema;
    extern const int    defaultId;

    INT_VECTOR  FindSchema(UINT rounds, UINT pairs, wxArrayString* schemaNames = 0);// return a vector of id's (and names) for schema's matching rounds and pairs
    int         GetId(const wxString& schema);                                // return the id of the wanted schema, SCHEMA_ID_NONE if not found
    wxString    GetName(int id);                                              // get name on base of its id
    UINT        GetMaxRound();                                                // get the maximum nr of rounds of the available schemas
    bool        ImportSchema(const wxString& a_file, bool bDeleteDup=false);  // import a schema from a testfile
    void        DebuggingSchemaData();                                        // testing only

    struct NS_EW{ UINT ns = 0; UINT ew = 0;};
    struct GameInfo
    {
        UINT round=0; UINT set=0; NS_EW pairs;
            bool operator < (const GameInfo &rhs) const
            {
                if (set != rhs.set) return set < rhs.set;
                return pairs.ns < rhs.pairs.ns; 
            }
    };

    typedef std::vector<GameInfo> vGameInfo;

    void GetRoundInfo       (int schemaId, UINT round, bool bGameOrder, vGameInfo& info);
    void GetSetInfo         (int schemaId, UINT a_set, vGameInfo& a_info);
} // end namespace schema

class NEW_SCHEMA;
class SchemaInfo
{
public:
    explicit SchemaInfo(int id);  /* explicit*/
    explicit SchemaInfo( const wxString& name );
    SchemaInfo();
    ~SchemaInfo(){;}
    bool SetId              ( int schemaId );    // return ok(true) if valid id was  supplied
    void GetRoundInfo       ( UINT round, bool bGameOrder, schema::vGameInfo& info );
    void GetSetInfo         ( UINT set  , schema::vGameInfo& info )const;
    UINT GetSet             ( UINT table, UINT round )const;
    UINT GetTable           ( UINT pair , UINT round )const;
    UINT GetBorrowTable     ( UINT table, UINT round )const;
    bool IsNs               ( UINT pair , UINT round )const;
    UINT GetOpponent        ( UINT pair , UINT round ) const;
    UINT GetNumberOfPairs   ()const { return m_pairs;    } UINT Pairs () const{ return m_pairs;  }
    UINT GetNumberOfTables  ()const { return m_tables;   } UINT Tables() const{ return m_tables; }
    UINT GetNumberOfRounds  ()const { return m_rounds;   } UINT Rounds() const{ return m_rounds; }
    const wxString& GetName ()const { return m_name;     }
    bool IsOk               ()const { return m_bSchemaInitOk; }
    bool GetTableRoundInfo  (const UINT table, UINT round, schema::GameInfo& info) const;

    schema::NS_EW GetPairs  ( UINT round, UINT table)const;
private:
    void Init();

    int                 m_id;
    UINT                m_rounds;
    UINT                m_tables;
    UINT                m_pairs;
    bool                m_bSchemaInitOk;
    wxString            m_name;
    const NEW_SCHEMA*   m_pSchema;
    size_t              m_tableSize;
};

#endif
