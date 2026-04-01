// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _FILEIO_H_
#define _FILEIO_H_

#include "cfg.h"
#include "dbkeys.h"
#include "names.h"
#include "score.h"
#include "corrections.h"
#include "calcscore.h"

/*
* the EX_RESULT_* are result values for the *Ex method(s)
*/
static constexpr auto EX_RESULT_OK          =  0;   // all ok
static constexpr auto EX_RESULT_CURRENT     = -1;   // matchname is current match, can't use *Ex
static constexpr auto EX_RESULT_ERROR       = -2;   // some error
static constexpr auto EX_RESULT_NOT_EXIST   = -3;   // given matchname does not exist, won't create
static constexpr auto EX_RESULT_NO_IMPL     = -4;   // not implemented

namespace io
{
    enum GlbDbType
    {   //apply actions supplied type of configuration. Only useful for 'original type' datafiles
          DB_MAIN      = 1
        , DB_MATCH     = 2
        , DB_SESSION   = 4
        , DB_ALL       = (DB_MAIN | DB_MATCH | DB_SESSION)
    };

    enum ActiveDbType
    {   // mask
          DB_ORG        = 1     // 'old' interface with many separate files
        , DB_DATABASE   = 2     // new interface with all matchdata in 1 file
        , DB_SQLITE     = 4     // new interface with all matchdata in 1 file
//        , DB_BOTH       = (DB_ORG | DB_DATABASE)
    };

    enum ConvertFromTo
    {
          FromOldToDb
        , FromDbToOld
    };

    void ConvertDataBase(ConvertFromTo how);

    void DatabaseTypeSet(ActiveDbType type, bool bQuiet = false);
    ActiveDbType DatabaseTypeGet();

    #include "interfaces.h"

} // namespace io
#endif // _FILEIO_H_
