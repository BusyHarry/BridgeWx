// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#if !defined _NAMES_H_
#define _NAMES_H_

#include <wx/string.h>
#include <vector>

#include "utils.h"

namespace names
{
    typedef std::vector<UINT16> UINT16_VECTOR;

    struct PairInfo
    {
        explicit PairInfo(int a_clubIndex=0){clubIndex = a_clubIndex;}
        explicit PairInfo(const wxString& a_name, int a_clubIndex=0){pairName=a_name, clubIndex = a_clubIndex;}
        bool operator == (const PairInfo& b) const
            { return (this->clubIndex == b.clubIndex) && (this->pairName == b.pairName) ? true : false; }
        bool operator != (const PairInfo& b) const {return !(*this == b);}
        wxString    pairName;
        int         clubIndex;
    };

//    typedef std::vector<names::PairInfo>::const_iterator PairInfoConstIt;
    typedef std::vector<names::PairInfo> PairInfoData;
    typedef PairInfoData::const_iterator PairInfoConstIt;
    typedef PairInfoData::      iterator PairInfoIt;

    const wxString      GetNotSet();                                                        // the string you get if no pairnr exists for it
    const PairInfoData& GetGlobalPairInfo       ();                                         // get all the pair info
    UINT                GetNumberOfGlobalPairs  ();                                         // the nr of global pairnr's
    PairInfo            GetGlobalPairInfo       (int pairnr);                               // get info for pair <index>
    bool                ExistGlobalPairnr       (int pairnr);                               // return true if pair exists
    bool                AddGlobalPairInfo       (const PairInfo& a_info);                   // add a new pair
    bool                UpdatePairClubIndex     (int pair, int club);                       // update the clubid for a pair
    bool                ChangePairInfo          (const PairInfo& info, int index);          // update info for pair <index> return true=added/changed
    bool                RemoveClubIdFromPair    (int index);                                // pair <index> not part of club anymore
    wxString            GetClubName             (int index);                                // get clubname for <index>
    int                 GetClubIndex            (const wxString& clubName);                 // get the index/id from the clubname, 0 if not found (case insensetive)
    int                 AddClubName             (const wxString& clubName);                 // add new clubname, return id, 0 == fail: no room
    bool                SetClubName             (const wxString& clubName, int index);      // update clubname <index>, return true=success
    bool                ExistClub               (int index);                                // return true if club exists for that index
    bool                ExistClub               (const wxString& clubName, int* pIndex=0);  // true if clubname exist and pIndex gets the index
    void                WriteClubNames          ();                                         // save the clubnames to  disk
    void                WritePairNames          ();                                         // save the pairnames todisk
    void                InitializePairNames     ();                                         // read all pairnames related files

    int                 GetFreeClubIndex        ();                                         // get an unused id for clubnames, 0 if non available
    void                SetRestorePoint         ();                                         // save current names/club info
    void                GetRestorePoint         ();                                         // restore saved names/club info
    wxString            PairnrSession2GlobalText(UINT sessionPair);                         // convert session pairnr to its global name 57 -> "HH Janssen - Pietersen"
    wxString            PairnrGlobal2GlobalText(UINT globalPair);                           // convert global pairnr to its global name 57 -> "HH Janssen - Pietersen"
    UINT                PairnrSession2GlobalPairnr(UINT sessionPairNr);                     // get global pairnr from sessionPairNr
    UINT                PairnrGlobal2SessionPairnr(UINT globalPairNr);                      // get session pairnr from globalPairNr
    wxString            PairnrSession2SessionText(UINT sessionPair);                        // convert session pairnr to session string:1-14 ->BB14
    wxString            PairnrGlobal2SessionText(UINT globalPair, const std::vector<unsigned int>* table = nullptr);   // convert global pairnr to sessionstring: 75 ->DD15
    UINT                PairnrSessionText2SessionPairnr(const wxString& sessionName);       // convert session name to session pair
    bool                ValidateSessionPairName (wxString& name, UINT& sessionPair);        // validates a session pairname and returns the sessionpairnr
    void                GetSessionAssignmentsPrevious(wxArrayString& a_vPreviousAssignments); // get name-assignments of previous session
    void                WriteAssignmentsToDisk  (const UINT_VECTOR& newGlobalAssignments);  // update new assignments for this session
    bool                ExistSessionPairWithClub();                                         // check if club involved in current session
    UINT                DetermineClubIndex(const wxString& club);                           // get clubindex for clubname

}   // end namespace names

#endif
