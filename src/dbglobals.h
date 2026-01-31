// Copyright(c) 2026-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _DBGLOBALS_H_
#define  _DBGLOBALS_H_
/*
* prototypes for functions to be used both for .db and .sqlite data storage
* remark: (most) data is stored as a string
* if you prefere finer datatypes for sqlite then don't use the globals,
* but implement read/write as you wish
*
 */
#include "fileio.h"
namespace glb
{
typedef bool (*CB_ScoreWriteLine)(UINT      game, const wxString& score, void* pUserData);
typedef bool (*CB_ScoreReadLine )(wxString& game,       wxString& score, void* pUserData);

wxChar   GetSeparator            ();                // get current separator for items in the db
void     SetSeparator            (wxChar separator);// set current separator for items in the db
wxString GetDefaultSchema        ();
bool     ScoresWrite             (const vvScoreData& scoreData, CB_ScoreWriteLine funWriteline, void* pUserData = nullptr);
bool     ScoresRead              (      vvScoreData& scoreData, CB_ScoreReadLine  funReadline , void* pUserData = nullptr);
bool     CorrectionsEndRead      (      cor::mCorrectionsEnd& mCorrectionsEnd, bool bEdit, const wxString& data);
wxString CorrectionsEndWrite     (const cor::mCorrectionsEnd& mCorrectionsEnd);
bool     CorrectionsSessionRead  (      cor::mCorrectionsSession& mCorrectionsSession, const wxString& corrections);
wxString CorrectionsSessionWrite (const cor::mCorrectionsSession& mCorrectionsSession);
bool     SchemaRead              (      cfg::SessionInfo& info, const wxString& schema);
wxString SchemaWrite             (const cfg::SessionInfo& info);
bool     SessionNamesRead        (      wxArrayString& names, const wxString& info);
wxString SessionNamesWrite       (const wxArrayString& names);
bool     SessionResultRead       (      cor::mCorrectionsEnd& mSessionResult, const wxString& info);
wxString SessionResultWrite      (const cor::mCorrectionsEnd& mSessionResult);
wxString UintVectorWrite         (const UINT_VECTOR& vUint);
bool     UintVectorRead          (      UINT_VECTOR& vUint, const wxString& info, const wxString& a_dbFile, const wxString& key, const wxString& errorMsg);

}   // namespace glb
#endif
