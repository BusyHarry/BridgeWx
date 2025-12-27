// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/listctrl.h>
#include <numeric>

#include "cfg.h"
#include "utils.h"
#include "score.h"
#include "calcScore.h"
#include "corrections.h"
#include "names.h"
#include "printer.h"
#include "utils.h"
#include "fileIo.h"
#include "main.h"

static const auto sButlerRemoveScoresPercent = 10;  // remove N % of highest/lowest scores
static const auto sButlerMinimumScores       = 3;   // we want atleast N scores after removal
static const auto sButlerImpsPer10Procent    = 2;   // referee scores: each 10% above/below 50% equals N imps, ASSUME scores are multiple of 5%

struct ButlerFkw{bool bInit = false; long scoreNs=0;long deltaNs=0; int impsNs=0; long scoreEw=0; long deltaEw=0;int impsEw=0;};

typedef std::map<long,ButlerFkw>  ButlerFkwMap;
static std::vector<ButlerFkwMap> svButlerFkw;

struct DatumScore{int dsNS=0; int dsEW=0;};
static std::vector<DatumScore> svDatumScores;
struct lowHighImp { int low; int high; int mp; };
static lowHighImp suImpTable[]=
{
    {0   , 10  , 0 }, {20  , 40  , 1 }, {50  , 80  , 2 }, {90  , 120 , 3 }, {130 , 160 , 4 }, {170 , 210 , 5 },
    {220 , 260 , 6 }, {270 , 310 , 7 }, {320 , 360 , 8 }, {370 , 420 , 9 }, {430 , 490 , 10}, {500 , 590 , 11},
    {600 , 740 , 12}, {750 , 890 , 13}, {900 , 1090, 14}, {1100, 1290, 15}, {1300, 1490, 16}, {1500, 1740, 17},
    {1750, 1990, 18}, {2000, 2240, 19}, {2250, 2490, 20}, {2500, 2990, 21}, {3000, 3490, 22}, {3500, 3990, 23},
    {4000, 99999,24}
};

int ButlerGetMpsFromScore(int a_score, int a_datumScore)
{
    if (score::IsProcent(a_score))
    {   // for now, assume a procentscore is a multiple of 5%
        auto deltaProcent = score::Procentscore2Procent(a_score) - 50;
        return sButlerImpsPer10Procent*deltaProcent/10;
    }

    // now we have a 'normal' score
    a_score = score::Score2Real(a_score);  //convert (possible) real adjusted score to a normal score
    auto const  count   = sizeof(suImpTable)/sizeof(suImpTable[0]);
    auto        diff    = a_score - a_datumScore;
    auto        absDiff = std::abs(diff);
    int         sign    = diff < 0 ? -1 : 1;

    for (auto index = 0; index < count; ++index)
    {
        if (absDiff >= suImpTable[index].low && absDiff <= suImpTable[index].high)
            return sign*suImpTable[index].mp;   // could have done: return sign*index
    }
    return 0;   // should not happen....

}   // ButlerGetMpsFromScore()

struct ScoreInfo
{
    long total              = 0;
    long average            = 0;
    long averageWeighted    = 0;
    long bonus              = 0;
    bool bWeightedAvg       = false;            // true, if not always same number of games
    bool bHasPlayed         = false;
    UINT nrOfSessions       = 0;                // number of sessions included in total/end result
};

struct TopsPerGame
{
    UINT topNS = 0;
    UINT topEW = 0;
};

static const vvScoreData*               spvGameSetData;
static       cor::mCorrectionsSession   smCorSessionValidated;          // contains ONLY correct (combi-)data, created in ValidateSessionCorrections()
static bool                             sbHaveValidCombi { false };     // (re)set in ValidateSessionCorrections() for combi results
static bool                             sbHaveValidNormal{ false };     // (re)set in ValidateSessionCorrections() for normal corrections

static std::vector<Total>               svSessionResult;        // session result for all pairs
static std::vector<UINT>                svSessionRankToPair;    // index array for session results: [a]=b -> at rank 'a' is sessionpair 'b'
static std::vector<UINT>                svSessionPairToRank;    // index array for session results: [a]=b -> sessionpair 'a' has rank 'b'
static std::vector<ScoreInfo>           svTotalResult;          // total result for all pairs
static std::vector<UINT>                svTotalRankToPair;      // index array for total results: [a]=b -> at rank 'a' is globalpair 'b'
static std::vector<UINT>                svTotalPairToRank;      // index array for total results: [a]=b -> globalpair 'a' has rank 'b'
static std::vector<TopsPerGame>         svGameTops;             // max points per game for NS/EW

static CalcScore::FS                    svFrequencyInfo;        // combined frq table for all games
static std::vector<std::vector<wxString> > svFrqstringTable;    // for each (internal) gamenr its string representation

static int ScoreEwToNs(int score);  // convert ew-score to ns-score
static void AddHeader(MyTextFile& a_file);
static bool IsCombiCandidate (UINT sessionPair);    // true, if pair plays against 'absent pair'
static UINT GetNumberOfRounds(UINT sessionPair);    // number of rounds according schema for this pair
static void ValidateSessionCorrections(const cor::mCorrectionsSession* pNonvalidatedCorrections);   // create a validated set of corrections
static void GetValidatedEndCorrections4Session(cor::mCorrectionsEnd& ce, UINT session);

static void GetSessionCorrectionStrings(UINT a_sessionPair, wxString& a_sCombiResult, wxString& a_sCorrectionResult)
{   // get combi/corrections for a sessionPair as strings
    a_sCombiResult.Clear();
    a_sCorrectionResult.Clear();
    if ( !smCorSessionValidated.size() ) return;           // no results, if no corrections at all
    auto it = smCorSessionValidated.find(a_sessionPair);
    if ( it == smCorSessionValidated.end() ) return;       // no results, if pair has no corrections

    bool bButler = cfg::GetButler();
    if ( it->second.games )
    {   // combi
        a_sCombiResult = bButler
            ? FMT("%ii"  , it->second.extra/10) // don't show fractional value!
            : FMT("%d/%d", it->second.extra/10, it->second.maxExtra);
    }

    int correction = it->second.correction;
    if ( correction )
    {   // only non-zero is a real correction
        a_sCorrectionResult = FMT("%d%s", correction
                                        , it->second.type == '%'
                                            ? "%"
                                            : ( bButler
                                                    ? "i"
                                                    : "mp"
                                              )
                                 );
    }
}   // GetSessionCorrectionStrings()

static void InitPairToRankVector(bool a_bSession)
{
    #define SCORE(pair) (a_bSession ? svSessionResult[pair].procentScore : svTotalResult[pair].total)
    std::vector<UINT>& pairToRank = a_bSession ? svSessionPairToRank : svTotalPairToRank;
    std::vector<UINT>& rankToPair = a_bSession ? svSessionRankToPair : svTotalRankToPair;
    pairToRank.clear();
    pairToRank.resize(rankToPair.size());
    for (UINT rank = 1; rank < rankToPair.size(); ++rank)
    {   // todo? if pair == 0, skip this?
        UINT pair       = rankToPair[rank];
        long score      = SCORE(pair);
        UINT realRank   = rank;
        while ( (realRank > 1) && (score == SCORE(rankToPair[realRank-1])) )
        {
            --realRank; // find highest rank with equal score
        }

        pairToRank[pair] = realRank;    // if scores are equal, so the rank is
    }
}   // InitPairToRankVector()

#define CHOICE_PAIR "ChoicePair"
#define CHOICE_GAME "ChoiceGame"
CalcScore::CalcScore(wxWindow* a_pParent, UINT a_pageId) : Baseframe(a_pParent, a_pageId)
{
    m_bDataChanged      = false;
    m_bButler           = cfg::GetButler();
    m_maxPair           = 1;
    m_maxGame           = 1;
    m_choiceResult      = ResultSession;
    m_findPos           = -1;

    wxItemAttr bb;
    bb.SetFont(wxFontInfo(14).Italic());
    long flags = wxLC_REPORT | wxLC_HRULES /* | wxLC_LIST | wxLC_NO_HEADER */;
    if ( cfg::IsDark() )
    {
        flags &= ~wxLC_HRULES;      // in darkmode, the h-rules disappear on mouseover
        bb.SetTextColour(*wxWHITE); // should be done by the system???
    }
    m_pListBox = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, flags);
    bool bResult = m_pListBox->SetHeaderAttr(bb); MY_UNUSED(bResult);
    int size = m_pListBox->GetFont().GetPointSize();
    wxFont  celFont (size, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL); //wxFONTWEIGHT_SEMIBOLD);
    m_pListBox->SetFont(celFont);
    m_pListBox->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED,[this](const wxListEvent & evt){m_findPos = evt.GetIndex();});

    auto pButtonPrint = new wxButton(this, wxID_ANY, _("Print"));
    pButtonPrint->SetToolTip(_("print current window content"));
    pButtonPrint->Bind(wxEVT_BUTTON,&CalcScore::OnPrint, this);

    m_pChoices = new MY_CHOICE(this, _("Result:"), _("show result of the calculations"), Unique("ChoiceResult"));
    m_pChoices->Bind(wxEVT_CHOICE,[this](const auto&){ AUTOTEST_BUSY("resultType");m_choiceResult = m_vChoices[m_pChoices->GetSelection()]; ShowChoice();});

    m_pPairSelect = new MyChoiceMC(this, _("Pair:"), _("Result per pair"), Unique(CHOICE_PAIR));
    m_pPairSelect->Bind(wxEVT_CHOICE, &CalcScore::OnCalcResultPair,this);

    m_pGameSelect = new MyChoiceMC(this, _("Game:"), _("Result per game"), Unique(CHOICE_GAME));
    m_pGameSelect->Bind(wxEVT_CHOICE, &CalcScore::OnCalcResultGame,this);

    wxSizerFlags defaultSF0(0); defaultSF0.Border(wxALL, MY_BORDERSIZE);
    wxSizerFlags defaultSF1(1); defaultSF1.Border(wxALL, MY_BORDERSIZE);

    wxBoxSizer* hBoxSearchOk = new wxBoxSizer(wxHORIZONTAL);
    hBoxSearchOk->Add(CreateSearchBox(), defaultSF1);
    hBoxSearchOk->Add(pButtonPrint     , defaultSF0);
    hBoxSearchOk->AddSpacer(10);
    hBoxSearchOk->MyAdd(m_pChoices     , defaultSF0);
    hBoxSearchOk->MyAdd(m_pPairSelect  , defaultSF0);
    hBoxSearchOk->MyAdd(m_pGameSelect  , defaultSF0);

    // add to layout
    wxStaticBoxSizer* vBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Score calculations"));
    vBox->Add(m_pListBox    , defaultSF1.Expand());
    vBox->Add(hBoxSearchOk  , defaultSF0);
    SetSizer(vBox);     // add to panel

    RefreshInfo();                  // fill the grid with data

    AUTOTEST_ADD_WINDOW(pButtonPrint , "Print"       );
    AUTOTEST_ADD_WINDOW(m_pChoices   , "ChoiceResult");
    AUTOTEST_ADD_WINDOW(m_pPairSelect, CHOICE_PAIR   );
    AUTOTEST_ADD_WINDOW(m_pGameSelect, CHOICE_GAME   );
    m_description = "CalcScore";
    //LogMessage("CalcScore() created");
}   // CalcScore()

CalcScore::~CalcScore()
{
}

void CalcScore::AutotestRequestMousePositions(MyTextFile* a_pFile)
{
    AutoTestAddWindowsNames(a_pFile, m_description);
}   // AutotestRequestMousePositions()

void CalcScore::OnPrint(wxCommandEvent&)
{
    AUTOTEST_BUSY("print");
    PrintPage();
}   // OnPrint()

void CalcScore::ShowChoice()
{
    wxString title;
    const MyTextFile* pTextFile;
    switch (m_choiceResult)
    {
        case ResultSession:
            title = _("Session result");
            pTextFile = &m_txtFileResultSession;
            break;
        case ResultSessionName:
            title = _("Session result on name");
            pTextFile = &m_txtFileResultOnName;
            break;
        case ResultTotal:
            title = _("Final result");
            pTextFile = &m_txtFileResultTotal;
            break;
        case ResultFrqTable:
            title = _("Frequency tables");
            pTextFile = &m_txtFileFrqTable;
            break;
        case ResultGroup:
            title = _("Group result");
            pTextFile = &m_txtFileResultGroup;
            break;
        case ResultClubTotal:
            title = _("Club result total");
            pTextFile = &m_txtFileResultClubTotal;
            break;
        case ResultClubSession:
            title = _("Club result session");
            pTextFile = &m_txtFileResultClubSession;
            break;
        case ResultPair:
            title = _("Result per pair");
            pTextFile = &m_txtFileResultPair;
            break;
        case ResultGame:
            title = _("Result per game");
            pTextFile = &m_txtFileResultGame;
            break;

        default:
            title = _("Unknown type");
            pTextFile = &m_txtFileResultSession;
            break;
    }

    m_pListBox->ClearAll();
    m_pListBox->InsertColumn(0, title); // only one column

    auto lineCount = pTextFile->GetLineCount();
    for (size_t line = 0 ; line < lineCount;  ++line)
    {
        m_pListBox->InsertItem(line, pTextFile->GetLine(line));
    }
    m_pListBox->SetColumnWidth(0, wxLIST_AUTOSIZE);
}   // ShowChoice()

