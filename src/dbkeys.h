﻿// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _DBKEYS_H_
#define _DBKEYS_H_

#define DEFAULT_SESSION ((UINT)-1)  /*main/global entry*/

enum keyId
{
    KEY_DB_VERSION,

    KEY_PRG_VERSION,
    KEY_MATCH_MAX_ABSENT, 
    KEY_MATCH_VIDEO,
    KEY_MATCH_CMNT,
    KEY_MATCH_PRNT,
    KEY_MATCH_MAXMEAN,
    KEY_MATCH_CLOCK,
    KEY_MATCH_WEIGHTAVG,
    KEY_MATCH_LINESPP,
    KEY_MATCH_NEUBERG,
    KEY_MATCH_GRPRESULT,
    KEY_MATCH_MMCLUB,
    KEY_MATCH_FF,
    KEY_MATCH_GLOBALNAMES,
    KEY_MATCH_SESSION,
    KEY_MATCH_PAIRNAMES,
    KEY_MATCH_CLUBNAMES,
    KEY_MATCH_DISCR,
    KEY_MATCH_BUTLER,

    KEY_SESSION_DISCR,
    KEY_SESSION_SCHEMA,
    KEY_SESSION_GROUPLETTERS,
    KEY_SESSION_FIRSTGAME,
    KEY_SESSION_GAMERESULT,
    KEY_SESSION_ASSIGNMENTS,
    KEY_SESSION_ASSIGNMENTS_NAME,
    KEY_SESSION_CORRECTION,
    KEY_SESSION_CORRECTION_END,
    KEY_SESSION_RESULT,
    KEY_SESSION_RANK_SESSION,
    KEY_SESSION_RANK_TOTAL,
};

#endif
