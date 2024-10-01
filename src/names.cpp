// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/ffile.h>
#include <wx/textfile.h>
#include <wx/wxcrtvararg.h>
#include <wx/msgdlg.h>

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>

#include <numeric>

#include "cfg.h"
#include "names.h"
#include "fileIo.h"
#include "database.h"

namespace names
{
static const wxString           ssNotSet = "???";
static PairInfoData             svGlobalPairInfo;           // entry 0 is a dummy!
static std::vector<wxString>    svClubNames;                // entry 0 is a dummy!
static UINT_VECTOR              svuPairnrGlobal2Session;    // entry 0 is a dummy! [x] gives sessionpairnr for global pair x
static UINT_VECTOR              svuPairnrSession2Global;    // entry 0 is a dummy! [x] gives globalpairnr for session pair x

//static UINT16 saiPairIndices[cfg::MAX_PAIRS+1]; // index to real pairnames for active match

static bool sbChangedPairNames = false;
static bool sbChangedClubNames = false;
static UINT suNrOfClubs;

static void SessionNamesWrite();            // write string's like "AA13"
static bool WriteSession2GlobalIds();       // write binairy's like 13
static void ReadSession2GlobalIds();
static void ReadClubNames();
static void ReadPairNames();

//auto convert = wxConvAuto(wxFONTENCODING_CP437);  // doesnot work!
auto convert = wxCSConv(wxFONTENCODING_CP437);      // this one does!

#ifdef _WIN32       // for VS (32/64 bit) we need 1 byte packing for the storage to be compatible
#pragma pack(1)
#endif
constexpr UINT AP_CURRENT                   = cfg::MAX_PAIRS;       // nr of pairs (120)
constexpr UINT NAME_LENGTH_CURRENT          = cfg::MAX_NAME_SIZE;   // pairnames max (30) chars
//constexpr auto FILE_LENGTH_NAMES_CURRENT  = 3993;                 // (AP_CURRENT+1)*((NAME_LENGTH_CURRENT+1)+2) = (120+1)*((30+1)+2)

typedef struct NAMES_CURRENT
{   char    name[NAME_LENGTH_CURRENT+1];
    INT16   clubindex;
} NAMES_CURRENT;                          //  newest format AP_CURRENT=120,NAME_LENGTH_CURRENT=30
#ifdef _WIN32
#pragma pack()
#endif

static PairInfoData             svPairInfoSaved;
static std::vector<wxString>    svClubNamesSaved;
static bool                     sbRestorepointMade = false;

bool ExistSessionPairWithClub()
{
    for (const auto sessionPair : svuPairnrSession2Global)
    {
        if (sessionPair == 0) continue;
        if (svGlobalPairInfo[sessionPair].clubIndex != 0)
            return true;    // at least one player associated with a club
    }

    return false;
}   // ExistSessionPairWithClub()

wxString PairnrGlobal2GlobalText(UINT a_globalPair)
{
    if (a_globalPair < svGlobalPairInfo.size() )
        return svGlobalPairInfo[a_globalPair].pairName;
    return ssNotSet;
}   // PairnrGlobal2GlobalText()

UINT GetNumberOfGlobalPairs()
{
    UINT size = svGlobalPairInfo.size();
    return size ? size - 1 : 0;
}   // GetNumberOfGlobalPairs()

const wxString GetNotSet()
{
    return ssNotSet;
}   // GetNotSet()

UINT PairnrGlobal2SessionPairnr(UINT a_globalPairNr)
{
    if (a_globalPairNr < svuPairnrGlobal2Session.size())
        return svuPairnrGlobal2Session[a_globalPairNr];
    return 0;
}   // PairnrGlobal2SessionPairnr()

UINT PairnrSession2GlobalPairnr(UINT a_sessionPairNr)
{
    if (a_sessionPairNr < svuPairnrSession2Global.size())
        return svuPairnrSession2Global[a_sessionPairNr];
    return 0;
}   // PairnrSession2GlobalPairnr()

static void XformPairnrFromSession2GlobleVicaVersa(const UINT_VECTOR& a_src, UINT_VECTOR& a_dst)
{
    size_t size = a_src.size();
    a_dst.clear();
    a_dst.resize(size, 0); // be sure destination atleast as big as source
    for (UINT pair = 1; pair < size; ++pair)
    {
        a_dst[a_src[pair]] = pair;  // should always fit!
    }
    if (size) a_dst[0] = 0; // for un-assigned pairs
}   // XformPairnrFromSession2GlobleVicaVersa()

void WriteAssignmentsToDisk(const UINT_VECTOR& a_newGlobalAssignments)
{
    if (svuPairnrGlobal2Session == a_newGlobalAssignments) return;

    (void)ConfigChanged(true);  // refresh config
    svuPairnrGlobal2Session = a_newGlobalAssignments;
    XformPairnrFromSession2GlobleVicaVersa(svuPairnrGlobal2Session, svuPairnrSession2Global);
    WriteSession2GlobalIds();
    SessionNamesWrite();
}   // WriteAssignmentsToDisk()

void SetRestorePoint()
{
    svPairInfoSaved     = svGlobalPairInfo;
    svClubNamesSaved    = svClubNames;
    sbRestorepointMade  = true;
}   // SetRestorePoint()

void GetRestorePoint()
{
    if (sbRestorepointMade)
    {
        sbRestorepointMade  = false;
        svGlobalPairInfo    = svPairInfoSaved;
        svClubNames         = svClubNamesSaved;
    }
}   // GetRestorePoint()

wxString PairnrSession2SessionText(UINT a_sessionPair)
{
    if (0 == a_sessionPair) return ssNotSet;

    auto pGrp = cfg::GetGroupData();

#if 0   // cppcheck tells me to use find_if(), but its not shorter and certainly not faster then a simple loop...
    auto it = std::find_if(pGrp->begin(), pGrp->end(),
        [sessionPair](/*const cfg::GROUP_DATA&*/ auto pData) -> bool { return pData.groupOffset + pData.pairs >= sessionPair; });
    if (it != pGrp->end())
    {
        wxString result = FMT("%s%u", it->groupChars.c_str(), sessionPair - it->groupOffset);
        return result;
    }
#else
    for (const auto& it : *pGrp)
    {
        if ( it.groupOffset+it.pairs >= a_sessionPair)
        {   // found group
            if (it.groupOffset + it.absent == a_sessionPair)
                return ssNotSet;
            wxString result = FMT("%s%u", it.groupChars.c_str(), a_sessionPair - it.groupOffset);
            return result;
        }
    }
#endif    
    return ssNotSet;  //?????
}   // PairnrSession2SessionText()

wxString PairnrGlobal2SessionText(UINT a_globalNr, const std::vector<unsigned int>* a_table)
{
    if (a_table == nullptr) a_table = &svuPairnrGlobal2Session;
    if ( (a_globalNr >= (*a_table).size()) || ((*a_table)[a_globalNr] == 0) )
        return ssNotSet;
    // we have an assigned pair
    UINT sessionPair = (*a_table)[a_globalNr];

    return PairnrSession2SessionText(sessionPair);
}   // PairnrGlobal2SessionText()

UINT PairnrSessionText2SessionPairnr(const wxString& a_sessionName)
{
    UINT sessionPair;
    wxString tmp = a_sessionName;
    (void)ValidateSessionPairName(tmp, sessionPair);
    return sessionPair;
}   // PairnrSessionText2SessionPairnr()

bool ValidateSessionPairName(wxString& a_sessionName, UINT& a_sessionPair)
{   // input: [AA]nr  : optional group chars and a pair number
    a_sessionPair = 0;
    a_sessionName.Trim(TRIM_LEFT); a_sessionName.Trim(TRIM_RIGHT);
    if (a_sessionName.IsEmpty())
    {
        a_sessionName = ssNotSet;
        return true;        // value cleared
    }
    a_sessionName.MakeUpper();
    UINT len = a_sessionName.Len();
    wxString groupChars;
    UINT pair = 0;

#if 0
    wxString grp;
    if (wxIsdigit(a_sessionName[0]))
    {   // no group chars
        bool bResult = a_sessionName.ToUInt(&pair);
    }
    else
    {
        char buf[10];
        int count =  wxSscanf(a_sessionName , "%9[A-Z]%d", buf, &pair);
        grp = buf;
    }
#endif

    UINT ii;
    for (ii = 0; ii<len; ++ii)
    {
        wxChar chr = a_sessionName[ii];
        if (!wxIsalpha(chr)) break;
        groupChars += chr;
    }
    for ( ; ii<len; ++ii)
    {
        wxChar chr = a_sessionName[ii];
        if (!wxIsdigit(chr)) return false;
        pair = pair*10+(chr-'0');
    }

    if (pair == 0) return false;

    // now check if it fits the groupinfo
    auto pGgrp = cfg::GetGroupData();
    if (groupChars.IsEmpty())
    {
        groupChars = (*pGgrp)[0].groupChars;
        a_sessionName = groupChars + a_sessionName;
    }
    for (const auto& it : *pGgrp )
    {
        if ( (it.groupChars == groupChars) && (it.pairs >= pair) )
        {
            a_sessionPair = it.groupOffset + pair;
            return true;
        }
    }

    return false;
}   // ValidateSessionPairName()

void ReadClubNames()
{
    io::ClubnamesRead(svClubNames,suNrOfClubs);
}   // readclubnames()

void ReadPairNames()
{
    sbChangedPairNames = false;
    if (!io::PairnamesRead(svGlobalPairInfo))
    {
        bool bOrg = io::DatabaseTypeGet() == io::DB_ORG;
        wxString msg = bOrg
            ? _("Probleem met paarnamenbestand: ") + cfg::ConstructFilename(cfg::EXT_NAMES)
            : FMT(_("Probleem met database '%s' bij lezen namen."), io::GetDbFileName());
        MyMessageBox(msg);
    }
}   //  ReadPairNames()

void WriteClubNames()
{
    if (!sbChangedClubNames) return;
    (void)ConfigChanged(true);  // refresh config
    sbChangedClubNames = false; // no changes anymore
    io::ClubnamesWrite(svClubNames);
}   // WriteClubNames()

void WritePairNames()
{
    sbRestorepointMade = false;
    if (!sbChangedPairNames) return;
    sbChangedPairNames = false;
    (void)ConfigChanged(true);  // refresh config
    io::PairnamesWrite(svGlobalPairInfo);
}   // WritePairNames()

const std::vector<PairInfo>& GetGlobalPairInfo()
{
    return svGlobalPairInfo;
}   // GetGlobalPairInfo()

PairInfo GetGlobalPairInfo(int a_index)
{
    if (a_index <= 0 || (static_cast<size_t>(a_index) >= svGlobalPairInfo.size()))
        return PairInfo (_("foute index"));

    return svGlobalPairInfo[a_index];
}   // GetGlobalPairInfo()

bool AddGlobalPairInfo(const PairInfo& a_info)
{
    if (svGlobalPairInfo.size() > cfg::MAX_PAIRS )
        return false;

    svGlobalPairInfo.push_back(a_info);   // add new data
    sbChangedPairNames = true;
    return true;
}   // AddGlobalPairInfo()

bool UpdatePairClubIndex(int a_pair, int a_club)
{
    if (a_pair < 1 || static_cast<size_t>(a_pair) >= svGlobalPairInfo.size() || a_club < 1 || static_cast<size_t>(a_club) >= svClubNames.size())
        return false;
    svGlobalPairInfo[a_pair].clubIndex = a_club;
    return true;
}   // UpdatePairClubIndex()

bool ChangePairInfo(const PairInfo& a_info, int a_index)
{
    if ( (a_index <= 0) || (static_cast<size_t>(a_index) >= svGlobalPairInfo.size()) || (svGlobalPairInfo[a_index] == a_info) )
        return false;

    svGlobalPairInfo[a_index] = a_info;
    sbChangedPairNames  = true;

    return true;
}   // ChangePairInfo()

bool ExistGlobalPairnr(int a_index)
{
    return (a_index > 0) && (static_cast<size_t>(a_index) < svGlobalPairInfo.size());
}   // ExistGlobalPairnr()

/******* club functions******/

wxString GetClubName(int a_index)
{   // 0 returns an empty string for conveniance...
    if ( (a_index < 0) || (static_cast<size_t>(a_index) >= svClubNames.size()))
        return _("foute index");

    return svClubNames[a_index];
}   // GetClubName()

int GetClubIndex(const wxString& a_clubName)
{
    for (size_t ii = 1;  ii < svClubNames.size(); ++ii)
    {
        if (0 == a_clubName.CmpNoCase(svClubNames[ii]))
        {
            return ii;
        }
    }

    return 0;
}   // GetClubIndex()

int AddClubName(const wxString& a_clubName)
{
    int index = GetFreeClubIndex();
    if (index)
    {
        svClubNames[index] = a_clubName;
        sbChangedClubNames = true;
    }

    return index;
}   // AddClubName()

bool SetClubName(const wxString& a_clubName, int a_index)
{
    if ( (a_index <= 0) || (static_cast<size_t>(a_index) >= svClubNames.size()) || (svClubNames[a_index] == a_clubName))
        return false;

    svClubNames[a_index] = a_clubName;
    sbChangedClubNames = true;
    return true;
}   // SetClubName()

bool ExistClub(int a_index)
{
    return     (a_index > 0)
            && (static_cast<size_t>(a_index) < svClubNames.size())
            && !svClubNames[a_index].empty();
}   // ExistClub()

bool ExistClub(const wxString& a_clubName, int* a_pIndex)
{
    bool  bFound =false;
    size_t  ii;

    for ( ii = 1;  ii < svClubNames.size(); ++ii)
    {
        if (0 == a_clubName.CmpNoCase(svClubNames[ii]))
        {
            bFound = true;
            break;
        }
    }

    if (a_pIndex) *a_pIndex = bFound ? ii : 0;
    return bFound;
}   // ExistClub()

int GetFreeClubIndex()
{
    for (size_t ii = cfg::MAX_CLUBID_UNION+1;  ii < svClubNames.size(); ++ii)
        if ( svClubNames[ii].IsEmpty() )
            return ii;
    return 0;
}   // GetFreeClubIndex()

UINT DetermineClubIndex(const wxString& a_club)
{
    if (a_club.empty()) return 0;

    UINT ii = 1;
    for (; ii <= suNrOfClubs; ++ii)
    {
        if (svClubNames[ii] == a_club)
            return ii;
    }

    if (suNrOfClubs >= cfg::MAX_CLUBNAMES )
        return 0;                       // no more room

    svClubNames[++suNrOfClubs] = a_club;
    return suNrOfClubs;              // return new index
}   // DetermineClubIndex()

bool RemoveClubIdFromPair(int a_index)
{
    if ( (a_index <= 0) || (static_cast<size_t>(a_index) >= svGlobalPairInfo.size()) )
        return false;

    svGlobalPairInfo[a_index].clubIndex = 0;
    sbChangedPairNames = true;
    return true;
}   // RemoveClubIdFromPair()

void SessionNamesWrite()
{
    (void)ConfigChanged(true);  // refresh config
    wxArrayString names;
    for (auto pair = 0; pair <= cfg::MAX_PAIRS; ++pair)
    {
        names.push_back(PairnrGlobal2SessionText(pair));
    }
    io::SessionNamesWrite(names, cfg::GetActiveSession());
}   // SessionNamesWrite()

void ReadSession2GlobalIds()
{
    // The file has the pairs based on sessionpairnr order, entry zero = dummy
    // pairIndex[x]: sessionPairNr 'x' is mapped to globalPairNr 'pairIndex[x]'
    // example: pairIndex[0,3,20,5,7] -> sessionpair 1 <==> globalpair 3, sp 4 <==> gp 7
    io::Session2GlobalIdsRead(svuPairnrSession2Global, cfg::GetActiveSession());
    XformPairnrFromSession2GlobleVicaVersa(svuPairnrSession2Global, svuPairnrGlobal2Session);
}   // ReadSession2GlobalIds()

bool WriteSession2GlobalIds()
{
    return io::Session2GlobalIdsWrite(svuPairnrSession2Global, cfg::GetActiveSession());
}   // WriteSession2GlobalIds()

 void GetSessionAssignmentsPrevious(wxArrayString& a_vPreviousAssignments)
{
     io::SessionNamesRead(a_vPreviousAssignments, cfg::GetActiveSession() - 1);
}   // GetSessionAssignmentsPrevious()

wxString PairnrSession2GlobalText(UINT a_sessionPair)
{
    if (a_sessionPair >= svuPairnrSession2Global.size()) return _("nog geen paarnamen toegekend");
    return svGlobalPairInfo[svuPairnrSession2Global[a_sessionPair]].pairName;
}   // PairnrSession2GlobalText()

void InitializePairNames()
{
    if (!ConfigChanged())
    {
        if (svGlobalPairInfo.size() == 0)
            MyMessageBox(_("InitializePairNames(): config not changed (yet) but sizeof(pairnames) == 0 ?????\n results in vector access errors"));
        return;
    }
    ReadClubNames();
    ReadPairNames();
    ReadSession2GlobalIds();
}   // InitializePairNames()

}   //  end namespace names
