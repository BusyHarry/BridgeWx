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
#include "fileio.h"

namespace names
{
static const wxString           s_sNotSet = "???";
static PairInfoData             s_vGlobalPairInfo;          // entry 0 is a dummy!
static std::vector<wxString>    s_vClubNames;               // entry 0 is a dummy!
static UINT_VECTOR              s_vuPairnrGlobal2Session;   // entry 0 is a dummy! [x] gives sessionpairnr for global pair x
static UINT_VECTOR              s_vuPairnrSession2Global;   // entry 0 is a dummy! [x] gives globalpairnr for session pair x

const UINT_VECTOR* GetPairnrSession2Global() {return &s_vuPairnrSession2Global;}
//static UINT16 saiPairIndices[cfg::MAX_PAIRS+1]; // index to real pairnames for active match

static bool s_bChangedPairNames = false;
static bool s_bChangedClubNames = false;
static UINT s_uNrOfClubs;

static void SessionNamesWrite();            // write string's like "AA13"
static bool WriteSession2GlobalIds();       // write binairy's like 13
static void ReadSession2GlobalIds();
static void ReadClubNames();
static void ReadPairNames();

//auto s_funConvert = wxConvAuto(wxFONTENCODING_CP437);  // doesnot work!
static const auto s_funConvert = wxCSConv(wxFONTENCODING_CP437);      // this one does!

#ifdef _WIN32       // for VS (32/64 bit) we need 1 byte packing for the storage to be compatible
#pragma pack(1)
#endif
constexpr UINT AP_CURRENT                   = cfg::MAX_PAIRS;       // nr of pairs (120)
constexpr UINT NAME_LENGTH_CURRENT          = cfg::MAX_NAME_SIZE;   // pairnames max (30) chars
//constexpr auto FILE_LENGTH_NAMES_CURRENT  = 3993;                 // (AP_CURRENT+1)*((NAME_LENGTH_CURRENT+1)+2) = (120+1)*((30+1)+2)

using NAMES_CURRENT = struct NAMES_CURRENT
{   char    name[NAME_LENGTH_CURRENT+1];
    INT16   clubindex;
};                          //  newest format AP_CURRENT=120,NAME_LENGTH_CURRENT=30
#ifdef _WIN32
#pragma pack()
#endif

static PairInfoData             s_vPairInfoSaved;
static std::vector<wxString>    s_vClubNamesSaved;
static bool                     s_bRestorepointMade = false;

bool ExistSessionPairWithClub()
{
    for ( const auto sessionPair : s_vuPairnrSession2Global )
    {
        if ( sessionPair == 0 )
            continue;
        if ( s_vGlobalPairInfo[sessionPair].clubIndex != 0 )
            return true;    // at least one player associated with a club
    }

    return false;
}   // ExistSessionPairWithClub()

wxString PairnrGlobal2GlobalText(UINT a_globalPair)
{
    if ( a_globalPair < s_vGlobalPairInfo.size() )
        return s_vGlobalPairInfo[a_globalPair].pairName;
    return s_sNotSet;
}   // PairnrGlobal2GlobalText()

UINT GetNumberOfGlobalPairs()
{
    auto size = (UINT)s_vGlobalPairInfo.size();
    return size ? size - 1 : 0;
}   // GetNumberOfGlobalPairs()

wxString GetNotSet()
{
    return s_sNotSet;
}   // GetNotSet()

UINT PairnrGlobal2SessionPairnr(UINT a_globalPairNr)
{
    if ( a_globalPairNr < s_vuPairnrGlobal2Session.size() )
        return s_vuPairnrGlobal2Session[a_globalPairNr];
    return 0;
}   // PairnrGlobal2SessionPairnr()

UINT PairnrSession2GlobalPairnr(UINT a_sessionPairNr)
{
    if ( a_sessionPairNr < s_vuPairnrSession2Global.size() )
        return s_vuPairnrSession2Global[a_sessionPairNr];
    return 0;
}   // PairnrSession2GlobalPairnr()

static void XformPairnrFromSession2GlobleVicaVersa(const UINT_VECTOR& a_src, UINT_VECTOR& a_dst)
{
    size_t size = a_src.size();
    a_dst.clear();
    a_dst.resize(size, 0); // be sure destination atleast as big as source
    for ( UINT pair = 1; pair < size; ++pair )
    {
        a_dst[a_src[pair]] = pair;  // should always fit!
    }
    if ( size )
        a_dst[0] = 0;   // for un-assigned pairs
}   // XformPairnrFromSession2GlobleVicaVersa()

void WriteAssignmentsToDisk()
{
    (void)ConfigChanged(true);  // refresh config
    XformPairnrFromSession2GlobleVicaVersa(s_vuPairnrGlobal2Session, s_vuPairnrSession2Global);
    WriteSession2GlobalIds();
    SessionNamesWrite();
}   // WriteAssignmentsToDisk()

void WriteAssignmentsToDisk(const UINT_VECTOR& a_newGlobalAssignments)
{
    if ( s_vuPairnrGlobal2Session == a_newGlobalAssignments )
        return;
    s_vuPairnrGlobal2Session = a_newGlobalAssignments;
    WriteAssignmentsToDisk();
}   // WriteAssignmentsToDisk()

void SetRestorePoint()
{
    s_vPairInfoSaved     = s_vGlobalPairInfo;
    s_vClubNamesSaved    = s_vClubNames;
    s_bRestorepointMade  = true;
}   // SetRestorePoint()

void GetRestorePoint()
{
    if ( s_bRestorepointMade )
    {
        s_bRestorepointMade  = false;
        s_vGlobalPairInfo    = s_vPairInfoSaved;
        s_vClubNames         = s_vClubNamesSaved;
    }
}   // GetRestorePoint()

wxString PairnrSession2SessionText(UINT a_sessionPair, bool a_bForceAbsent)
{
    if ( 0 == a_sessionPair )
        return s_sNotSet;

    auto pGrp = cfg::GetGroupData();

#if 0   // cppcheck tells me to use find_if(), but its not shorter and certainly not faster then a simple loop...
    auto it = std::find_if(pGrp->begin(), pGrp->end(),
        [sessionPair](/*const cfg::GROUP_DATA&*/ auto pData) -> bool { return pData.groupOffset + pData.pairs >= sessionPair; });
    if ( it != pGrp->end() )
    {
        wxString result = FMT("%s%u", it->groupChars.c_str(), sessionPair - it->groupOffset);
        return result;
    }
#else
    for ( const auto& it : *pGrp )
    {
        if ( it.groupOffset+it.pairs >= a_sessionPair )
        {   // found group
            if ( !a_bForceAbsent && it.groupOffset + it.absent == a_sessionPair )
                return s_sNotSet;
            wxString result = FMT("%s%u", it.groupChars.c_str(), a_sessionPair - it.groupOffset);
            return result;
        }
    }
#endif
    return s_sNotSet;  //?????
}   // PairnrSession2SessionText()

wxString PairnrGlobal2SessionText(UINT a_globalNr, const std::vector<unsigned int>* a_table)
{
    if ( a_table == nullptr )
        a_table = &s_vuPairnrGlobal2Session;
    if ( (a_globalNr >= (*a_table).size()) || ((*a_table)[a_globalNr] == 0) )
        return s_sNotSet;
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
    if ( a_sessionName.IsEmpty() )
    {
        a_sessionName = s_sNotSet;
        return true;        // value cleared
    }
    a_sessionName.MakeUpper();
    auto len = (UINT)a_sessionName.Len();
    wxString groupChars;
    UINT pair = 0;

#if 0
    wxString grp;
    if ( wxIsdigit(a_sessionName[0]) )
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
    for ( ii = 0; ii<len; ++ii )
    {
        wxChar chr = a_sessionName[ii];
        if ( !wxIsalpha(chr) )
            break;
        groupChars += chr;
    }
    for ( ; ii<len; ++ii )
    {
        wxChar chr = a_sessionName[ii];
        if ( !wxIsdigit(chr) )
            return false;
        pair = pair*10+(chr-'0');
    }

    if ( pair == 0 )
        return false;

    // now check if it fits the groupinfo
    auto pGgrp = cfg::GetGroupData();
    if ( groupChars.IsEmpty() )
    {
        groupChars = (*pGgrp)[0].groupChars;
        a_sessionName = groupChars + a_sessionName;
    }
    for ( const auto& it : *pGgrp )
    {
        if ( (it.groupChars == groupChars) && (it.pairs >= pair) )
        {
            a_sessionPair = it.groupOffset + pair;
            return true;
        }
    }

    return false;
}   // ValidateSessionPairName()

static void ReadClubNames()
{
    io::ClubnamesRead(s_vClubNames,s_uNrOfClubs);
}   // readclubnames()

static void ReadPairNames()
{
    s_bChangedPairNames = false;
    if ( !io::PairnamesRead(s_vGlobalPairInfo) )
    {
        bool bOrg = io::DatabaseTypeGet() == io::DB_ORG;
        wxString msg = bOrg
            ? _("Problem with the pairname file: ") + cfg::ConstructFilename(cfg::EXT_NAMES)
            : FMT(_("Problem with database '%s' at reading names."), io::GetDbFileName());
        MyMessageBox(msg);
    }
}   //  ReadPairNames()

void WriteClubNames()
{
    if ( !s_bChangedClubNames )
        return;
    (void)ConfigChanged(true);  // refresh config
    s_bChangedClubNames = false; // no changes anymore
    io::ClubnamesWrite(s_vClubNames);
}   // WriteClubNames()

void WritePairNames()
{
    s_bRestorepointMade = false;
    if ( !s_bChangedPairNames )
        return;
    s_bChangedPairNames = false;
    (void)ConfigChanged(true);  // refresh config
    io::PairnamesWrite(s_vGlobalPairInfo);
}   // WritePairNames()

const std::vector<PairInfo>& GetGlobalPairInfo()
{
    return s_vGlobalPairInfo;
}   // GetGlobalPairInfo()

PairInfo GetGlobalPairInfo(int a_index)
{
    if ( a_index <= 0 || (static_cast<size_t>(a_index) >= s_vGlobalPairInfo.size()) )
        return PairInfo (_("bad index"));

    return s_vGlobalPairInfo[a_index];
}   // GetGlobalPairInfo()

bool AddGlobalPairInfo(const PairInfo& a_info)
{
    if ( s_vGlobalPairInfo.size() > cfg::MAX_PAIRS )
        return false;

    s_vGlobalPairInfo.push_back(a_info);   // add new data
    s_bChangedPairNames = true;
    return true;
}   // AddGlobalPairInfo()

bool UpdatePairClubIndex(int a_pair, int a_club)
{
    if ( a_pair < 1 || static_cast<size_t>(a_pair) >= s_vGlobalPairInfo.size() || a_club < 1 || static_cast<size_t>(a_club) >= s_vClubNames.size() )
        return false;
    s_vGlobalPairInfo[a_pair].clubIndex = a_club;
    return true;
}   // UpdatePairClubIndex()

bool ChangePairInfo(const PairInfo& a_info, int a_index)
{
    if ( (a_index <= 0) || (static_cast<size_t>(a_index) >= s_vGlobalPairInfo.size()) || (s_vGlobalPairInfo[a_index] == a_info) )
        return false;

    s_vGlobalPairInfo[a_index] = a_info;
    s_bChangedPairNames  = true;

    return true;
}   // ChangePairInfo()

bool ExistGlobalPairnr(int a_index)
{
    return (a_index > 0) && (static_cast<size_t>(a_index) < s_vGlobalPairInfo.size());
}   // ExistGlobalPairnr()

/******* club functions******/

wxString GetClubName(int a_index)
{   // 0 returns an empty string for conveniance...
    if ( (a_index < 0) || (static_cast<size_t>(a_index) >= s_vClubNames.size()) )
        return _("bad index");

    return s_vClubNames[a_index];
}   // GetClubName()

int GetClubIndex(const wxString& a_clubName)
{
    for ( size_t ii = 1;  ii < s_vClubNames.size(); ++ii )
    {
        if ( 0 == a_clubName.CmpNoCase(s_vClubNames[ii]) )
        {
            return (int)ii;
        }
    }

    return 0;
}   // GetClubIndex()

int AddClubName(const wxString& a_clubName)
{
    int index = GetFreeClubIndex();
    if ( index )
    {
        s_vClubNames[index] = a_clubName;
        s_bChangedClubNames = true;
    }

    return index;
}   // AddClubName()

bool SetClubName(const wxString& a_clubName, int a_index)
{
    if ( (a_index <= 0) || (static_cast<size_t>(a_index) >= s_vClubNames.size()) || (s_vClubNames[a_index] == a_clubName) )
        return false;

    s_vClubNames[a_index] = a_clubName;
    s_bChangedClubNames = true;
    return true;
}   // SetClubName()

bool ExistClub(int a_index)
{
    return     (a_index > 0)
            && (static_cast<size_t>(a_index) < s_vClubNames.size())
            && !s_vClubNames[a_index].IsEmpty();
}   // ExistClub()

bool ExistClub(const wxString& a_clubName, int* a_pIndex)
{
    bool  bFound =false;
    size_t  ii;

    for ( ii = 1;  ii < s_vClubNames.size(); ++ii )
    {
        if ( 0 == a_clubName.CmpNoCase(s_vClubNames[ii]) )
        {
            bFound = true;
            break;
        }
    }

    if ( a_pIndex )
        *a_pIndex = bFound ? ii : 0;
    return bFound;
}   // ExistClub()

int GetFreeClubIndex()
{
    for ( size_t ii = cfg::MAX_CLUBID_UNION+1;  ii < s_vClubNames.size(); ++ii )
        if ( s_vClubNames[ii].IsEmpty() )
            return ii;
    return 0;
}   // GetFreeClubIndex()

UINT DetermineClubIndex(const wxString& a_club)
{
    if ( a_club.IsEmpty() )
        return 0;

    UINT ii = 1;
    for ( ; ii <= s_uNrOfClubs; ++ii )
    {
        if ( s_vClubNames[ii] == a_club )
            return ii;
    }

    if ( s_uNrOfClubs >= cfg::MAX_CLUBNAMES )
        return 0;                       // no more room

    s_vClubNames[++s_uNrOfClubs] = a_club;
    return s_uNrOfClubs;              // return new index
}   // DetermineClubIndex()

bool RemoveClubIdFromPair(int a_index)
{
    if ( (a_index <= 0) || (static_cast<size_t>(a_index) >= s_vGlobalPairInfo.size()) )
        return false;

    s_vGlobalPairInfo[a_index].clubIndex = 0;
    s_bChangedPairNames = true;
    return true;
}   // RemoveClubIdFromPair()

static void SessionNamesWrite()
{
    (void)ConfigChanged(true);  // refresh config
    wxArrayString names;
    for ( auto pair = 0; pair <= cfg::MAX_PAIRS; ++pair )
    {
        names.push_back(PairnrGlobal2SessionText(pair));
    }
    io::SessionNamesWrite(names, cfg::GetActiveSession());
}   // SessionNamesWrite()

static void ReadSession2GlobalIds()
{
    // The file has the pairs based on sessionpairnr order, entry zero = dummy
    // pairIndex[x]: sessionPairNr 'x' is mapped to globalPairNr 'pairIndex[x]'
    // example: pairIndex[0,3,20,5,7] -> sessionpair 1 <==> globalpair 3, sp 4 <==> gp 7
    io::Session2GlobalIdsRead(s_vuPairnrSession2Global, cfg::GetActiveSession());
    XformPairnrFromSession2GlobleVicaVersa(s_vuPairnrSession2Global, s_vuPairnrGlobal2Session);
}   // ReadSession2GlobalIds()

static bool WriteSession2GlobalIds()
{
    return io::Session2GlobalIdsWrite(s_vuPairnrSession2Global, cfg::GetActiveSession());
}   // WriteSession2GlobalIds()