void CalcScore::RefreshInfo()
{
    m_bButler = cfg::GetButler();   // update flag
    m_numberOfSessionPairs = cfg::GetNrOfSessionPairs();
    InitializeAndCalcScores();
    // initalize the result-choices
    wxArrayString choices = {_("session"), _("session on name"), _("frequencytables"), _("group")};
    m_vChoices.clear(); // indexes must match choice-array
    m_vChoices.push_back(ResultSession);
    m_vChoices.push_back(ResultSessionName);
    m_vChoices.push_back(ResultFrqTable);
    m_vChoices.push_back(ResultGroup);
    // now add dynamic parts
    if ( cfg::GetActiveSession() != 0)
    {
        choices.push_back(_("final"));
        m_vChoices.push_back(ResultTotal);
    }
    // club related parts
    if (names::ExistSessionPairWithClub())
    {
        choices.push_back(_("club session"));
        m_vChoices.push_back(ResultClubSession);
        if ( cfg::GetActiveSession() != 0)
        {
            choices.push_back(_("club total"));
            m_vChoices.push_back(ResultClubTotal);
        }
    }
    m_choiceResult = ResultSession;
    m_pChoices->Init(choices, ResultSession);
    ShowChoice();

    const auto&     groupInfo = *cfg::GetGroupData();
    wxArrayString   pairNames;

    for (const auto& it : groupInfo)
    {
        for (auto pair = 1U; pair <= it.pairs; ++pair)
        {
            //if (pair != it.absent)    // easier if ALL pairs are there: selectionId+1 = sessionPairnr
            pairNames.push_back(names::PairnrSession2SessionText(pair+it.groupOffset));
        }
    }

    m_pPairSelect->Set(pairNames);
    m_pGameSelect->Init(score::GetNumberOfGames(), (UINT)(-1));
    cfg::FLushConfigs();            // write all to disk
    Layout();
    if (FindBadGameData())
    {   // show messagebox on top of the result
        CallAfter([this] {MyMessageBox(m_txtBadGameData, _("Error")); });
    }
}   // RefreshInfo()

void CalcScore::PrintPage()
{
    auto itemCount = m_pListBox->GetItemCount();
    if (itemCount == 0) return;     // nothing to print
    wxListItem result;
    result.SetMask(wxLIST_MASK_TEXT);
    bool bResult        = m_pListBox->GetColumn(0, result); MY_UNUSED(bResult);
    wxString headerTxt  = result.GetText();
    bResult             = prn::BeginPrint(headerTxt); MY_UNUSED(bResult);
    for (int line = 0; line < itemCount; ++line)
    {
        wxString info = m_pListBox->GetItemText(line);
        if (info.IsEmpty() || '\n' != *info.rbegin()) info += '\n';    // Last()== *info.rbegin()
        prn::PrintLine(info);
    }

    prn::PrintLine(cfg::GetCopyrightDateTime());
    prn::EndPrint();
}   // PrintPage()

void CalcScore::BackupData()
{
    m_txtFileResultSession  .Flush();
    m_txtFileResultOnName   .Flush();
    m_txtFileFrqTable       .Flush();
    m_pPairSelect->SetSelection(-1);    //unselect
}   // BackupData()

/* virtual */ void CalcScore::DoSearch(wxString& search)
{
    m_pListBox->Select(m_findPos,false);

    search.MakeLower();
    int itemCount = m_pListBox->GetItemCount();
    // wxString wxListCtrl::GetItemText 	( 	long  	item,  int  	col = 0   ) 		const
    for (long ii = 1; ii <= itemCount; ++ii)
    {
        long line = (ii + m_findPos) % itemCount;
        wxString fnd = m_pListBox->GetItemText(line, 0);
        fnd.MakeLower();
        if (wxNOT_FOUND != m_pListBox->GetItemText(line, 0).MakeLower().Find(search))
        {
            m_findPos= line;
            m_pListBox->Select(m_findPos);
            m_pListBox->EnsureVisible(m_findPos);
            break;
        }
    }
}   // DoSearch()

void CalcScore::MergeFrqTables( FS_INFO& ns, const FS_INFO& ew)
{
    for (const auto& itEW : ew)    // now merge ew into ns
    {
        int nsScore  = ScoreEwToNs(itEW.score); // transform ew-score to a comparable ns-score
        auto itNS    = std::find_if(ns.begin(), ns.end(), [nsScore](const FrequencyState& fq){return nsScore == fq.score;});
        if (itNS != ns.end())
        {
            itNS->pointsEW = itEW.points;  // already existing NS score, so just add EW points for this score
        }
        else
        {   // non existing yet, so add the new info
            FrequencyState fkw;
            fkw.pointsEW = itEW.points;
            fkw.score    = nsScore;
            ns.push_back(fkw);
        }
    }
    std::sort(ns.begin(), ns.end(),[](const auto& left, const auto& right){return left.score > right.score;});
}   // MergeFrqTables()

void CalcScore::CalcSession()
{
    m_maxGame = score::GetNumberOfGames();       // highest gamenr

    svFrequencyInfo.clear();
    svFrequencyInfo.resize(m_maxGame+1ULL);                        
    svSessionResult.clear();
    svSessionResult.resize(m_numberOfSessionPairs+1ULL);
    svGameTops.resize(m_maxGame+1ULL);
    if (m_bButler)
    {
        svDatumScores.resize(m_maxGame+1ULL);
        svButlerFkw.clear();
        svButlerFkw.resize(m_maxGame + 1ULL);
    }
    for (UINT game=1; game <= m_maxGame; ++game)            // calc all games
    {
        const auto NS_SCORE = true;
        if (m_bButler)
        {
            CalcGameButler (game,  NS_SCORE);
            CalcGameButler (game, !NS_SCORE);
            CalcButlerFkw(game);
        }
        else
        {
            FS_INFO fsInfoEW;
            CalcGamePercent(game,  NS_SCORE, svFrequencyInfo[game]);    // first scores for NS, add results to 'svSessionResult'
            CalcGamePercent(game, !NS_SCORE, fsInfoEW);                 // then for EW
            MergeFrqTables( svFrequencyInfo[game], fsInfoEW);           // merge ew to main table frequencyInfo
        }
    }

    ApplySessionCorrections();
    SaveSessionResults();
    SaveGroupResult();
    SaveFrequencyTable();
    score::WriteSessionRank(svSessionRankToPair);
    SaveSessionResultShort();
    CalcClub(false);        // club-result for this session
}   // CalcSession()

void CalcScore::InitializeAndCalcScores()
{
    if (!ConfigChanged()) return;
    names::InitializePairNames();                   // get all nameinfo
    cor::InitializeCorrections();                   //   and needed corrections
    spvGameSetData  = score::GetScoreData();        //      and scores
    ValidateSessionCorrections(cor::GetCorrectionsSession());   // ouput in 'smCorSessionValidated'

    CalcSession();
    CalcTotal();
}   // InitializeAndCalcScores()

void CalcScore::CalcGamePercent(UINT game, bool bNs, FS_INFO& fsInfo)
{
    UINT sets = (*spvGameSetData)[game].size();
    if (sets == 0) return;      // nothing to do, not played yet
    
    UINT  adjustedScoreCount = 0;
    std::vector<int> tmpScores;
    for ( auto it : (*spvGameSetData)[game])
    {
        if (it.pairNS > m_numberOfSessionPairs || it.pairEW > m_numberOfSessionPairs)
            continue;   // just ignore scores with a bad pair involved

        int score = bNs ? it.scoreNS : it.scoreEW;
        score = score::Score2Real(score);   //we only want/need real scores or %
        tmpScores.push_back(score);
        if (score::IsProcent(score))      // determine adjustedscore count
            ++adjustedScoreCount;
    }

    std::sort(tmpScores.begin(),tmpScores.end(), [](int a, int b){return a > b;});  // from high to low
    UINT normalTop      = (sets-1)*2;
    UINT top            = normalTop-adjustedScoreCount;
    UINT neubergCount   = 0;
    if (cfg::GetNeuberg())
    {
        neubergCount = sets-adjustedScoreCount;    // nr of comparable scores
        top -= adjustedScoreCount;                 // == sets-1-adjustedScoreCount*2
    }

    if (bNs)
        svGameTops[game].topNS = normalTop;
    else
        svGameTops[game].topEW = normalTop;

    size_t maxIndex = tmpScores.size();
    size_t index = 0;
    do
    {   // determine the different scores and there appearance count
        int score = tmpScores[index++];
        UINT equalCount = 1;
        while (index < maxIndex && score == tmpScores[index] )
        {
            ++equalCount;
            ++index;
        }
        FrequencyState fs;
        fs.nrOfEqualScores = equalCount;
        fs.score = score;
        /*
        * Till now all scores are 'normal' integers. To prevent calculation errors when using floatingpoint
        * we use from now on long values with an implied decimalpoint position of 1.
        * This means: 10 --> 100, -300 --> -3000
        * When you have a value of 3001, it would be displayed lateron as "300.1"
        * For procent values we use an implied dp position of 2:  5037  --> "50.37"
        */
        long points = 0;
        if (score::IsProcent(score))
        {
            points = normalTop*score::Procentscore2Procent(score);
            // 100* to big!
            if (cfg::GetNeuberg())
                points = RoundLong(points, 10);    // round to .1
            else            // round to 
                points = 10*RoundLong(points, 100);
        }
        else
        {   points=1L+top-equalCount;
            top-=equalCount*2;                              // new 'top'
            if (adjustedScoreCount && cfg::GetNeuberg())    // recalc points with special formule
                points = NeubergPoints(points,sets,neubergCount);
            else
                points *=10;                // 1 decimal after dp!!
        } 

        fs.points=points;
        fs.pointsEW = 0;
        fsInfo.push_back(fs);
    } while (index  < maxIndex);

    // now update scores for pairs

    for ( const auto& it : (*spvGameSetData)[game])
    {   // update totals for each pair that played this game
        if (it.pairNS > m_numberOfSessionPairs || it.pairEW > m_numberOfSessionPairs)
            continue;   // just ignore scores with a bad pair involved
        UINT pair = bNs ? it.pairNS  : it.pairEW;
        int score = bNs ? it.scoreNS : it.scoreEW;
        score = score::Score2Real(score);   //we only want/need real scores or %
        svSessionResult[pair].maxScore += normalTop;
        svSessionResult[pair].nrOfGames++;
        auto pScore = std::find_if(fsInfo.begin(), fsInfo.end(), [score](const auto& it){return score == it.score;});
        if (pScore != fsInfo.end()) // should always be the case....
            svSessionResult[pair].points += pScore->points;
    }
}   // CalcGamePercent()

