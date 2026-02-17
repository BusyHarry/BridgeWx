// Copyright(c) 2026-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/wxcrtvararg.h>

#include "dbglobals.h"
#include "mylog.h"

namespace glb
{
    static wxChar theSeparator = '@';   // between the datasets
    wxChar GetSeparator()                   { return theSeparator; }
    void   SetSeparator(wxChar a_separator) { theSeparator = a_separator; }

    static char* TrimContract(char a_contract[])
    {   // remove spaces and '"' at end of input
        size_t len = strlen(a_contract);
        while ( len && (a_contract[len - 1] == ' ' || a_contract[len - 1] == '"') )
        {
            --len;
            a_contract[len] = 0;
        }
        return a_contract;
    }   // TrimContract()

    bool ScoresRead(vvScoreData& a_scoreData, CB_ScoreReadLine a_pFunReadLine, void* a_pUserData)
    {
        a_scoreData.clear();                    // remove old data
        a_scoreData.resize(cfg::MAX_GAMES + 1); //  and assure room voor all games. We depend on the entries of this vector!

        wxString scores, sGame;
        while ( a_pFunReadLine(sGame, scores, a_pUserData) )
        {
            UINT game = wxAtoi(sGame);
            if ( game > cfg::MAX_GAMES ) { MyLogError(_("Reading scores: gamenr <%s> too high!"), sGame); continue; }
            auto splitValues = wxSplit(scores, theSeparator);
            std::vector<score::GameSetData> gameData;
            for ( const auto& it : splitValues )
            {
                score::GameSetData setData;     // ensure that contracts are cleared!
                char nsScore[10]    = { 0 };    // {1,2,'score','score'} or {1,2,'score','score',"nsContract","ewContract"}
                char ewScore[10]    = { 0 };
                char nsContract[20] = { 0 };
                char ewContract[20] = { 0 };

                // OK, so sscanf fails if the string to read is empty! Solution: read string INCLUSIVE closing quote!
                auto count = wxSscanf(it, " { %u , %u, %9[^, ] , %9[^} ,] , \"%19[^,], \"%19[^}]}"
                    , &setData.pairNS, &setData.pairEW, nsScore, ewScore, nsContract, ewContract);
                if ( count == 6 ) // we have also read two biddings
                {   // remove possible spaces and '"' at end of input
                    setData.contractNS = TrimContract(nsContract);
                    setData.contractEW = TrimContract(ewContract);
                }
                if ( count != 4 && count != 6 )
                {
                    MyLogError(_("Reading scores: game %u: <%s> invalid!"), game, it);
                    continue;
                }
                setData.scoreNS = score::ScoreFromString(nsScore);
                setData.scoreEW = score::ScoreFromString(ewScore);
                gameData.push_back(setData);
            }
            a_scoreData[game] = gameData;
        }
        return true;
    } // ScoresRead()

    bool ScoresWrite(const vvScoreData& a_scoreData, CB_ScoreWriteLine a_pFunWriteLine, void* a_pUserData)
    {
        bool bResult = true;
        UINT game    = 0;
        for ( const auto& it : a_scoreData )
        {
            if ( game && it.size() )
            {   // only games with data
                wxString theScores;
                wxChar separator = ' ';
                for ( auto score : it )
                {
                    wxString contracts;
                    if ( !score.contractNS.IsEmpty() || !score.contractEW.IsEmpty() )
                        contracts = FMT(",\"%s\",\"%s\"", score.contractNS, score.contractEW);
                    theScores += FMT("%c{%u,%u,%s,%s%s}", separator, score.pairNS, score.pairEW,
                        score::ScoreToString(score.scoreNS), score::ScoreToString(score.scoreEW), contracts);
                    separator = theSeparator;
                }
                if ( !a_pFunWriteLine(game, theScores, a_pUserData) )
                    bResult = false;
            }
            ++game;
        }

        return bResult;
    } // ScoresWrite()