 void GetSessionAssignmentsPrevious(wxArrayString& a_vPreviousAssignments)
{
     io::SessionNamesRead(a_vPreviousAssignments, cfg::GetActiveSession() - 1);
}   // GetSessionAssignmentsPrevious()

wxString PairnrSession2GlobalText(UINT a_sessionPair)
{
    if ( a_sessionPair >= s_vuPairnrSession2Global.size() )
        return _("no names assigned yet");
    return s_vGlobalPairInfo[s_vuPairnrSession2Global[a_sessionPair]].pairName;
}   // PairnrSession2GlobalText()

void InitNames4Conversion(UINT a_session)
{
    ReadClubNames();
    ReadPairNames();
    io::Session2GlobalIdsRead(s_vuPairnrSession2Global, a_session);
    XformPairnrFromSession2GlobleVicaVersa(s_vuPairnrSession2Global, s_vuPairnrGlobal2Session);
}   // InitNames4Conversion()

void InitializePairNames()
{
    if ( !ConfigChanged() )
    {
        if ( s_vGlobalPairInfo.size() == 0 )
            MyMessageBox(_("InitializePairNames(): config not changed (yet) but sizeof(pairnames) == 0 ?????\n results in vector access errors"));
        return;
    }
    ReadClubNames();
    ReadPairNames();
    ReadSession2GlobalIds();
}   // InitializePairNames()

bool AdjustAssignments(UINT a_fromPair, int a_delta)
{
    bool bChange = false;
    for ( UINT pair = 1; pair < s_vuPairnrGlobal2Session.size(); ++pair )
    {
        UINT sessionPair = s_vuPairnrGlobal2Session[pair];
        if ( sessionPair >= a_fromPair )
        {
            s_vuPairnrGlobal2Session[pair] = sessionPair + a_delta;
            bChange = true;
        }
    }
    return bChange;
}   // AdjustAssignments()

bool DeleteAssignmentFromPair(UINT a_sessionPair)
{
    if ( a_sessionPair < s_vuPairnrSession2Global.size() )
    {
        UINT globalPair = s_vuPairnrSession2Global[a_sessionPair];   // can be 0
        if ( s_vuPairnrGlobal2Session[globalPair] )
        {
            s_vuPairnrGlobal2Session[globalPair] = 0;
            return true;
        }
    }
    return false;
}   // DeleteAssignmentFromPair()

bool ExistAssignments()
{
    for ( auto sessionPair : s_vuPairnrGlobal2Session )
    {
        if ( sessionPair )
            return true;
    }
    return false;
}   // ExistAssignments()

}   //  end namespace names