void CalcScore::CalcGameButler(UINT a_game, bool a_bNs)
{
    UINT sets = (*spvGameSetData)[a_game].size();
    if (sets == 0) return;      // nothing to do, not played yet

    std::vector<int> tmpScores;
    for ( auto it : (*spvGameSetData)[a_game])
    {
        if (it.pairNS > m_numberOfSessionPairs || it.pairEW > m_numberOfSessionPairs)
            continue;   // just ignore scores with a bad pair involved
        int score = a_bNs ? it.scoreNS : it.scoreEW;
        if (!score::IsProcent(score))
        {   //we only want/need real scores, percent-scores will be converted to imps lateron
            score = score::Score2Real(score);
            tmpScores.push_back(score);
        }
    }
    // sort the scores so we can easily remove N lowest and N highest scores
    std::sort(tmpScores.begin(),tmpScores.end(), [](int a, int b){return a > b;});  // from high to low

    wxString log = FMT("butler: %s, game=%u, scoreCount=%u", a_bNs?"NS":"EW", a_game, (UINT)tmpScores.size());
    if (tmpScores.size() > sButlerMinimumScores + 2)
    {   // want at least 'sButlerMinimumScores' scores after removal of highest/lowest scores
        // We ALWAYS round-up! So 10 scores -> 1, 11 -> 2 scores to remove
        auto removeCount = (tmpScores.size()*sButlerRemoveScoresPercent+90)/100;
        if (removeCount == 0) removeCount = 1;
        log += FMT(", removeCount = %u", (UINT)removeCount);
        for (; removeCount; --removeCount)
        {
            tmpScores.pop_back();
            tmpScores.erase(tmpScores.begin());
        }
    }
    // now determine datum-score
    // int sumTest = std::accumulate(tmpScores.begin(), tmpScores.end(), 0);
    int count = static_cast<int>(tmpScores.size());
    int sum = 0;
    for (auto it : tmpScores) sum += it;
    int datumScore = sum/count; // remark: DON'T round here! -> 4.9 -> 4 -> 0 and not 4.9 -> 5 -> 10 as datum score!
    datumScore = ((datumScore + ((datumScore<0)?-5:+5))/10)*10;   //round to nearest multiple of 10
    log += FMT(", datumScore = %i", datumScore);
//    MyLogDebug(log);
    if (a_bNs)
        svDatumScores[a_game].dsNS = datumScore;
    else
        svDatumScores[a_game].dsEW = datumScore;
    // now determine the mps for all boards/players
    for (auto it : (*spvGameSetData)[a_game])
    {
        if (it.pairNS > m_numberOfSessionPairs || it.pairEW > m_numberOfSessionPairs)
            continue;   // just ignore scores with a bad pair involved
        auto pair = a_bNs ? it.pairNS  : it.pairEW;
        auto score= a_bNs ? it.scoreNS : it.scoreEW;

        svSessionResult[pair].butlerMp += ButlerGetMpsFromScore(score, datumScore); // %scores are handled in there
        svSessionResult[pair].nrOfGames++;
    }
}   // CalcGameButler()

static wxString DottedName(const wxString& a_name)
{   // return string with size MAX_NAME_SIZE and a_name padded with " ."
    char dots[]=" . . . . . . . . . . . . . . . . . . . . . . . . .";
    size_t nLen = a_name.Len(); if (nLen >= cfg::MAX_NAME_SIZE) return a_name;
    size_t dLen = sizeof(dots);
    size_t neededChars = cfg::MAX_NAME_SIZE - nLen;
    wxString dottedName = a_name+&dots[dLen-neededChars-1];
    if (dottedName[nLen] == '.') dottedName[nLen] = ' ';
    return dottedName;
}   // DottedName()

struct PlayerInfo
{
    bool    bHasPlayed  = false;
    bool    bIsNS       = false;
    UINT    opponent    = 0U;
    long    score       = 0;
};

bool GetPlayerInfo(UINT a_pair, UINT a_game, PlayerInfo& a_playerInfo)
{   // get info for player/game of the current session
    auto begin  = (*spvGameSetData)[a_game].begin();
    auto end    = (*spvGameSetData)[a_game].end();
    auto it     = std::find_if (begin, end,
        [a_pair](const auto& setInfo){return setInfo.pairNS == a_pair || setInfo.pairEW == a_pair;});
    
    if (it == end) return false;
    a_playerInfo.bHasPlayed = true;
    bool bIsNS = it->pairNS == a_pair;
    a_playerInfo.bIsNS      = bIsNS;
    a_playerInfo.opponent   = bIsNS ? it->pairEW  : it->pairNS;
    a_playerInfo.score      = bIsNS ? it->scoreNS : it->scoreEW;
    return true;
}   // GetPlayerInfo()

static long GetGameResultButler(UINT game, bool a_bNs, int a_score)
{
    long gameResult = 0;
    int datumScore = a_bNs ? svDatumScores[game].dsNS : svDatumScores[game].dsEW;
    gameResult = ButlerGetMpsFromScore(a_score, datumScore);
    return gameResult;
}   // GetGameResultButler()

static long GetGameResultPercent(UINT game, bool a_bNs, int a_score)
{
    long gameResult = 0;
    int score = a_bNs ? a_score : ScoreEwToNs(a_score);
    score = score::Score2Real(score);
    auto end2 = svFrequencyInfo[game].end();
    auto it2  = std::find_if(svFrequencyInfo[game].begin(), end2,
        [score](const auto& info){return info.score == score;});
    if (it2 != end2) gameResult = a_bNs ? it2->points : it2->pointsEW;
    return gameResult;
}   // GetGameResultPercent()

long CalcScore::GetSetResult( UINT pair, UINT firstGame, UINT nrOfGames, UINT* a_pGamesPlayed )
{
    long setResult = 0;

    if (a_pGamesPlayed) *a_pGamesPlayed = 0;    // nr of games played for the requested count
    for (UINT ii = 0; ii < nrOfGames; ++ii)
    {
        UINT game = firstGame+ii;
        auto begin = (*spvGameSetData)[game].begin();
        auto end   = (*spvGameSetData)[game].end();
        auto it = std::find_if (begin, end,
            [pair](const auto& setInfo){return setInfo.pairNS == pair || setInfo.pairEW == pair;});
        if (it != end)
        {   // pair has played game, so get its matchpoints
            bool bNs  = it->pairNS == pair;
            int score = bNs ? it->scoreNS : it->scoreEW;
            setResult += m_bButler ? GetGameResultButler(game, bNs, score) : GetGameResultPercent(game, bNs, score);
            if (a_pGamesPlayed) (*a_pGamesPlayed )++;
        }
    }
    return setResult;
}   // GetSetResult()

void CalcScore::SaveGroupResult()
{
    UINT session            = cfg::GetActiveSession();
    UINT setSize            = cfg::GetSetSize();
    UINT sets               = cfg::GetNrOfGames()/setSize;
    UINT offsetFirstGame    = cfg::GetFirstGame() - 1;

    m_txtFileResultGroup.MyCreate(cfg::ConstructFilename(cfg::EXT_RESULT_GROUP), MyTextFile::WRITE);
    m_txtFileResultGroup.AddLine(ES);

    wxString tmp;
    if (session) tmp.Printf(_(", session %u"), session);
    tmp = FMT(_("Group-result of match '%s'%s"), cfg::GetDescription(),  tmp);
    m_txtFileResultGroup.AddLine(tmp);
    m_txtFileResultGroup.AddLine(ES);

    const size_t SIZE_GRP_PAIR       (4);
    const size_t SIZE_GRP_NAME       (cfg::MAX_NAME_SIZE);
    const size_t SIZE_GRP_SCORE      (5);
    const size_t SIZE_GRP_COMBI      (6);
    const size_t SIZE_GRP_CORRECTION (6);
    const size_t SIZE_GRP_GAMES      (7);
    const size_t SIZE_GRP_TOTAL      (6);

    #define GRP_INSERT_POS ((size_t)2)   /*start index to insert set-info*/
    std::vector<FormBuilder::ColumnInfoRow> formInfo =
        {
              {SIZE_GRP_PAIR         , FormBuilder::Align::RIGHT       , ES, true}
            , {SIZE_GRP_NAME         , FormBuilder::Align::LEFT        , ES, true}
                // setinfo added later
            , {SIZE_GRP_COMBI        , FormBuilder::Align::CENTER      , ES, sbHaveValidCombi}
            , {SIZE_GRP_CORRECTION   , FormBuilder::Align::RIGHT_SPACE1, ES, sbHaveValidNormal}
            , {SIZE_GRP_GAMES        , FormBuilder::Align::RIGHT_SPACE2, ES, true}
            , {SIZE_GRP_TOTAL        , FormBuilder::Align::RIGHT       , ES, true}
            , {SIZE_GRP_SCORE        , FormBuilder::Align::RIGHT       , ES, true}
        };
    FormBuilder::Align alignScore = m_bButler ? FormBuilder::Align::RIGHT : FormBuilder::Align::RIGHT_SPACE2;
    std::vector<FormBuilder::ColumnInfoHeader> headerInfo =
    {   // as it says: info for the header, columnsize is taken from the form-info
          {FormBuilder::Align::RIGHT  , ES, ES          }
        , {FormBuilder::Align::RIGHT  , ES, _("games") + " :"}
            // setinfo added later
        , {FormBuilder::Align::RIGHT  , ES, _("combi"  )}
        , {FormBuilder::Align::RIGHT  , ES, _("corr."  )}
        , {FormBuilder::Align::CENTER , ES, _("games"  )}
        , {alignScore                 , ES, _("tot"    )}
        , {FormBuilder::Align::LEFT   , ES, _("score"  )}
    };

    for (UINT ii = 0; ii < sets; ++ii)
    {   // add set-info for form and header
        wxString tmp2 = FMT("%u-%u", offsetFirstGame + setSize * ii + 1, offsetFirstGame + setSize * ii + setSize);
        headerInfo.insert(headerInfo.begin() + GRP_INSERT_POS + ii, {FormBuilder::Align::CENTER, ES, tmp2});
        formInfo  .insert(formInfo  .begin() + GRP_INSERT_POS + ii, {SIZE_GRP_SCORE, FormBuilder::Align::RIGHT, ES, true});
    }

    FormBuilder group(formInfo);
    wxString tmp2 = group.CreateHeader(headerInfo);
    tmp2 += " " + (m_bButler ? _("(imps/game)") : wxString("(%)"));
    m_txtFileResultGroup.AddLine(tmp2);
    for (UINT pair=1; pair <= m_maxPair; ++pair)
    {
        if (svSessionResult[pair].nrOfGames == 0)
        {
            if (cfg::IsSessionPairAbsent(pair))
                tmp.Printf("%*s --> %s", (int)SIZE_GRP_PAIR, names::PairnrSession2SessionText(pair,true),  _("absent"));
            else
                tmp.Printf("%*s %s --> %s", (int)SIZE_GRP_PAIR, names::PairnrSession2SessionText(pair), names::PairnrSession2GlobalText(pair), _("NOT PLAYED"));  // ???? not possible if session ready!
            m_txtFileResultGroup.AddLine(tmp);
            continue;                   // pair didn't play
        }
        std::vector<wxString> rowInfo = {names::PairnrSession2SessionText(pair), DottedName(names::PairnrSession2GlobalText(pair))};

        for (UINT ii = 0; ii < sets; ++ii)
        {
            UINT gamesPlayed = 0;
            long score = GetSetResult(pair, offsetFirstGame+setSize*ii+1, setSize, &gamesPlayed);
            if (gamesPlayed)    // if at least played once in this set, show score
                rowInfo.push_back(LongToAscii1(score * (m_bButler ? 10 : 1)));
            else
                rowInfo.push_back("-----");
        }

        wxString combi;
        wxString correction;
        GetSessionCorrectionStrings(pair, combi, correction);
        rowInfo.push_back(combi);
        rowInfo.push_back(correction);
        rowInfo.push_back(U2String(svSessionResult[pair].nrOfGames));
        rowInfo.push_back(m_bButler ? L2String(svSessionResult[pair].butlerMp) : LongToAscii1(svSessionResult[pair].points));
        rowInfo.push_back(LongToAscii2(svSessionResult[pair].procentScore));
        m_txtFileResultGroup.AddLine(group.CreateRow(rowInfo));
    }

    m_txtFileResultGroup.Flush();
}   // SaveGroupResult()