    bool CorrectionsEndRead(cor::mCorrectionsEnd& a_mCorrectionsEnd, bool a_bEdit, const wxString& a_data)
    {
        if ( a_data.IsEmpty() ) return true;    // no results, but no error!
        auto split = wxSplit(a_data, theSeparator);
        bool bOk   = true;
        for ( const auto& it : split )
        {   // {<global pairnr>,<score.2>,<bonus.2>,<games>}
            cor::CORRECTION_END ce;
            char scoreBuf[10 + 1] = { 0 };
            char bonusBuf[10 + 1] = { 0 };
            UINT pairNr;
            bool bItemError = (4 != wxSscanf(it, " {%u , %10[^, ] , %10[^, ] ,%u }", &pairNr, scoreBuf, bonusBuf, &ce.games));
            (void)bItemError;
            ce.score = Fdp(scoreBuf);
            ce.bonus = Fdp(bonusBuf);
            //        if ( cor::IsValidCorrectionEnd(pairNr, ce, it, bItemError) )   // let application handle this
            {   // only add result if editing (map == empty) or when pair exists in supplied map
                if ( a_bEdit || a_mCorrectionsEnd.find(pairNr) != a_mCorrectionsEnd.end() )
                {
                    if ( !a_bEdit && ce.score == SCORE_IGNORE )
                    {   // keep original score and nr of played games if bonus present
                        ////ce.score = a_mCorrectionsEnd[pairNr].score;
                        ////ce.games = a_mCorrectionsEnd[pairNr].games;
                        a_mCorrectionsEnd[pairNr].bonus = ce.bonus;
                    }
                    else
                        a_mCorrectionsEnd[pairNr] = ce;
                }
            }
            //        else bOk = false; // let application handle this
        }
        return bOk;
    }   // CorrectionsEndRead()

    wxString CorrectionsEndWrite(const cor::mCorrectionsEnd& a_mCorrectionsEnd)
    {
        wxChar   separator = ' ';
        wxString correction;
        for ( const auto& it : a_mCorrectionsEnd )
        {
            correction += FMT("%c{%u,%s,%s,%u}"
                , separator
                , it.first
                , it.second.score.AsString2F()
                , it.second.bonus.AsString2F()
                , it.second.games
            );
            separator = theSeparator;
        }
        return correction;
    }   // CorrectionsEndWrite()

    bool CorrectionsSessionRead(cor::mCorrectionsSession& a_mCorrectionsSession, const wxString& a_corrections)
    {
        if ( a_corrections.IsEmpty() ) return true;
        bool bResult = true;    // assume all is ok
        auto split   = wxSplit(a_corrections, theSeparator);
        for ( const auto& it : split )
        {
            UINT sessionPairnr;
            cor::CORRECTION_SESSION cs;
            char extraBuf[10 + 1] = { 0 };
            // {<session pairnr>,<correction><type>,<extra.1>,<max extra>}
            auto entries = wxSscanf(it, " {%u ,%i %c , %10[^, ] ,%i, %u }"
                , &sessionPairnr, &cs.correction, &cs.type, extraBuf, &cs.maxExtra, &cs.games
            );  // older db may NOT have games component
            bool bErrorEntry = entries < 5; (void)bErrorEntry;
            cs.extra = Fdp(extraBuf);
#if 0   // don't remove (partly) bad input, application should do it
            if ( !IsValidCorrectionSession(sessionPairnr, cs, it, bErrorEntry) )
            {
                bResult = false;
            }
            else
#endif
            {   // add info to map
                a_mCorrectionsSession[sessionPairnr] = cs;
            }
        }
        return bResult;
    }   // CorrectionsSessionRead()

    wxString CorrectionsSessionWrite(const cor::mCorrectionsSession& a_mCorrectionsSession)
    {
        wxChar   separator = ' ';
        wxString correction;
        for ( const auto& it : a_mCorrectionsSession )
        {
            correction += FMT("%c{%u,%+i%c,%s,%i,%u}"
                , separator
                , it.first
                , it.second.correction
                , it.second.type
                , it.second.extra.AsString1()
                , it.second.maxExtra
                , it.second.games
            );
            separator = theSeparator;
        }
        return correction;
    }   // CorrectionsSessionWrite()