void CalcScore::SaveSessionResultsProcent()
{
    const size_t SIZE_RP_RANK        (4);
    const size_t SIZE_RP_PAIR        (4);
    const size_t SIZE_RP_NAME        (cfg::MAX_NAME_SIZE);
    const size_t SIZE_RP_TOTAL       (6);
    const size_t SIZE_RP_MAX         (5);
    const size_t SIZE_RP_SCORE       (6);
    const size_t SIZE_RP_COMBI       (6);
    const size_t SIZE_RP_CORRECTION  (6);
    const size_t SIZE_RP_GROUPRESULT (FormBuilder::NO_LIMIT);

    std::vector<FormBuilder::ColumnInfoRow> formInfo =
    {
          {SIZE_RP_RANK       , FormBuilder::Align::RIGHT       , ES , true}
        , {SIZE_RP_PAIR       , FormBuilder::Align::RIGHT       , ES , true}
        , {SIZE_RP_NAME       , FormBuilder::Align::LEFT        , ES , true}
        , {SIZE_RP_TOTAL      , FormBuilder::Align::RIGHT       , ES , true}
        , {SIZE_RP_MAX        , FormBuilder::Align::RIGHT       , ES , true}
        , {SIZE_RP_SCORE      , FormBuilder::Align::RIGHT       , '%', true}
        , {SIZE_RP_COMBI      , FormBuilder::Align::CENTER      , ES , sbHaveValidCombi}
        , {SIZE_RP_CORRECTION , FormBuilder::Align::RIGHT_SPACE1, ES , sbHaveValidNormal}
        , {SIZE_RP_GROUPRESULT, FormBuilder::Align::LEFT        , ES , true}
    };

    std::vector<FormBuilder::ColumnInfoHeader> headerInfo =
    {
          {FormBuilder::Align::RIGHT , ES , _("rank"    )}
        , {FormBuilder::Align::RIGHT , ES , _("pair"    )}
        , {FormBuilder::Align::LEFT  , ES , _("pairname")}
        , {FormBuilder::Align::CENTER, ES , _("tot."    )}
        , {FormBuilder::Align::RIGHT , ES , _("max"     )}
        , {FormBuilder::Align::RIGHT , ' ', _("score"   )}
        , {FormBuilder::Align::RIGHT , ES , _("combi"   )}
        , {FormBuilder::Align::RIGHT , ES , _("corr."   )}
        , {FormBuilder::Align::LEFT  , ES , GetGroupResultString(0, &svSessionRankToPair, GROUPRESULT_SESSION)}
    };

    FormBuilder percentResult(formInfo);
    wxString tmp = percentResult.CreateHeader(headerInfo);

    m_txtFileResultSession.AddLine(tmp);m_txtFileResultOnName.AddLine(tmp);
    UINT startLine = m_txtFileResultSession.GetLineCount();    //from here the pairinfo is addded
    // pre-create empty lines for result on name-order
    for (UINT lc = 1; lc <= m_maxPair;++lc) m_txtFileResultOnName.AddLine(ES);

    for (UINT rank=1; rank < svSessionRankToPair.size(); ++rank)
    {
        UINT pair   = svSessionRankToPair[rank];
        long score  = svSessionResult[pair].procentScore;
        if (svSessionResult[pair].maxScore == 0)
            continue;                   // pair didn't play

        wxString combi;
        wxString correction;
        GetSessionCorrectionStrings(pair, combi, correction);
        std::vector<wxString> rowInfo =
        {
              U2String(svSessionPairToRank[pair])
            , names::PairnrSession2SessionText(pair)
            , DottedName(names::PairnrSession2GlobalText(pair))
            , LongToAscii1(svSessionResult[pair].points)
            , U2String(svSessionResult[pair].maxScore)
            , LongToAscii2(score)
            , combi
            , correction
            , GetGroupResultString(pair)
        };
        tmp = percentResult.CreateRow(rowInfo);
        m_txtFileResultSession.AddLine(tmp);                       // rank order
        m_txtFileResultOnName[0 - 1ULL + pair + startLine] = tmp;  // pair order
    }
}   // SaveSessionResultsProcent()

void CalcScore::SaveSessionResultsButler()
{
    const size_t SIZE_RI_RANK        (4);
    const size_t SIZE_RI_PAIR        (4);
    const size_t SIZE_RI_NAME        (cfg::MAX_NAME_SIZE);
    const size_t SIZE_RI_IMPS        (4);
    const size_t SIZE_RI_GAMES       (7);
    const size_t SIZE_RI_SCORE       (6);
    const size_t SIZE_RI_COMBI       (6);
    const size_t SIZE_RI_CORRECTION  (6);
    const size_t SIZE_RI_GROUPRESULT (FormBuilder::NO_LIMIT);

    std::vector<FormBuilder::ColumnInfoRow> formInfo =
    {   // column-size definition and info for all lines (except header)
          {SIZE_RI_RANK       , FormBuilder::Align::RIGHT       , ES , true}
        , {SIZE_RI_PAIR       , FormBuilder::Align::RIGHT       , ES , true}
        , {SIZE_RI_NAME       , FormBuilder::Align::LEFT        , ES , true}
        , {SIZE_RI_IMPS       , FormBuilder::Align::RIGHT       , ES , true}
        , {SIZE_RI_GAMES      , FormBuilder::Align::RIGHT_SPACE2, ES , true}
        , {SIZE_RI_SCORE      , FormBuilder::Align::RIGHT       , ES , true}
        , {SIZE_RI_COMBI      , FormBuilder::Align::CENTER      , ES , sbHaveValidCombi}
        , {SIZE_RI_CORRECTION , FormBuilder::Align::RIGHT_SPACE1, ES , sbHaveValidNormal}
        , {SIZE_RI_GROUPRESULT, FormBuilder::Align::LEFT        , ES , true}
    };

    std::vector<FormBuilder::ColumnInfoHeader> headerInfo =
    {   // as it says: info for the header, size is taken from the form-info
          {FormBuilder::Align::RIGHT , ES , _("rank"    )}
        , {FormBuilder::Align::RIGHT , ES , _("pair"    )}
        , {FormBuilder::Align::LEFT  , ES , _("pairname")}
        , {FormBuilder::Align::CENTER, ES , _("imps"    )}
        , {FormBuilder::Align::RIGHT , ES , _("games"   )}
        , {FormBuilder::Align::RIGHT , ES , _("score"   )}
        , {FormBuilder::Align::RIGHT , ES , _("combi"   )}
        , {FormBuilder::Align::RIGHT , ES , _("corr."   )}
        , {FormBuilder::Align::LEFT  , ES , GetGroupResultString(0, &svSessionRankToPair, GROUPRESULT_SESSION)}
    };

    FormBuilder impsResult(formInfo);
    wxString tmp = impsResult.CreateHeader(headerInfo);

    m_txtFileResultSession.AddLine(tmp);m_txtFileResultOnName.AddLine(tmp);
    UINT startLine = m_txtFileResultSession.GetLineCount();    //from here the pairinfo is addded
    // pre-create empty lines for result on name-order
    for (UINT lc = 1; lc <= m_maxPair;++lc) m_txtFileResultOnName.AddLine(ES);

    for (UINT rank=1; rank < svSessionRankToPair.size(); ++rank)
    {
        UINT pair   = svSessionRankToPair[rank];
        long score  = svSessionResult[pair].mpPerGame;
        if (svSessionResult[pair].nrOfGames == 0)
            continue;                   // pair didn't play

        wxString combi;
        wxString correction;
        GetSessionCorrectionStrings(pair, combi, correction);
        std::vector<wxString> rowInfo =
        {
              U2String(svSessionPairToRank[pair])
            , names::PairnrSession2SessionText(pair)
            , DottedName(names::PairnrSession2GlobalText(pair))
            , L2String(svSessionResult[pair].butlerMp)
            , U2String(svSessionResult[pair].nrOfGames)
            , LongToAscii2(score)
            , combi
            , correction
            , GetGroupResultString(pair)
        };
        tmp = impsResult.CreateRow(rowInfo);

        m_txtFileResultSession.AddLine(tmp);                       // rank order
        m_txtFileResultOnName[0 - 1ULL + pair + startLine] = tmp;  // pair order
    }
}   // SaveSessionResultsButler()

void CalcScore::SaveSessionResults()
{
    m_txtFileResultSession.MyCreate(cfg::ConstructFilename(cfg::EXT_RESULT_SESSION_RANK), MyTextFile::WRITE);
    m_txtFileResultOnName .MyCreate(cfg::ConstructFilename(cfg::EXT_RESULT_SESSION_NAME), MyTextFile::WRITE);
    AddHeader(m_txtFileResultSession);
    AddHeader(m_txtFileResultOnName);
    m_txtFileResultSession.AddLine(ES); m_txtFileResultOnName.AddLine(ES);

    if (m_bButler)
        SaveSessionResultsButler();
    else
        SaveSessionResultsProcent();

    m_txtFileResultSession.Flush();        // write to disk
    m_txtFileResultOnName.Flush();
}   // SaveSessionResults()

long CalcScore::NeubergPoints(long points, UINT gameCount, UINT comparableCount)
{   // https://www.bridgeservice.nl/NBB-rekenprogramma/Vraag%20&%20Antwoord%2003.pdf
    //SN = (Sn + 1) * N/n - 1    // Sn=points, N=gamecCount, n=comparableCount
    long nbpoints;
    long temp;
    // input 'points' are the 'real' points, return value = points*10: so 10.1 --> 101
    switch (comparableCount)
    {
    case 2: // (points/10.0+1.1)*(gameCount-1);
        nbpoints = (points+11L)*(gameCount-1);
        break;
    case 3: // (points/10.0+1.0)*(gameCount-1);
        nbpoints = (points+10L)*(gameCount-1);
        break;
    default:// (points+1)*gameCount/comparable - 1;
        temp = 100L*(points+1)*gameCount;
        nbpoints = temp/comparableCount - 100L;
        if ((temp % comparableCount) == 0)  //multiples!
            if (((temp/comparableCount) % 10) == 5) // x.x5!!!
                if (nbpoints > 100L*((long)gameCount-1)) //above medium
                    nbpoints -= 5;  // ?.?5 --> ?.?0
                                    //above medium: medium multiples of .05 get a cutoff!
        nbpoints = (nbpoints+5)/10; //round to 1 digit after dp
    }
    return nbpoints;
}   // NeubergPoints()

void CalcScore::ApplySessionCorrections(void)
{
    m_maxPair = 1;

    for (UINT pair = 1; pair < svSessionResult.size(); ++pair)
    {
        if (svSessionResult[pair].nrOfGames == 0)  // valid for percent AND butler score
            continue;    // pair did not play any game, so no corrections possible

        int     correctionProcent   = 0;
        auto    it                  = smCorSessionValidated.find(pair);
        long    butlerCorImps       = 0;    // corrections in imps for butler: 100* real value so mpPerGame can be calculated easy

        m_maxPair = pair;
        if (it != smCorSessionValidated.end())
        {
            cor::CORRECTION_SESSION cs = it->second;
            // now handle the different types of correction
            if (cs.correction)
            {   // some correction in 'mp' or '% '
                if (cs.type == '%')
                {
                    correctionProcent = 100L*cs.correction;
                    //butlerCorImps = 10*cs.correction*sButlerImpsPer10Procent;   // 100*((cor/10)*sButlerImpsPer10Procent)
                }
                else
                {
                    svSessionResult[pair].points += 10L*cs.correction;
                    butlerCorImps = cs.correction*100;
                }
            }

            if (cs.maxExtra)    // can ONLY be true, if no butler!
            {   // from a combi-table, calculated separately!
                UINT combiTop  = 2 * (GetNumberOfRounds(pair) - 1);
                UINT normalTop = cfg::GetNrOfSessionPairs() - 2 - 2;
                long extra     = RoundLong(normalTop * cs.extra   , combiTop);
                int  maxExtra  = RoundLong(normalTop * cs.maxExtra, combiTop);
                svSessionResult[pair].points   += extra;
                svSessionResult[pair].maxScore += maxExtra;
            }
            else
                butlerCorImps += cs.extra*10;     // extra: 2 decimals now

            svSessionResult[pair].nrOfGames += cs.games;
        }   // end of correction calculation

        //  now all corrections are handled: mp and combinationtables are in (max)points, the '%' waits in scoreProcent
        if (m_bButler)
        {
            svSessionResult[pair].procentScore = // for now: too many things depend on it
            svSessionResult[pair].mpPerGame = RoundLong(butlerCorImps + svSessionResult[pair].butlerMp * 100 , (int)svSessionResult[pair].nrOfGames);
            svSessionResult[pair].butlerMp += RoundLong(butlerCorImps, 100);
        }
        else
            svSessionResult[pair].procentScore = correctionProcent + RoundLong(1000L*svSessionResult[pair].points, svSessionResult[pair].maxScore);
    }
    m_maxPair = std::min(m_maxPair, m_numberOfSessionPairs);    // no more then we have active players!
    svSessionRankToPair.resize(m_maxPair+1ULL);
    std::iota (svSessionRankToPair.begin(), svSessionRankToPair.end(), 0); // Fill with 0, 1, ..., i.e. non-sorted! 0->0, 1->1 etc
    std::sort(svSessionRankToPair.begin()+1, svSessionRankToPair.begin()+m_maxPair+1,
        [](UINT left, UINT right){return svSessionResult[left].nrOfGames && svSessionResult[left].procentScore > svSessionResult[right].procentScore; });
    UINT lastPair = svSessionRankToPair[m_maxPair];
    if (svSessionResult[lastPair].nrOfGames == 0)
        svSessionRankToPair[m_maxPair] = 0; // pair is absent, so no rank
    InitPairToRankVector(true);
}   // ApplySessionCorrections()