    wxString GetDefaultSchema()
    {
        return FMT("{24,4,1}%c{14,0,\"6multi14\",\"\"}", theSeparator);
    }   // GetDefaultSchema()

    bool SchemaRead(cfg::SessionInfo& a_sessionInfo, const wxString& a_schema)
    {
        wxString info         = a_schema;
        wxString defaultValue = GetDefaultSchema();
        if ( info.IsEmpty() ) info = defaultValue;
        auto count = wxSscanf(info, " {%u ,%u ,%u }", &a_sessionInfo.nrOfGames, &a_sessionInfo.setSize, &a_sessionInfo.firstGame);
        if ( count != 3 )
        {   // on error, we just take a default value
            MyLogError(_("Error while reading schema <%s>"), info);
            info = defaultValue;
            a_sessionInfo.nrOfGames = 24;
            a_sessionInfo.setSize   = 4;
            a_sessionInfo.firstGame = 1;
        }
        auto split = wxSplit(info, theSeparator);
        split.erase(split.begin()); // now we have only schema descriptions
        a_sessionInfo.groupData.clear();
        cfg::GROUP_DATA groupData;
        UINT groupOffset = 0;
        for ( const auto& it : split )
        {
            char schema[20]     = { 0 };
            char groupChars[20] = { 0 };
            groupData.pairs     = 0;
            groupData.absent    = 0;
            count               = wxSscanf(it, " {%u ,%u , \"%19[^\"]\" , \"%19[^\"]\" }", &groupData.pairs, &groupData.absent, schema, groupChars);
            // count == 2 -> empty schema
            // count == 3 -> empty groupchars
            if ( count == 2 )
            {   // no schema, but perhaps groupchars
                count = wxSscanf(it, " {%u ,%u , \"\" , \"%19[^\"]\" }", &groupData.pairs, &groupData.absent, groupChars);
            }
            if ( count < 3 )
            {   // schema and groupchars empty
                MyLogError(_("Error while reading schema <%s>"), info);
            }
            if ( (groupChars[0] == ' ') && (groupChars[1] == 0) ) groupChars[0] = 0; // remove single space
            groupData.schema      = schema;
            groupData.groupChars  = groupChars;
            groupData.schemaId    = schema::GetId(groupData.schema);
            groupData.groupOffset = groupOffset;
            groupOffset          += groupData.pairs;
            a_sessionInfo.groupData.push_back(groupData);
        }
        return true;
    }  // SchemaRead()

    wxString SchemaWrite(const cfg::SessionInfo& a_info)
    {
        wxString schema = FMT("{%u,%u,%u}", a_info.nrOfGames, a_info.setSize, a_info.firstGame);
        for ( const auto& it : a_info.groupData )
        {
            wxString groupChars = it.groupChars;
            schema += FMT("%c{%u,%u,\"%s\",\"%s\"}", theSeparator, it.pairs, it.absent, it.schema, groupChars);
        }

        return schema;
    }   // SqlSchemaWrite()

    bool SessionNamesRead(wxArrayString& a_assignmentsName, const wxString& a_info)
    {
        a_assignmentsName = wxSplit(a_info, theSeparator);
        a_assignmentsName.insert(a_assignmentsName.begin(), names::GetNotSet());
        a_assignmentsName.resize(cfg::MAX_PAIRS + 1, names::GetNotSet());
        a_assignmentsName[1].Trim(TRIM_LEFT);   // first name has a ' ' in front of it
        return true;
    }   // SessionNamesRead()

    wxString SessionNamesWrite(const wxArrayString& a_names)
    {   // sessionnames for globalpairnrs of a session
        size_t max;
        for ( max = a_names.size() - 1; max > 0; --max )
        {   // no const reverse-iterator for wxArrayString....
            if ( a_names[max] != names::GetNotSet() ) break;    // found highest present pair
        }
        wxString info;
        wxChar separator = ' ';
        for ( size_t pair = 1; pair <= max; ++pair )
        {
            info += FMT("%c%s", separator, a_names[pair]);
            separator = theSeparator;
        }
        return info;
    }   // SessionNamesWrite()

    bool SessionResultRead(cor::mCorrectionsEnd& a_mSessionResult, const wxString& a_info)    // write and read: different params!
    {   // NB input MAP is initialized, so don't clear or resize it or add 'new' pairnrs!
        if ( a_info.IsEmpty() ) return true;    // no results, but no error!
        bool bResult = true;    // assume all is ok
        auto split   = wxSplit(a_info, theSeparator);
        for ( const auto& it : split )
        {   // {<global pairnr>,<score.2>,<games>}
            cor::CORRECTION_END ce;
            char scoreBuf[10 + 1] = { 0 };
            UINT pairNr;
            bool bItemError = (3 != wxSscanf(it, " {%u , %10[^, ] , %u }", &pairNr, scoreBuf, &ce.games));
            ce.score = Fdp(scoreBuf);
            if ( a_mSessionResult.find(pairNr) == a_mSessionResult.end() )
                bItemError = true;  // pair MUST be present!
            if ( cor::IsValidCorrectionEnd(pairNr, ce, it, bItemError) )
            {
                a_mSessionResult[pairNr] = ce;
            }
            else bResult = false;
        }

        return bResult;
    }   //SessionResultRead()

    wxString SessionResultWrite(const cor::mCorrectionsEnd& a_mSessionResult)
    {
        wxString result;
        wxChar   separator = ' ';
        for ( const auto& it : a_mSessionResult )   // save score of all pairs
        {
            if ( it.second.games == 0 ) continue;   // only present on conversion old --> db
            result += FMT("%c{%u,%s,%u}"
                , separator
                , it.first                          // global pairnr
                , it.second.score.AsString2F()      // score
                , it.second.games                   // played games
            );
            separator = theSeparator;
        }

        return result;
    }   // SessionResultWrite()

    wxString UintVectorWrite(const UINT_VECTOR& a_vUint)
    {   // write contents of vector as UINT, ignoring entry 0
        wxString info;
        auto it = std::find_if(a_vUint.rbegin(), a_vUint.rend(), [](UINT pair){return pair != 0U ;} );
        int maxPair = a_vUint.rend() - it - 1;  // index of highest pair in session, -1 if none found

        wxChar separator = ' ';
        for ( int pair = 1; pair <= maxPair; ++pair)
        {
            info += FMT("%c%u", separator, a_vUint[pair]);
            separator = theSeparator;
        }
        return info;
    }   // UintVectorWrite()

    bool UintVectorRead(UINT_VECTOR& a_vUint, const wxString& a_info, const wxString& a_dbFile, const wxString& a_key, const wxString& a_errorMsg)
    {   // Resize vector to cfg::MAX_PAIRS and read a set of UINTs and put them in a vector.
        bool bOk = true;
        a_vUint.clear();
        a_vUint.resize(cfg::MAX_PAIRS+1ULL, 0);
        auto split = wxSplit(a_info, theSeparator);
        UINT entry = 0;  // entry zero is a  dummy
        UINT max   = names::GetNumberOfGlobalPairs();
        for (const auto& it : split)
        {
            if ( ++entry > cfg::MAX_PAIRS )
                break;
            UINT value = (UINT)wxAtoi(it);
            if ( value > max )
            {
                wxString err = FMT(_("Error in database <%s> value in key <%s>: <%i> higher then max <%u>\nWill be ignored.")
                    , a_dbFile, a_key
                    , (int)value, max);
                MyLogError("%s", err);
                bOk = false;
                continue;
            }
            a_vUint[entry] = value;
        }
        if ( !bOk )
            MyMessageBox(a_errorMsg);
        return bOk;
    }   //UintVectorRead()

}   // namespace glb