int ScoreEwToNs(int ewScore)
{   // remark: scores are real or % scores, so we don't need to check for real adjusted scores
    if (score::IsProcent(ewScore))
        return 100+2*OFFSET_PROCENT-ewScore;
    return -ewScore;
}   // ScoreEwToNs()

void CalcScore::MakeFrequenceTable(UINT a_game, std::vector<wxString>& a_stringTable)
{
    a_stringTable.clear();
    wxString tmp;
    tmp.Printf("%s %u", _("game"), a_game + cfg::GetFirstGame() - 1);
    a_stringTable.push_back(tmp);
    if (m_bButler)
    {
        a_stringTable.push_back(FMT("%s %s: %i, %s: %i", _("datumscore"), _("NS"), svDatumScores[a_game].dsNS, _("EW"), svDatumScores[a_game].dsEW));
        // form definitions
        const size_t SIZE_FT_SCORE  (5);
        const size_t SIZE_FT_DELTA  (5);
        const size_t SIZE_FT_IMPS   (4);

        std::vector<FormBuilder::ColumnInfoRow> formInfo =
        {   // column-size definition and info for all rows
              {SIZE_FT_SCORE, FormBuilder::Align::RIGHT, ES , true}
            , {SIZE_FT_DELTA, FormBuilder::Align::RIGHT, ES , true}
            , {SIZE_FT_IMPS , FormBuilder::Align::RIGHT, ' ', true}
            , {SIZE_FT_SCORE, FormBuilder::Align::RIGHT, ES , true}
            , {SIZE_FT_DELTA, FormBuilder::Align::RIGHT, ES , true}
            , {SIZE_FT_IMPS , FormBuilder::Align::RIGHT, ES , true}
        };

        std::vector<FormBuilder::ColumnInfoHeader> headerInfo =
        {   // as it says: info for the header, columnsize is taken from the form-info
              {FormBuilder::Align::RIGHT, ES , _("NS"   )}
            , {FormBuilder::Align::RIGHT, ES , _("delta")}
            , {FormBuilder::Align::RIGHT, ' ', _("imps" )}
            , {FormBuilder::Align::RIGHT, ES , _("EW"   )}
            , {FormBuilder::Align::RIGHT, ES , _("delta")}
            , {FormBuilder::Align::RIGHT, ES , _("imps" )}
        };

        FormBuilder frequencyTable(formInfo);
        tmp = frequencyTable.CreateHeader(headerInfo);
        a_stringTable.push_back(tmp);
        for (auto it = svButlerFkw[a_game].rbegin(); it != svButlerFkw[a_game].rend(); ++it)
        {   //scores high to low overview
            std::vector<wxString> rowInfo = {     score::ScoreToString(it->second.scoreNs)
                                                , L2String(it->second.deltaNs)
                                                , I2String(it->second.impsNs)
                                                , score::ScoreToString(it->second.scoreEw)
                                                , L2String(it->second.deltaEw)
                                                , I2String(it->second.impsEw)
                                            };
            a_stringTable.push_back(frequencyTable.CreateRow(rowInfo));
        }
    }
    else
    {
        // form definitions
        const size_t SIZE_FT_SCORE  (7);
        const size_t SIZE_FT_MP     (5);

        std::vector<FormBuilder::ColumnInfoRow> formInfo =
        {   // column-size definition and info for all rows
              {SIZE_FT_SCORE, FormBuilder::Align::RIGHT, ES, true}
            , {SIZE_FT_MP   , FormBuilder::Align::RIGHT, ES, true}
            , {SIZE_FT_MP   , FormBuilder::Align::RIGHT, ES, true}
        };

        std::vector<FormBuilder::ColumnInfoHeader> headerInfo =
        {   // as it says: info for the header, columnsize is taken from the form-info
              {FormBuilder::Align::RIGHT , ES, _("scoreNS")}
            , {FormBuilder::Align::CENTER, ES, _("NS"     )}
            , {FormBuilder::Align::CENTER, ES, _("EW"     )}
        };

        FormBuilder frequencyTable(formInfo);
        a_stringTable.push_back(frequencyTable.CreateHeader(headerInfo));
        std::vector<wxString> rowInfo = { _("top:"), U2String(svGameTops[a_game].topNS)+"  ", U2String(svGameTops[a_game].topEW)+"  "};
        a_stringTable.push_back(frequencyTable.CreateRow(rowInfo));

        const auto& frqInfo = svFrequencyInfo[a_game];
        for (auto it : frqInfo)
        {
            rowInfo = {score::ScoreToString(it.score),LongToAscii1(it.points),LongToAscii1(it.pointsEW)};
            a_stringTable.push_back(frequencyTable.CreateRow(rowInfo));
        }
    }
}   // MakeFrequenceTable()

void CalcScore::SaveFrequencyTable()
{
    UINT maxGame = score::GetNumberOfGames();
    svFrqstringTable.clear();
    svFrqstringTable.resize(maxGame+1ULL);
    std::vector<size_t> tableSize;  // the size of each frq table
    tableSize.resize(maxGame+1ULL);

    for (UINT game = 1; game <= maxGame; ++game)
    {   // fkw per game
        MakeFrequenceTable(game, svFrqstringTable[game]);
        tableSize[game] = svFrqstringTable[game].size();
    }
    
    m_txtFileFrqTable.MyCreate(cfg::ConstructFilename(cfg::EXT_FKW), MyTextFile::WRITE);
    AddHeader(m_txtFileFrqTable);
    size_t linesOnPage          = m_txtFileFrqTable.GetLineCount();
    const UINT suFrqStringSize  = m_bButler ? 40 : 19;  // size of each line in a frq table
    const UINT suNrOfFrqColumns = m_bButler ?  2 :  4;  // number of frq tables next to eachother

    // now add all tables in 'suNrOfFrqColumns' columns
    for (UINT game = 1; game <= maxGame; game += suNrOfFrqColumns)
    {
        size_t maxLines = 0;
        for (UINT ii = 0; ii < suNrOfFrqColumns && game + ii <= maxGame; ++ii)
        {
            maxLines = std::max(maxLines,tableSize[static_cast<size_t>(game)+ii]);
        }

        if (linesOnPage + 2ULL + maxLines > cfg::GetLinesPerPage())
        {   // doesn't fit on current page, so give a formfeed ('\f')
            m_txtFileFrqTable.AddLine('\f');
            linesOnPage = 0;
        }
        linesOnPage += 2ULL + maxLines;
        m_txtFileFrqTable.AddLine(ES); m_txtFileFrqTable.AddLine(ES);
        for (size_t line = 0; line < maxLines; ++line)
        {
            wxString tmp;
            for (UINT ii = 0; ii < suNrOfFrqColumns && game + ii <= maxGame; ++ii)
            {
//                tmp += line < tableSize[0ULL+game+ii] ? svFrqstringTable[0ULL+game+ii][line] : wxString('.',suFrqStringSize);
                tmp += FMT("%-*s", suFrqStringSize,  (line < tableSize[0ULL+game+ii]) ? svFrqstringTable[0ULL + game + ii][line] : ES);
            }
            m_txtFileFrqTable.AddLine(tmp);
        }

    }
    m_txtFileFrqTable.Flush();
}   // SaveFrequencyTable()

void AddHeader(MyTextFile& a_file)
{
    a_file.AddLine('\'' + cfg::GetDescription() + '\'');

    wxString tmp;
    if (cfg::GetActiveSession() >= 1)
        tmp.Printf(_("result of session %u"), cfg::GetActiveSession());
    else
        tmp = _("result");
    a_file.AddLine(tmp);
}   // AddHeader()

void CalcScore::SaveSessionResultShort()
{
    if (cfg::GetActiveSession()==0)
        return;         // 0 ==> stand-alone session: this data would only be used in next session!

    cor::mCorrectionsEnd mce;    // transform data to write into same format as its being read
    for (UINT pair = 1; pair < svSessionResult.size(); ++pair)          // save score of all pairs
    {
        if (svSessionResult[pair].nrOfGames != 0)                       // check if pair has played
        {   // pair has played
            auto globalPair = names::PairnrSession2GlobalPairnr(pair);
            if (globalPair == 0)
            {   // can't store this result (for possible use in next session), because we do not know to whom it belongs...
                // remark: no scoring-data lost!
                wxString infoMsg = wxString::Format(_("Pair '%s' has played, but was NOT assigned to a global name"), names::PairnrSession2SessionText(pair));
                MyLogError("%s", infoMsg);  // log as error!
                CallAfter([infoMsg] {MyMessageBox(infoMsg, _("Warning")); });
                continue;
            }
            cor::CORRECTION_END ce;
            ce.score = svSessionResult[pair].procentScore;
            ce.games = svSessionResult[pair].nrOfGames;
            mce[globalPair] = ce;           // need global pairnr!
        }
    }

    io::SessionResultWrite(mce, cfg::GetActiveSession());
}   // SaveSessionResultShort()

static long GetResultScore(UINT a_sessionPairnr, bool a_bSession)
{   // just for use in GetGroupResultString() to find equal scores (and so ranks)
    if (a_bSession) return svSessionResult[a_sessionPairnr].procentScore;
    return svTotalResult[names::PairnrSession2GlobalPairnr(a_sessionPairnr)].total;
}   // GetResultScore()

wxString CalcScore::GetGroupResultString(UINT a_sessionPair, const std::vector<UINT>* a_pRankIndex, bool a_bSession)
{
    // on init: 3 chars per group, empty if only one group:  " BL YE RE GR" for group BLue YEllow REd and GReen
    // for a pair: the rank in its group, a '.' if none   :  "  .  .  7  ." for pair being rank 7 in group 'RE'
    static std::vector<UINT> svGroupRank;   // rank within group for sessionPair
    const auto& groupInfo  = *cfg::GetGroupData();

    if ( groupInfo.size() <= 1 || !cfg::GetGroupResult() ) return ES;   // if only 1 group, or cfg says no grp-info, then ES

    wxString result;
    if (a_pRankIndex != nullptr)
    {   // init of data: get string of all group characters and init svGroupRank
        svGroupRank.clear();
        svGroupRank.resize(a_pRankIndex->size(), 0);
        for (const auto& itGroup : groupInfo)
        {
            auto minPair = itGroup.groupOffset;
            auto maxPair = minPair + itGroup.pairs;
            for (auto pair = minPair+1 ; pair <= maxPair; ++pair )  // so these are session-pairnumbers!
            {   // determine rank in group
                UINT rank           = 1;
                UINT groupRank      = 1;
                long previousScore  = -1;
                for (auto rankPair : *a_pRankIndex)
                {
                    long score = GetResultScore(rankPair, a_bSession);
                    if (score != previousScore) groupRank = rank;    // check for equal scores-->equal rank
                    previousScore = score;
                    if (rankPair == pair)
                    {
                        svGroupRank[pair] = groupRank;   // rank within group
                        break;
                    }
                    if (rankPair > minPair && rankPair <= maxPair)
                    {
                        ++rank;     // pair in same group with higher ranking, so rank for current pair is incremented
                    }
                }
            }
        }
        // also init the group-string
        for (const auto& it : groupInfo) {result += FMT("%3s", it.groupChars);}
        return result;  // " BL GR YE OR" : 3 chars per group
    }

    // create string showing rank within group:  "  .  .  5  .  ." --> 3 chars per group: rank or "  ."
    for (const auto& itGroup : groupInfo)
    {
        auto minPair = itGroup.groupOffset;
        auto maxPair = minPair + itGroup.pairs;
        result += ( (a_sessionPair > minPair) && (a_sessionPair <= maxPair) ) ?
            FMT("%3u", svGroupRank[a_sessionPair]) : wxString("  .");
    }

    return result;
}   // GetGroupResultString()

void CalcScore::CalcTotal()
{
    UINT maxSession = cfg::GetActiveSession();
    if (maxSession == 0) return;       // no total result: session result == end result

    UINT globalPairs = names::GetNumberOfGlobalPairs();
    if (globalPairs == 0)
    {
        if (!m_bIsScriptTesting)
            MyMessageBox(_("No names entered yet!"));
        return;  // no names yet
    }
    UINT session;
    std::vector<cor::mCorrectionsEnd> sessionResults;    // corrections have same data as session-results
    sessionResults.resize(maxSession+1ULL);
    for (session = 1; session <= maxSession; ++session)
    {
        cor::CORRECTION_END ce; // preset pairnr's
        for (UINT pair = 0; pair <= globalPairs; ++pair) sessionResults[session][pair] = ce;
        //load session result
        (void)io::SessionResultRead(sessionResults[session], session);
        //append end-corrections
        // get validated end-corrections for this session
        cor::mCorrectionsEnd correctionsEnd;
        GetValidatedEndCorrections4Session(correctionsEnd, session);
        (void)io::CorrectionsEndRead(sessionResults[session], session, false);
    }

    svTotalResult.resize(globalPairs+1ULL);

    long    totalScore;
    long    totalBonus;
    UINT    games;
    bool    bBonus = false;          // assume no bonus
    long    tempscore;
    UINT    pair;
    bool    bWeightedAvg = cfg::GetWeightedAvg();

    for (pair=1; pair <= globalPairs; ++pair)   // calc total+average
    {
        long totalScoreAvg   = 0;           // init used vars
        UINT totalGames      = 0;
        long average         = 0;
        long averageWeighted = 0;
        UINT absentCount     = 0;
        UINT noSessionCount  = 0;   // nr of times the session score is not added to end/total count (== endcorrection)
        totalScore           = 0;
        totalBonus           = 0;
        svTotalResult[pair].nrOfSessions = maxSession;  // assume all played sessions account to end/total result
        for (session=1;session <= maxSession;++session)
        {
            long score = sessionResults[session][pair].score;
            if (score == SCORE_NO_TOTAL)
            {
                ++noSessionCount;
                --svTotalResult[pair].nrOfSessions;
                continue;
            }
            if (sessionResults[session][pair].games == 0)
                ++absentCount;
            else
            {
                svTotalResult[pair].bHasPlayed = true;  // at least played once in all sessions
                totalScore      += score;
                games            = sessionResults[session][pair].games;
                totalScoreAvg   += (int)games*sessionResults[session][pair].score;
                totalGames      += games;
                if (games * session != totalGames)
                    svTotalResult[pair].bWeightedAvg = true;
            }
            totalBonus += sessionResults[session][pair].bonus;
        }
        if (                                            // correct average if .... 
            (absentCount != 0) &&                       // not always present
            (absentCount <= cfg::GetMaxAbsent()) &&     //   and not too often not present
            (maxSession != absentCount)                 //     and not always not present
            )
        {
            averageWeighted=RoundLong(totalScoreAvg,(int)totalGames);   // determine average score
            if (averageWeighted > (long)cfg::GetMaxMean())              // too big: take max average
            {   //total= average*present + absentCount*min(average,maxaverage)
                totalScoreAvg = averageWeighted*(maxSession-absentCount);
                averageWeighted=cfg::GetMaxMean();
                totalScoreAvg += averageWeighted*absentCount;
            }
            else
                totalScoreAvg = averageWeighted*maxSession;
            average=RoundLong(totalScore,maxSession-(absentCount+noSessionCount)); // determine average score
            if (average > (long)cfg::GetMaxMean() )                         // too big: take max average
                average=cfg::GetMaxMean();
            totalScore += average*absentCount;
        }
        else
        {
            if (totalGames)  // don't devide by 0!
                averageWeighted=RoundLong(totalScoreAvg,(int)totalGames);    // determine average score
            else
                averageWeighted = 0;
            average=RoundLong(totalScore,maxSession);
            totalScoreAvg = averageWeighted*maxSession;
        }
        svTotalResult[pair].averageWeighted=averageWeighted;   // save it
        if (bWeightedAvg)
        {   svTotalResult[pair].total   = totalScoreAvg+totalBonus;
            svTotalResult[pair].average = svTotalResult[pair].averageWeighted;
        }
        else
        {   svTotalResult[pair].total     = totalScore+totalBonus;// and total
            svTotalResult[pair].average = average;      // save it
        }
        svTotalResult[pair].bonus=totalBonus;           //  and bonus
        if (totalBonus)
            bBonus = TRUE;                              // for display
    }   // end for all pairs

    svTotalRankToPair.resize(globalPairs+1ULL);
    std::iota(svTotalRankToPair.begin(), svTotalRankToPair.end(), 0);   // fill with 0,1,2,3....
    std::sort(svTotalRankToPair.begin()+1, svTotalRankToPair.end(),
        [](auto left, auto right){return  svTotalResult[left].bHasPlayed && svTotalResult[left].total > svTotalResult[right].total;} );

    for (auto it = svTotalRankToPair.rbegin(); it != svTotalRankToPair.rend(); ++it)
    {
        if (svTotalResult[*it].bHasPlayed) break;   // from here on, all pairs have played
        *it = 0;    // set pair-id to 0 if pair has not played yet, so has no rank yet
    }

    InitPairToRankVector(false);

    std::vector<UINT> indexSessionPairnr;    // index with session pairnrs in it for getting group-string if wanted
    indexSessionPairnr.resize(svTotalRankToPair.size());
    for ( UINT ii = 1; ii < indexSessionPairnr.size(); ++ii)
    {
        indexSessionPairnr[ii] = names::PairnrGlobal2SessionPairnr(svTotalRankToPair[ii]);
    }
    indexSessionPairnr[0] = 0;

    m_txtFileResultTotal.MyCreate(cfg::ConstructFilename(cfg::EXT_RESULT_TOTAL), MyTextFile::WRITE);
    m_txtFileResultTotal.AddLine(cfg::GetDescription());
    m_txtFileResultTotal.AddLine(FMT(_("Final result%s"), bWeightedAvg ? _(" (weighted average)") : ES));
    m_txtFileResultTotal.AddLine(ES);

    // form definitions
    const size_t SIZE_RT_RANK        (4);
    const size_t SIZE_RT_NAME        (cfg::MAX_NAME_SIZE);
    const size_t SIZE_RT_SESSION     (5);
    const size_t SIZE_RT_BONUS       (7);
    const size_t SIZE_RT_TOTAL       (7);
    const size_t SIZE_RT_AVG         (5);
    const size_t SIZE_RI_SCORE       (6);
    const size_t SIZE_RT_CORRECTION  (5);
    const size_t SIZE_RT_GROUPRESULT (FormBuilder::NO_LIMIT);

    #define RT_INSERT_POS ((size_t)2)   /*start index to insert session info*/
    std::vector<FormBuilder::ColumnInfoRow> formInfo =
    {   // column-size definition and info for all rows
          {SIZE_RT_RANK       , FormBuilder::Align::RIGHT, ES , true}
        , {SIZE_RT_NAME       , FormBuilder::Align::LEFT , ' ', true}
        // session info added later
        , {SIZE_RT_BONUS      , FormBuilder::Align::RIGHT, ES , bBonus}
        , {SIZE_RT_TOTAL      , FormBuilder::Align::RIGHT, ES , true}
        , {SIZE_RT_AVG        , FormBuilder::Align::RIGHT, ES , true}
        , {SIZE_RT_GROUPRESULT, FormBuilder::Align::LEFT , ES , true}
    };

    std::vector<FormBuilder::ColumnInfoHeader> headerInfo =
    {   // as it says: info for the header, columnsize is taken from the form-info
          {FormBuilder::Align::LEFT , ES , _("rank"    )}
        , {FormBuilder::Align::LEFT , ' ', _("pairname")}
        // session info added later
        , {FormBuilder::Align::RIGHT, ES , _("bonus"   )}      //xgettext:TRANSLATORS: "total", translation max length = 7
        , {FormBuilder::Align::RIGHT, ES , _("total"   )}      //xgettext:TRANSLATORS: "avg.", translation max length = 5
        , {FormBuilder::Align::RIGHT, ES , _("avg."    )}
        , {FormBuilder::Align::LEFT , ES , GetGroupResultString(0, &indexSessionPairnr, GROUPRESULT_FINAL)}
    };

    for (session=1; session <= maxSession; ++session)
    {
        formInfo  .insert(formInfo.begin()   + RT_INSERT_POS + session - 1, {SIZE_RT_SESSION, FormBuilder::Align::RIGHT, ES, true});
        //xgettext:TRANSLATORS: 'S' is first character of 'Session'
        headerInfo.insert(headerInfo.begin() + RT_INSERT_POS + session - 1, {FormBuilder::Align::CENTER, ' ', FMT(_("S%u"), session)});
    }

    FormBuilder totalResult(formInfo);
    wxString tmp = totalResult.CreateHeader(headerInfo);
    m_txtFileResultTotal.AddLine(tmp);

    for (UINT rank = 1; rank < svTotalRankToPair.size(); ++rank)
    {
        pair        = svTotalRankToPair[rank];
        if (!svTotalResult[pair].bHasPlayed)
            break;  // all global players that have not played yet, should be at end of rank-array!
        totalScore  = svTotalResult[pair].total;
        std::vector<wxString> rowInfo = {U2String(svTotalPairToRank[pair]), DottedName(names::PairnrGlobal2GlobalText(pair))};

        for (session=1; session <= maxSession; ++session)
        {
            char extra = ' ';
            tempscore = sessionResults[session][pair].score;
            if (tempscore == SCORE_NO_TOTAL)
                rowInfo.push_back("-----");
            else if (sessionResults[session][pair].games == 0)
            {
                rowInfo.push_back(LongToAscii2(svTotalResult[pair].average));
                extra = 'a';
            }
            else
                rowInfo.push_back(LongToAscii2(tempscore));

            formInfo[RT_INSERT_POS + session - 1].extra = extra;
        }
        wxString bonusString;
        if (bBonus)                         // yes, we have a bonus!
        {
            totalBonus = svTotalResult[pair].bonus;
            if (totalBonus)                // and this pair has it
                bonusString=LongToAscii2(totalBonus);
        }
        rowInfo.push_back(bonusString);
        wxString totalString;
        if (bWeightedAvg)
        {
            if (svTotalResult[pair].bWeightedAvg)
                totalString = '+';      // average mean gives a difference
            else
                totalString = ' ';      // no dif
        }
        else
        {
            totalString = LongToAscii2(totalScore);
        }
        rowInfo.push_back(totalString);
        rowInfo.push_back(LongToAscii2( RoundLong(totalScore,svTotalResult[pair].nrOfSessions)));
        rowInfo.push_back(GetGroupResultString(names::PairnrGlobal2SessionPairnr(pair)));
        tmp = totalResult.CreateRow(rowInfo);
        m_txtFileResultTotal.AddLine(tmp);
    }   // end for all ranks

    m_txtFileResultTotal.Flush();
    score::WriteTotalRank(svTotalRankToPair);  // rankorder for support of pair-assignments in next session
    CalcClub(true);
}   // CalcTotal()

struct CLUB_DATA
{
    long    score       = 0;    // accumulated (session) scores, up to max nr of allowed pairs
    long    totalScore  = 0;    // accumulated scores for all pairs
    UINT    clubCount   = 0;    // nr of pairs playing for this club
    UINT    clubId      = 0;    // the id of the club, from name-info
    long    average     = 0;    // average of upto cfg::GetMaxClubcount() pairs;
};

static bool CompareClubs(const CLUB_DATA& left, const CLUB_DATA& right)
{   // compare function for sorting the club-results
    UINT state = 0;
    if (left.clubCount >= cfg::GetMinClubcount())
        state  = 1;
    if (right.clubCount >= cfg::GetMinClubcount())
        state |= 2;
    switch (state)
    {
        case 1: return true;    // left  within limits, right not: so left  is larger!
        case 2: return false;   // right within limits, left  not: so right is larger!
        case 3:                 // both within limits, so simply compare (inrange) averages (== scores)
            // comparing averages is much more 'fair' then scores/totals: more pairs are favored!
            return left.average > right.average;    // return left.score > right.score;
        default:                // both NOT within limits, compare averages (totalscores)
            return left.average > right.average;    //return left.totalScore > right.totalScore;
    }
}   // CompareClubs()

void CalcScore::CalcClub( bool a_bTotal)
{   // results for clubs, if you have assigned pairnames to a club
    UINT                maxClubIndex= 0;
    std::vector<UINT>&  rankToPair  = a_bTotal ? svTotalRankToPair       : svSessionRankToPair;
    MyTextFile&         txtFile     = a_bTotal ? m_txtFileResultClubTotal : m_txtFileResultClubSession;
    UINT                session     = cfg::GetActiveSession();
    UINT                maxSession  = a_bTotal ? session : 1;
    wxString            header      = a_bTotal ? _("Final result") : session ? FMT(_("Result of session %u"), session): _("Session result");
    wxString            fileName    = cfg::ConstructFilename(a_bTotal ? cfg::EXT_CLUB_TOTAL : cfg::EXT_SESSION_CLUB);

    std::vector<CLUB_DATA> club;
    club.resize(cfg::MAX_CLUBNAMES+1ULL);

    UINT maxClubCount = cfg::GetMaxClubcount();
    UINT minClubCount = cfg::GetMinClubcount();
    for (UINT rank = 1; rank < rankToPair.size(); ++rank)   // sum first N scores per club
    {
        UINT pair = rankToPair[rank];
        if (pair)                                           // a pair that has played
        {
            // for total, use sum of session-results for better accuracy
            // like: (x.01+x.00)/2 = x.01   and (x.01+x.01)/2=x.01, but its 'more'!
            long score = a_bTotal ? svTotalResult[pair].total : svSessionResult[pair].procentScore;                 // score bepalen
            if (score)
            {
                if (!a_bTotal) pair = names::PairnrSession2GlobalPairnr(pair);
                UINT clubId =  names::GetGlobalPairInfo(pair).clubIndex;         // club-id
                if (clubId > maxClubIndex)
                    maxClubIndex = clubId;
                if (club[clubId].clubCount < maxClubCount)
                {
                    club[clubId].score += score;        // sum allowable scores
                    club[clubId].clubId = clubId;       // and remember its id
                    // only use 'average' for sorting: arbitrairy *100, so we have 2 extra significant digits
                    club[clubId].average= (100*club[clubId].score)/(int)(1+club[clubId].clubCount);
                }
                club[clubId].totalScore += score;       // sum ALL scores
                club[clubId].clubCount++;               // and adjust clubCount
            }
        }
    }
    // now we have all scores
    if (maxClubIndex == 0) return;      // without clubs, no clubresults!

    std::sort(club.begin()+1, club.begin()+maxClubIndex+1, CompareClubs);
    txtFile.MyCreate(fileName, MyTextFile::WRITE);
    txtFile.AddLine(ES);

    wxString tmp = FMT(_("%s of the clubs for '%s'"), header, cfg::GetDescription());
    txtFile.AddLine(tmp);

    tmp = FMT(_("minimum number of pairs : %u"),minClubCount);
    txtFile.AddLine(tmp);

    tmp = FMT(_("maxsimum number of pairs: %u"),maxClubCount);
    txtFile.AddLine(tmp);
    txtFile.AddLine(m_bButler ? _("Scores are in imps, results and average in imps/pair") : _("Scores, results and average are in %"));
    txtFile.AddLine(ES);

    const size_t SIZE_CR_RANK        (4);
    const size_t SIZE_CR_COUNT       (6);
    const size_t SIZE_CR_CLUB        (cfg::MAX_CLUB_SIZE);
    const size_t SIZE_CR_SCORE       (7);
    const size_t SIZE_CR_AVG         (5);
    const size_t SIZE_CR_TOTAL       (FormBuilder::NO_LIMIT);


    std::vector<FormBuilder::ColumnInfoRow> formInfo =
    {
          {SIZE_CR_RANK , FormBuilder::Align::RIGHT_SPACE1, ES, true}
        , {SIZE_CR_COUNT, FormBuilder::Align::RIGHT_SPACE2, ES, true}
        , {SIZE_CR_CLUB , FormBuilder::Align::LEFT        , ES, true}
        , {SIZE_CR_SCORE, FormBuilder::Align::RIGHT       , ES, true}
        , {SIZE_CR_AVG  , FormBuilder::Align::RIGHT       , ES, true}
        , {SIZE_CR_TOTAL, FormBuilder::Align::RIGHT       , ES, true}
    };

    std::vector<FormBuilder::ColumnInfoHeader> headerInfo =
    {   // as it says: info for the header, columnsize is taken from the form-info
          {FormBuilder::Align::RIGHT, ES , _("rank"      )}
        , {FormBuilder::Align::RIGHT, ES , _("count"     )}
        , {FormBuilder::Align::LEFT , ES , _("club-name" )}
        , {FormBuilder::Align::RIGHT, ES , _("score"     )}
        , {FormBuilder::Align::RIGHT, ' ', _("avg."      )}
        , {FormBuilder::Align::LEFT , ES , _("totalscore")}
    };

    FormBuilder clubResult(formInfo);
    wxString tmp2 = clubResult.CreateHeader(headerInfo);

    txtFile.AddLine(tmp2);


    UINT rank               = 1;
    UINT actualRank         = 1;
    long previousAvgScore   = 99999;    // just some impossible score..

    for (UINT clubIndex = 1; clubIndex <= maxClubIndex; ++clubIndex)
    {
        UINT clubCount = club[clubIndex].clubCount;
        if (clubCount == 0) continue;      // this club was not present in match

        if (clubCount > maxClubCount)
            clubCount = maxClubCount;
        long avgScore = RoundLong(club[clubIndex].score, clubCount*maxSession);
        if (avgScore != previousAvgScore)
        {   // check if previous score is equal, and then emit equal rank
            actualRank = rank;
            previousAvgScore = avgScore;
        }
        std::vector<wxString> rowInfo =
        {
              U2String(actualRank)
            , U2String(clubCount)
            , names::GetClubName(club[clubIndex].clubId)
            , LongToAscii2(club[clubIndex].score)
            , LongToAscii2(avgScore)
            , FMT("%7s (%2u, %5s)"
                , LongToAscii2(club[clubIndex].totalScore)
                , club[clubIndex].clubCount
                , LongToAscii2 (RoundLong(club[clubIndex].totalScore, club[clubIndex].clubCount * maxSession)))
        };
        tmp2 = clubResult.CreateRow(rowInfo);
        txtFile.AddLine(tmp2);

        ++rank;
    }

    if (club[0].clubCount)
    {   // pairs, not a member of a club
        txtFile.AddLine(ES);
        tmp = FMT(_("individual pairs: %u, score: %6s (%5s)"),
            club[0].clubCount,
            LongToAscii2(club[0].totalScore),
            LongToAscii2(RoundLong(club[0].totalScore,club[0].clubCount*maxSession))
        );
        txtFile.AddLine(tmp);
    }

    txtFile.Flush();    // write to disk
}   // CalcClub()

void CalcScore::CalcResultPairHelper(long& sumPoints, UINT& sumTops, UINT& gamesPlayed, wxString& tmp)
{
    #undef ADDLINE
    #define ADDLINE m_txtFileResultPair.AddLine

    if (gamesPlayed)
    {
        if (m_bButler)
            tmp += FMT(_(" set-score: %ld (%s imps/game)"), sumPoints, LongToAscii2(sumPoints*100/(int)gamesPlayed)    );
        else
            tmp += FMT(_(" set-score: %s(%u%%)"), LongToAscii1(sumPoints), (sumPoints*10+sumTops/2)/sumTops);
    }
    ADDLINE(tmp);
    ADDLINE(ES);
    gamesPlayed = 0;
    sumPoints   = 0;
    sumTops     = 0;
    #undef ADDLINE
}   // CalcResultPairHelper()

void CalcScore::OnCalcResultPair(const wxCommandEvent& a_evt)
{
    AUTOTEST_BUSY("resultPair");
    UINT        pair            = 1U + a_evt.GetInt();
    auto        session         = cfg::GetActiveSession();
    UINT        maxGame         = score::GetNumberOfGames();
    UINT        setSize         = cfg::GetSetSize();
    UINT        firstGame       = cfg::GetFirstGame()-1;
    wxString    sessionString   = session >= 1 ? FMT(_(", session %u"), session) : ES ;
    wxString    tmp;

    m_txtFileResultPair.Clear();
#undef ADDLINE
#define ADDLINE m_txtFileResultPair.AddLine
    ADDLINE(ES);
    ADDLINE(FMT(_("Result of pair %s '%s' for '%s'%s"), names::PairnrSession2SessionText(pair), names::PairnrSession2GlobalText(pair), cfg::GetDescription(), sessionString ));
    ADDLINE(ES);

    if (svSessionResult[pair].nrOfGames == 0)
    {
        if (cfg::IsSessionPairAbsent(pair))
            ADDLINE(_("absent"));
        else
            ADDLINE(_("Pair has not played (yet)"));
        m_choiceResult = ResultPair;
        ShowChoice();
        return;
    }
    long sumPoints  = 0;
    UINT sumTops    = 0;
    UINT gamesPlayed= 0;    // nr of games played at a table
    for (UINT game = 1; game <= maxGame; ++game)
    {
        tmp.Printf(_("Game %2u: "), game+firstGame);
        PlayerInfo playerInfo;
        if (!GetPlayerInfo(pair, game, playerInfo))
        {
            tmp += _("not played");
            if ((game % setSize) == 0)
            {
                CalcResultPairHelper(sumPoints, sumTops, gamesPlayed, tmp);
            }
            else
                ADDLINE(tmp);
            continue;
        }

        ++gamesPlayed;
        UINT top    = playerInfo.bIsNS ? svGameTops[game].topNS : svGameTops[game].topEW;
        if  (!m_bButler && top == 0) continue;    // no top yet, only playd once??
        long score  = playerInfo.score;
        long points = GetSetResult(pair, game, 1);
        sumPoints  += points;
        sumTops    += top;
        tmp        += playerInfo.bIsNS ? _("NS") : _("EW");
        tmp        += FMT(_(", score: %5s"), score::ScoreToString(score)  );
        if (m_bButler)
            tmp    += FMT(_(", imps %3ld" )  , points                       );
        else
        {
            tmp += FMT(_(", points%6s"), LongToAscii1(points));
            tmp += FMT(" %3ld%%", (points * 10 + top / 2) / top);
        }

        if ((game % setSize) == 1)  // first game of set, show opponent
            tmp += FMT(_(" Played against (%s): %s"), names::PairnrSession2SessionText(playerInfo.opponent), names::PairnrSession2GlobalText(playerInfo.opponent));

        if ((game % setSize) == 0)
        {
            CalcResultPairHelper(sumPoints, sumTops, gamesPlayed, tmp);
        }
        else
            ADDLINE(tmp);
    }

    wxString combiString;
    wxString correctionString;
    wxString corrections;

    GetSessionCorrectionStrings(pair, combiString, correctionString);
    UINT state = 0;
    if ( combiString     .Len() ) state  = 1;
    if ( correctionString.Len() ) state |= 2;

    switch (state)
    {
        case 1:
            corrections = " (" + combiString + ')';
            break;
        case 2:
            corrections = " (" + correctionString + ')';
            break;
        case 3:
            corrections = " (" + combiString + ", " + correctionString + ')';
    }

    long score = svSessionResult[pair].procentScore;
    if (m_bButler)
    {
        ADDLINE(FMT(_("imps: %ld, games: %u, sessionscore: %s imps/game%s, rank: %u"),
            svSessionResult[pair].butlerMp,
            svSessionResult[pair].nrOfGames,
            LongToAscii2(score),
            corrections,
            svSessionPairToRank[pair]
        ));

    }
    else
        ADDLINE(FMT(_("points: %s (%i), games: %u, sessionscore: %s%%%s, rank: %u"),
            LongToAscii1(svSessionResult[pair].points),
            svSessionResult[pair].maxScore,
            svSessionResult[pair].nrOfGames,
            LongToAscii2(score),
            corrections,
            svSessionPairToRank[pair]
            ));

    m_choiceResult = ResultPair;
    ShowChoice();
#undef ADDLINE
}   // OnCalcResultPair()

void CalcScore::OnCalcResultGame(const wxCommandEvent& a_evt)
{
    AUTOTEST_BUSY("resultGame");
    UINT        game            = 1U + a_evt.GetInt();
    auto        session         = cfg::GetActiveSession();
    wxString    sessionString   = session >= 1 ? FMT(_(", session %u"), session) : ES ;

    m_txtFileResultGame.Clear();
#undef ADDLINE
#define ADDLINE m_txtFileResultGame.AddLine
    ADDLINE(ES);
    ADDLINE(FMT(_("Result of game %u for '%s'%s"), game, cfg::GetDescription(), sessionString ));
    ADDLINE(ES);
    for (auto& frq : svFrqstringTable[game])
    {
        ADDLINE(frq);
    }

    ADDLINE(ES);

    const auto& scores      = (*score::GetScoreData())[game];   // scores for this game
    int         sets        = scores.size();
    auto        setsPerLine = std::min(3, sets);                // max n scores/line
    auto        SIZE_NS     = 5;
    auto        SIZE_SCORE  = 12;
    wxString    tmp;

    for (int count = 1; count <= setsPerLine; ++count)
    {
//        tmp += FMT(_("  NS   EW  SCORE       "));
        tmp +=   FormBuilder::CreateColumn(_("NS"   ), SIZE_NS   , FormBuilder::Align::RIGHT_SPACE1)
               + FormBuilder::CreateColumn(_("EW"   ), SIZE_NS   , FormBuilder::Align::RIGHT_SPACE1) + " "
               + FormBuilder::CreateColumn(_("SCORE"), SIZE_SCORE, FormBuilder::Align::LEFT )
            ;

    }
    ADDLINE(tmp);
    tmp.clear();
    int count = 1;
    for (const auto& setData : scores)
    {
        int scoreNs = setData.scoreNS;
        tmp += FMT("%4s %4s %6s%s"
                    , names::PairnrSession2SessionText(setData.pairNS)
                    , names::PairnrSession2SessionText(setData.pairEW)
                    , score::ScoreToString(scoreNs)
                    , score::IsReal(scoreNs)
                                ? wxString("       ")
                                : FMT(",%-6s",score::ScoreToString(setData.scoreEW))
                 );

        if (++count > setsPerLine)
        {
            ADDLINE(tmp);
            tmp.clear();
            count = 1;
        }

    }
    ADDLINE(tmp);

    m_choiceResult = ResultGame;
    ShowChoice();

#undef ADDLINE
}   // OnCalcResultGame()

static long GetDelta(long score, long datum)
{
    return score::IsProcent(score) ? 0 : score::Score2Real(score)-datum;
}   // GetDelta()

void CalcScore::CalcButlerFkw(UINT a_game)
{   // called when 'agame' is calculated, so datum-scores are available
    if ( (*spvGameSetData)[a_game].size() == 0)
        return; // game not played

    ButlerFkwMap  gameMap;
    for (auto it : (*spvGameSetData)[a_game])
    {
        long scoreNs = it.scoreNS;
        if (svButlerFkw[a_game].find(scoreNs) != svButlerFkw[a_game].end() )
            continue;   // score already present
        ButlerFkw fkw;
        fkw.bInit   = true;
        fkw.scoreNs = scoreNs;
        auto datum  = svDatumScores[a_game].dsNS;
        fkw.deltaNs =  GetDelta(fkw.scoreNs, datum);
        fkw.impsNs  = ButlerGetMpsFromScore(fkw.scoreNs, datum);

        fkw.scoreEw = it.scoreEW;
        datum       = svDatumScores[a_game].dsEW;
        fkw.deltaEw = GetDelta(fkw.scoreEw, datum);
        fkw.impsEw  = ButlerGetMpsFromScore(fkw.scoreEw, datum);

        gameMap[scoreNs] = fkw;
    }
    svButlerFkw[a_game] = gameMap;

    if (0) for ( const auto& [score, data] : svButlerFkw[a_game])
        MyLogDebug("Game=%2u, scoreNs=%6s, delta=%5ld, imps=%3i, scoreEw=%6s, delta=%5ld, imps=%3i"
            , a_game
            , score::ScoreToString(data.scoreNs)
            , data.deltaNs
            , data.impsNs
            , score::ScoreToString(data.scoreEw)
            , data.deltaEw
            , data.impsEw
        );

}   //CalcButlerFkw()

bool CalcScore::FindBadGameData()
{   // check if there are pair numbers greater then max number of players according schema
    // calculation of the results would assert (invalid array-index)
    bool bBadGameData= false;
    m_txtBadGameData = _("In game results:\n\n");
    std::vector<UINT> badPairs;
#define CHECK_PAIR(pair) \
        if ( (pair > m_numberOfSessionPairs) && badPairs.end() == std::find(badPairs.begin(), badPairs.end(), pair )) \
        {\
            m_txtBadGameData += FMT(_("Pairnr out of range:%3u, score(s) found for this pair.\n"), pair);\
            badPairs.push_back(pair);\
            bBadGameData = true;\
        }

    for (const auto& gamesData : (*spvGameSetData))
    {   // for all games
        for (const auto& result : gamesData)
        {   // for all gameresults
            CHECK_PAIR(result.pairNS);
            CHECK_PAIR(result.pairEW);
        }
    }

    if (bBadGameData)
    {
        m_txtBadGameData +=     _("\nDid you lower the number of players in any group?");
        m_txtBadGameData += FMT(_("\nMaximum pair nr: %u (== sum of pairs in all groups)."), m_numberOfSessionPairs);
        m_txtBadGameData +=     _("\nBetter remove scores of pairs you want to remove.");
        m_txtBadGameData +=     _("\nResults are NOT reliable!");
    }
    return bBadGameData;
#undef CHECK_PAIR
}   // CheckGameData()

bool IsCombiCandidate(UINT a_sessionPair)
{
    auto        pGroupData = cfg::GetGroupDataFromSessionPair(a_sessionPair);
    SchemaInfo  si(pGroupData->schemaId);
    return si.AreOpponents(a_sessionPair - pGroupData->groupOffset, pGroupData->absent);
}   // IsCombiCandidate()

void ValidateSessionCorrections(const cor::mCorrectionsSession* a_pNonValidatedCorrections)
{   // validate session corrections once, so we don't constantly need to check its validity
    bool bButler      = cfg::GetButler();
    sbHaveValidCombi  = false;
    sbHaveValidNormal = false;
    smCorSessionValidated.clear();
    wxString errorMsg;

    for (const auto& [pair, correction] : *a_pNonValidatedCorrections)
    {
        #define OK_NORMAL 1
        #define OK_COMBI  2
        UINT state = 0;
        cor::CORRECTION_SESSION cs;

        if ( pair > 0 && pair <= cfg::GetNrOfSessionPairs() )
            cs = correction;
        if ( cs.correction )
        {   // 'normal' correction
            if ( cs.type == '%' && bButler )
            {   // butler can't have % correction
                cs.correction = 0;
            }
            else
                state |= OK_NORMAL;    // set valid normal state
        }

        if ( cs.maxExtra || cs.extra || cs.games )
        {   // combi data
            do
            {
                if ( !IsCombiCandidate(pair) )
                {
                    cs.maxExtra = cs.extra = cs.games = 0;
                    break;
                }
                if ( cs.games == 0 )
                {
                    cs.maxExtra = cs.extra = 0;
                    break;
                }
                if ( bButler )
                {
                    if ( cs.maxExtra )
                        cs.maxExtra = cs.extra = cs.games = 0;
                    else
                        state |= OK_COMBI;
                    break;
                }
                if ( (cs.maxExtra * 10 < cs.extra) || (cs.maxExtra == 0) || (cs.extra < 0) )
                    cs.maxExtra = cs.extra = cs.games = 0;
                else
                    state |= OK_COMBI;
            } while (0);
        }

        if ( state )    // some valid data
        {
            sbHaveValidNormal |= 0 != (state & OK_NORMAL);
            sbHaveValidCombi  |= 0 != (state & OK_COMBI);
            smCorSessionValidated[pair] = cs;
        }

        if ( cs != correction && !cfg::IsScriptTesting() )
        {   // some bad data ignored, accumulate errors and show them all at once
            errorMsg += FMT("\n: '%u, %+i%c, %s,%i,%u'"
                                , pair
                                , correction.correction
                                , correction.type
                                , LongToAscii1(correction.extra)
                                , correction.maxExtra
                                , correction.games
                            );
        }
    }   // for()

    if ( errorMsg.Len() )
    {
        GetMainframe()->CallAfter([errorMsg]
            {   // if not CallAfter() we get the msgbox on an empty page
                MyMessageBox(_("Bad data or combi-table results for non-combi player(s) ignored") + errorMsg
                    , _("Warning"), wxOK | wxICON_INFORMATION); 
            });
    }
}   // ValidateSessionCorrections()

void GetValidatedEndCorrections4Session(cor::mCorrectionsEnd& a_ce, UINT a_session)
{
    (void)io::CorrectionsEndRead( a_ce, a_session, true);
    wxString errorMsg;
    cor::mCorrectionsEnd ceTemp;
    bool bButler = cfg::GetButler();
    bool bTesting= cfg::IsScriptTesting();

    for (const auto& [globalPair, ce] : a_ce)
    {
        if (
               (ce.score < (bButler ? -10000 : 0 ))
            || ((ce.score  > 10000) && (ce.score != SCORE_IGNORE && ce.score != SCORE_NO_TOTAL))  // between 0 and 100%  :10000 = 100.00%
            || (ce.bonus   < -9999)
            || (ce.bonus   > 9999)  // between -99.99% and +99.99% or imps
            || (globalPair < 1)
            || (globalPair > names::GetNumberOfGlobalPairs())
            || (ce.games   > cfg::GetNrOfGames())
            || (ce.bonus && (ce.games || ce.score))
            )
        {   // (some) incorrect value(s)
            if ( !bTesting )
            {
                errorMsg += FMT("\n:  '%u,%s,%s,%u'"
                                    , globalPair
                                    , LongToAscii2(ce.score)
                                    , LongToAscii2(ce.bonus)
                                    , ce.games
                               );
            }
        }
        else
            ceTemp[globalPair] = ce;
    }   // for

    if ( errorMsg.Len() )
    {
        GetMainframe()->CallAfter([errorMsg]
            {   // if not CallAfter() we get the msgbox on an empty page
                MyMessageBox(_("Invalid total-correction/end data, will be ignored")+ errorMsg
                                , _("Warning"), wxOK | wxICON_INFORMATION
                            ); 
            });
    }

    a_ce = ceTemp;  // copy to destination what we have...
}   // GetValidatedEndCorrections4Session();

UINT GetNumberOfRounds(UINT a_sessionPair)
{
    auto        pGroupData = cfg::GetGroupDataFromSessionPair(a_sessionPair);
    SchemaInfo  si(pGroupData->schemaId);
    return si.GetNumberOfRounds();
}   // GetNumberOfRounds()

wxString FormBuilder::CreateHeader(const std::vector<ColumnInfoHeader>& a_headerInfo)
{   // creation of the header, called once
    auto size = a_headerInfo.size();
    if (size != m_rowInfo.size())   // no translation: error should popup during development!
        return "Error: mismatch in size of headerInfo and rowInfo";
    wxString result;
    auto rowInfo = m_rowInfo.begin();
    for (const auto& column : a_headerInfo)
    {
        if (rowInfo->active)
        {
            if (FormBuilder::NO_LIMIT == rowInfo->size)
                result += column.header;
            else
                result += CreateColumn(column.header, rowInfo->size, column.align);
            result += column.extra;
        }
        result += FormBuilder::SEPERATOR;   // add a separator between colums
        ++rowInfo;
    }
    return result.RemoveLast(); // remove last separator
}   // CreateHeader()

wxString FormBuilder::CreateRow(const std::vector<wxString>& a_columsContent)
{   // called for each data-row
    size_t size = a_columsContent.size();
    if (size != m_rowInfo.size())   // no translation: error should popup during development!
        return "Error: mismatch in size of columnInfo and rowInfo";
    wxString result;
    auto content = a_columsContent.begin();
    for (const auto& column : m_rowInfo)
    {
        if (column.active)
        {
            if (FormBuilder::NO_LIMIT == column.size)
                result += *content;
            else
                result += CreateColumn(*content, column.size, column.align);
            result += column.extra;
        }
        result += FormBuilder::SEPERATOR;   // add a separator between colums
        ++content;
    }
    return result.RemoveLast(); // remove last separator
}   // CreateRow()

wxString FormBuilder::CreateColumn(const wxString& a_input, size_t a_len, Align a_align)
{   // create a string with the wanted columnsize and alignment
    size_t len = a_input.Len();
    if (len == a_len)
        return a_input;
    if (len > a_len)
        return a_input.Left(a_len);
    wxString result;
    #define _TEST_ 0
    #if _TEST_
    #define FILL_LEFT  '<'
    #define FILL_RIGHT '>'
    #else
    #define FILL_LEFT  ' '
    #define FILL_RIGHT ' '
    #endif
    switch (a_align)
    {
        case FormBuilder::Align::LEFT:
            result = a_input + wxString(FILL_RIGHT, a_len - len);
            break;
        case FormBuilder::Align::CENTER:
            result = wxString(FILL_LEFT, (a_len - len)/2) + a_input + wxString(FILL_RIGHT, a_len - len - (a_len - len)/2);
            break;
        case FormBuilder::Align::RIGHT:
            result = wxString(FILL_LEFT, a_len - len) + a_input;
            break;
        case FormBuilder::Align::LEFT_SPACE1:
            return CreateColumn(" " + a_input, a_len, FormBuilder::Align::LEFT);
            break;
        case FormBuilder::Align::LEFT_SPACE2:
            return CreateColumn("  " + a_input, a_len, FormBuilder::Align::LEFT);
            break;
        case FormBuilder::Align::RIGHT_SPACE1:
            return CreateColumn(a_input + " ", a_len, FormBuilder::Align::RIGHT);
            break;
        case FormBuilder::Align::RIGHT_SPACE2:
            return CreateColumn(a_input + "  ", a_len, FormBuilder::Align::RIGHT);
            break;
    }
    return result;
}   // CreateColumn()

