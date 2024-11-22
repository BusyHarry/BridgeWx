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
#include "database.h"
#include "fileIo.h"

struct ScoreInfo
{
    long total              = 0;;
    long average            = 0;
    long averageWeighted    = 0;
    long bonus              = 0;
    bool bWeightedAvg       = false;            // true, if not always same number of games
};

struct TopsPerGame
{
    UINT topNS = 0;
    UINT topEW = 0;
};

static const vvScoreData*               spvGameSetData;
static const cor::mCorrectionsSession*  spmCorSession;
static const cor::mCorrectionsEnd*      spmCorEnd;
static std::vector<Total>               svSessionResult;        // session result for all pairs
static std::vector<UINT>                svSessionRankToPair;    // index array for session results: [a]=b -> at rank 'a' is sessionpair 'b'
static std::vector<UINT>                svSessionPairToRank;    // index array for session results: [a]=b -> sessionpair 'a' has rank 'b'
static std::vector<ScoreInfo>           svTotalResult;          // total result for all pairs
static std::vector<UINT>                svTotalRankToPair;      // index array for total results: [a]=b -> at rank 'a' is globalpair 'b'
static std::vector<UINT>                svTotalPairToRank;      // index array for total results: [a]=b -> globalpair 'a' has rank 'b'
static std::vector<TopsPerGame>         svGameTops;             // max points per game for NS/EW

static CalcScore::FS                    svFrequencyInfo;        // combined frq table for all games
static std::vector<std::vector<wxString> > svFrqstringTable;    // for each (internal) gamenr its string representation
static const UINT                       suFrqStringSize = 19;   // size of each line in a frq table
static const UINT                       suNrOfFrqColumns = 4;   // number of frq tables next to eachother


static void AddHeader(MyTextFile& a_file);

static wxString GetSessionCorrectionString(UINT a_sessionPair, bool a_bForceResult = false)
{   // if a_bForceResult, then always return a non-empty string (spaces or real values)
    wxString correction; 
    if (a_bForceResult) correction = "       "; // result is always 7 chars
    auto it = spmCorSession->find(a_sessionPair);
    if (it != spmCorSession->end())
    {
        if (it->second.correction)
            correction.Printf("(%+4d%c)",it->second.correction,it->second.type);
        else
            if (it->second.maxExtra)
                correction.Printf("(%5s)",LongToAscii2(RoundLong(10*it->second.extra,it->second.maxExtra)));
    }
    return correction;
}   // GetSessionCorrectionString()

static void InitPairToRankVector(bool a_bSession)
{
    #define SCORE(pair) (a_bSession ? svSessionResult[pair].score : svTotalResult[pair].total)
    std::vector<UINT>& pairToRank = a_bSession ? svSessionPairToRank : svTotalPairToRank;
    std::vector<UINT>& rankToPair = a_bSession ? svSessionRankToPair : svTotalRankToPair;
    pairToRank.clear();
    pairToRank.resize(rankToPair.size());
    for (UINT rank = 1; rank < rankToPair.size(); ++rank)
    {
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
    m_bSomeCorrection   = false;
    m_maxPair           = 1;
    m_maxGame           = 1;
    m_choiceResult      = ResultSession;
    m_findPos           = -1;

    m_pListBox = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      /* wxLC_LIST*/ wxLC_REPORT | wxLC_HRULES);//| wxLC_NO_HEADER);
    wxItemAttr bb;
    bb.SetFont(wxFontInfo(14).Italic());
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
    //LogMessage("CalcScore() aangemaakt");
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
            title = _("Total result");
            pTextFile = &m_txtFileResultTotal;
            break;
        case ResultFrqTable:
            title = _("Frequency states");
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
    InitializeAndCalcScores();
    // initalize the result-choices
    wxArrayString choices = {_("session"), _("session on name"), _("frequencystates"), _("group")};
    m_vChoices.clear(); // indexes must match choice-array
    m_vChoices.push_back(ResultSession);
    m_vChoices.push_back(ResultSessionName);
    m_vChoices.push_back(ResultFrqTable);
    m_vChoices.push_back(ResultGroup);
    // now add dynamic parts
    if ( cfg::GetActiveSession() != 0)
    {
        choices.push_back(_("total"));
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

    auto&           groupInfo = *cfg::GetGroupData();
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
    m_pGameSelect->Init(score::GetNumberOfGames(), (0U-1U));
    cfg::FLushConfigs();            // write all to disk
    Layout();
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
        if (info.IsEmpty() || '\n' != info.Last()) info += '\n';    // Last()== *info.rbegin()
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
    svSessionResult.resize(cfg::MAX_PAIRS+1);
    svGameTops.resize(m_maxGame+1ULL);

    for (UINT game=1; game <= m_maxGame; ++game)            // calc all games
    {
        FS_INFO fsInfoEW;
        CalcGame(game, true , svFrequencyInfo[game]);       // first scores for NS, add results to 'svSessionResult'
        CalcGame(game, false, fsInfoEW);                    // then for EW
        MergeFrqTables( svFrequencyInfo[game], fsInfoEW);   // merge ew to main table frequencyInfo
    }

    ApplySessionCorrections();
    SaveSessionResults();
    SaveGroupResult();
    SaveFrequencyTable();
    score::WriteSessionRank(svSessionRankToPair);
    SaveSessionResultShort();
    CalcClub(false);
}   // CalcSession()

void CalcScore::InitializeAndCalcScores()
{
    if (!ConfigChanged()) return;
    names::InitializePairNames();                   // get all nameinfo
    cor::InitializeCorrections();                   //   and needed corrections
    spvGameSetData  = score::GetScoreData();        //      and scsores
    spmCorSession   = cor::GetCorrectionsSession();
    spmCorEnd       = cor::GetCorrectionsEnd();

    CalcSession();
    CalcTotal();
}   // InitializeAndCalcScores()

void CalcScore::CalcGame(UINT game, bool bNs, FS_INFO& fsInfo)
{
    UINT sets = (*spvGameSetData)[game].size();
    if (sets == 0) return;      // nothing to do, not played yet
    
    UINT  arbitraryCount = 0;
    std::vector<int> tmpScores;
    for ( auto it : (*spvGameSetData)[game])
    {
        int score = bNs ? it.scoreNS : it.scoreEW;
        score = score::Score2Real(score);   //we only want/need real scores or %
        tmpScores.push_back(score);
        if (score::IsProcent(score))      // determine arbitrary count
            ++arbitraryCount;
    }

    std::sort(tmpScores.begin(),tmpScores.end(), [](int a, int b){return a > b;});  // from high to low
    UINT normalTop      = (sets-1)*2;
    UINT top            = normalTop-arbitraryCount;
    UINT neubergCount   = 0;
    if (cfg::GetNeuberg())
    {
        neubergCount = sets-arbitraryCount;    // nr of comparable scores
        top -= arbitraryCount;                 // == sets-1-arbitrarycount*2
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
            top-=equalCount*2;                       // new 'top'
            if (arbitraryCount && cfg::GetNeuberg())       // recalc points with special formule
                points = NeubergPoints(points,sets,neubergCount);
            else
                points *=10;                // 1 decimal after dp!!
        } 

        fs.points=points;
        fs.pointsEW = 0;
        fsInfo.push_back(fs);
    } while (index  < maxIndex);

    // now update scores for pairs

    for ( auto it : (*spvGameSetData)[game])
    {   // update totals for each pair that played this game
        UINT pair = bNs ? it.pairNS  : it.pairEW;
        int score = bNs ? it.scoreNS : it.scoreEW;
        score = score::Score2Real(score);   //we only want/need real scores or %
        svSessionResult[pair].maxScore += normalTop;
        svSessionResult[pair].nrOfGames++;
        auto pScore = std::find_if(fsInfo.begin(), fsInfo.end(), [score](const auto& it){return score == it.score;});
        if (pScore != fsInfo.end()) // should always be the case....
            svSessionResult[pair].points += pScore->points;
    }
}   // CalcGame()

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

long CalcScore::GetSetResult( UINT pair, UINT firstGame, UINT nrOfGames )
{
    long setResult = 0;
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
            auto end2 = svFrequencyInfo[game].end();
            int score = score::Score2Real(bNs ? it->scoreNS : ScoreEwToNs(it->scoreEW));
            auto it2  = std::find_if(svFrequencyInfo[game].begin(), end2,
                [score](const auto& info){return info.score == score;});
            if (it2 != end2) setResult += bNs ? it2->points : it2->pointsEW;
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

    tmp.Printf("    %*s", cfg::MAX_NAME_SIZE, _("games :"));
    for (UINT ii = 0; ii < sets; ++ii)
        tmp += FMT("%2u-%-2u ", offsetFirstGame+setSize*ii+1, offsetFirstGame+setSize*ii+setSize);
    if (spmCorSession->size())
        tmp += _(" extra  cor  cor.");
    tmp += _("  tot   score");
    m_txtFileResultGroup.AddLine(tmp);

    for (UINT pair=1; pair <= m_maxPair; ++pair)
    {
        if (svSessionResult[pair].maxScore == 0)
        {
            tmp.Printf(_("%3u  NOT PLAYED?? absent??"), pair);  //???? kan niet, is "paar heeft niet gespeeld: afwezig..."
            m_txtFileResultGroup.AddLine(tmp);
            continue;                   // pair didn't play
        }
        tmp.Printf("%3u %s", pair, DottedName(names::PairnrSession2GlobalText(pair)));
        for (UINT ii = 0; ii < sets; ++ii)
        {
            long score = GetSetResult(pair, offsetFirstGame+setSize*ii+1, setSize);
            tmp += FMT(" %5s", LongToAscii1(score));
        }
        // now add corrections
        if (spmCorSession->size())
        {
            int procent     = 0;
            int mp          = 0;
            int extra       = 0;
            int maxExtra    = 0;
            auto it = spmCorSession->find(pair);
            if (it != spmCorSession->end())
            {
                extra    = it->second.extra/10;
                maxExtra = it->second.maxExtra;
                if (it->second.type == '%')
                    procent = it->second.correction;
                else
                    mp = it->second.correction;
            }
            tmp += FMT("%3d/%-3d" , extra, maxExtra);
            tmp += FMT( " %2d%%"  , procent);
            tmp += FMT( " %3dmp"  , mp);
        }
        tmp += FMT(" %6s %s%%", LongToAscii1(svSessionResult[pair].points), LongToAscii2(svSessionResult[pair].score));
        m_txtFileResultGroup.AddLine(tmp);
    }

    m_txtFileResultGroup.Flush();
}   // SaveGroupResult()

void CalcScore::SaveSessionResults()
{
    m_txtFileResultSession.MyCreate(cfg::ConstructFilename(cfg::EXT_RESULT_SESSION_RANK), MyTextFile::WRITE);
    m_txtFileResultOnName.MyCreate(cfg::ConstructFilename(cfg::EXT_RESULT_SESSION_NAME), MyTextFile::WRITE);
    AddHeader(m_txtFileResultSession);
    AddHeader(m_txtFileResultOnName);
    m_txtFileResultSession.AddLine(ES); m_txtFileResultOnName.AddLine(ES);

    wxString tmp;
    tmp.Printf(_("rank pair %-*s  tot.    max  score  %s  %s"),
        cfg::MAX_NAME_SIZE,_("pairname"),
        m_bSomeCorrection ? _("corr.  ") : ES,
        GetGroupResultString(0, &svSessionRankToPair, true));
    m_txtFileResultSession.AddLine(tmp);m_txtFileResultOnName.AddLine(tmp);
    UINT startLine = m_txtFileResultSession.GetLineCount();    //from here the pairinfo is addded
    // pre-create empty lines for result on name-order
    for (UINT lc = 1; lc <= m_maxPair;++lc) m_txtFileResultOnName.AddLine(ES);

    for (UINT rank=1; rank < svSessionRankToPair.size(); ++rank)
    {
        UINT pair   = svSessionRankToPair[rank];
        long score  = svSessionResult[pair].score;
        if (svSessionResult[pair].maxScore == 0)
            continue;                   // pair didn't play
        wxString correction = GetSessionCorrectionString(pair, m_bSomeCorrection);
        tmp.Printf(" %3u %4s %s %6s  %4d  %5s%% %s  %s",
            svSessionPairToRank[pair],
            names::PairnrSession2SessionText(pair),
            DottedName(names::PairnrSession2GlobalText(pair)),
            LongToAscii1(svSessionResult[pair].points),
            svSessionResult[pair].maxScore,
            LongToAscii2(score),
            correction,
            GetGroupResultString(pair));
        m_txtFileResultSession.AddLine(tmp);                       // rank order
        m_txtFileResultOnName[0 - 1ULL + pair + startLine] = tmp;  // pair order
    }

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
        if (svSessionResult[pair].maxScore == 0) continue;    // pair did not play any game, so no corrections possible

        int     correctionProcent   = 0;
        auto    it                  = spmCorSession->find(pair);

        m_maxPair = pair;
        if (it != spmCorSession->end())
        {
            cor::CORRECTION_SESSION cs = it->second;
            m_bSomeCorrection = true;
            // now handle the different types of correction
            if (cs.correction)
            {   // some correction in 'mp' or '% '
                if (cs.type == '%')
                    correctionProcent = 100L*cs.correction;
                else
                    svSessionResult[pair].points += 10L*cs.correction;
            }
            if (cs.maxExtra)
            {
                svSessionResult[pair].points    += cs.extra;
                svSessionResult[pair].maxScore  += cs.maxExtra;
            }
        }

        //  now all corrections are handled: mp and combinationtables are in(max)points, the '%' waits in scoreProcent
        svSessionResult[pair].score = correctionProcent + RoundLong(1000L*svSessionResult[pair].points, svSessionResult[pair].maxScore);
    }
    m_maxPair = std::min(m_maxPair, cfg::GetNrOfSessionPairs());    // no more then we have active players!
    svSessionRankToPair.resize(m_maxPair+1ULL);
    std::iota (svSessionRankToPair.begin(), svSessionRankToPair.end(), 0); // Fill with 0, 1, ..., i.e. non-sorted! 0->0, 1->1 etc
    std::sort(svSessionRankToPair.begin()+1, svSessionRankToPair.begin()+m_maxPair+1,
        [](UINT left, UINT right){return svSessionResult[left].score > svSessionResult[right].score; });
    UINT lastPair = svSessionRankToPair[m_maxPair];
    if (svSessionResult[lastPair].maxScore == 0)
        svSessionRankToPair[m_maxPair] = 0; // pair is absent, so no rank
    InitPairToRankVector(true);
}   // ApplySessionCorrections()

int CalcScore::ScoreEwToNs(int ewScore)
{
    if (score::IsProcent(ewScore))
        return 100+2*OFFSET_PROCENT-ewScore;
    return -ewScore;
}   // ScoreEwToNs()

void CalcScore::MakeFrequenceTable(UINT a_game, std::vector<wxString>& a_stringTable)
{
    a_stringTable.clear();
    wxString tmp;
    tmp.Printf(_("    game %-3u       "), a_game + cfg::GetFirstGame() - 1);
    a_stringTable.push_back(tmp);
    a_stringTable.push_back(_("scoreNS NS    EW   "));
    a_stringTable.push_back(FMT(_("   top: %-5u %-5u"), svGameTops[a_game].topNS, svGameTops[a_game].topEW));
    const auto& frqInfo = svFrequencyInfo[a_game];
    for (auto it : frqInfo)
    {
        tmp.Printf("%6s %5s %5s ",
            score::ScoreToString(it.score),
            LongToAscii1(it.points),
            LongToAscii1(it.pointsEW));
        a_stringTable.push_back(tmp);
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
    {
        MakeFrequenceTable(game, svFrqstringTable[game]);
        tableSize[game] = svFrqstringTable[game].size();
    }
    
    m_txtFileFrqTable.MyCreate(cfg::ConstructFilename(cfg::EXT_FKW), MyTextFile::WRITE);
    AddHeader(m_txtFileFrqTable);
    size_t linesOnPage = m_txtFileFrqTable.GetLineCount();

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
               tmp += line < tableSize[0ULL+game+ii] ? svFrqstringTable[0ULL+game+ii][line] : wxString(' ',suFrqStringSize);
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
        return;         // 0 ==> stand-alone session

    cor::mCorrectionsEnd mce;    // transform data to write into same format as its beeing read
    for (UINT pair = 1; pair < svSessionResult.size(); ++pair)         // save score of all pairs
    {
        if (svSessionResult[pair].maxScore != 0)                       // check if pair has played
        {   
            cor::CORRECTION_END ce;
            ce.score = svSessionResult[pair].score;
            ce.games = svSessionResult[pair].nrOfGames;
            mce[names::PairnrSession2GlobalPairnr(pair)] = ce;  // need global pairnr!
        }
    }

    io::SessionResultWrite(mce, cfg::GetActiveSession());
}   // SaveSessionResultShort()

static long GetResultScore(UINT a_sessionPairnr, bool a_bSession)
{   // just for use in GetGroupResultString() to find equal scores (and so ranks)
    if (a_bSession) return svSessionResult[a_sessionPairnr].score;
    return svTotalResult[names::PairnrSession2GlobalPairnr(a_sessionPairnr)].total;
}   // GetResultScore()

wxString CalcScore::GetGroupResultString(UINT a_sessionPair, const std::vector<UINT>* a_pRankIndex, bool a_bSession)
{
    // on init: 2 chars per group, empty if only one group:  "BLYEREGR" for group BLue YEllow REd and GReen
    // for a pair: the rank in its group, a '.' if none   :  " . . 7 ." for pair being rank 7 in group 'RE'
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
        for (const auto& it : groupInfo) {result += FMT("%2s", it.groupChars);}
        return result;  // "BLGRYEOR" : 2 chars per group
    }

    // create string showing rank within group:  " . . 5 . ." --> 2 chars per group: rank or " ."
    for (const auto& itGroup : groupInfo)
    {
        auto minPair = itGroup.groupOffset;
        auto maxPair = minPair + itGroup.pairs;
        result += ( (a_sessionPair > minPair) && (a_sessionPair <= maxPair) ) ?
            FMT("%2u", svGroupRank[a_sessionPair]) : wxString(" .");
    }

    return result;
}   // GetGroupResultString()

void CalcScore::CalcTotal()
{
    UINT maxSession = cfg::GetActiveSession();
    if (maxSession == 0) return;       // no total result: session result == end result

    UINT maxPair = names::GetNumberOfGlobalPairs();
    if (maxPair == 0)
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
        for (UINT pair = 0; pair <= maxPair; ++pair) sessionResults[session][pair] = ce;
        //load session result
        (void)io::SessionResultRead(sessionResults[session], session);
        //append end-corrections
        (void)io::CorrectionsEndRead(sessionResults[session], session, false);
    }

    svTotalResult.resize(maxPair+1ULL);

    long    totalScore;
    long    totalBonus;
    UINT    games;
    bool    bBonus = false;          // assume no bonus
    long    tempscore;
    UINT    pair;
    bool    bWeightedAvg = cfg::GetWeightedAvg();

    for (pair=1; pair <= maxPair; ++pair)   // calc total+average
    {
        long totalScoreAvg   = 0;           // init used vars
        UINT totalGames      = 0;
        long average         = 0;
        long averageWeighted = 0;
        UINT absentCount     = 0;
        totalScore           = 0;
        totalBonus           = 0;
        for (session=1;session <= maxSession;++session)
        {
            if (sessionResults[session][pair].score == 0)
                ++absentCount;
            else
            {
                totalScore      += sessionResults[session][pair].score;
                totalScoreAvg   += (int)sessionResults[session][pair].games*sessionResults[session][pair].score;
                games            = sessionResults[session][pair].games;
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
            average=RoundLong(totalScore,maxSession-absentCount);   // determine avarage score
            if (average > (long)cfg::GetMaxMean() )                 // too big: take max average
                average=cfg::GetMaxMean();
            totalScore += average*absentCount;
        }
        else
        {
            if (totalGames)  // niet door 0 delen!
                averageWeighted=RoundLong(totalScoreAvg,(int)totalGames);    // determine average score
            else
                averageWeighted = 0;
            average=RoundLong(totalScore,maxSession);
            totalScoreAvg = averageWeighted*maxSession;
        }
        svTotalResult[pair].averageWeighted=averageWeighted;   // save it
        if (bWeightedAvg)
        {   svTotalResult[pair].total     = totalScoreAvg+totalBonus;
            svTotalResult[pair].average = svTotalResult[pair].averageWeighted;
        }
        else
        {   svTotalResult[pair].total     = totalScore+totalBonus;// en totaal
            svTotalResult[pair].average = average;      // save it
        }
        svTotalResult[pair].bonus=totalBonus;           //  and bonus
        if (totalBonus)
            bBonus = TRUE;                              // for display
    }   // end for all pairs

    svTotalRankToPair.resize(maxPair+1ULL);
    std::iota(svTotalRankToPair.begin(), svTotalRankToPair.end(), 0);   // fill with 0,1,2,3....
    std::sort(svTotalRankToPair.begin()+1, svTotalRankToPair.end(),[](auto left, auto right){return svTotalResult[left].total > svTotalResult[right].total;} );

    UINT lastPair = svTotalRankToPair[m_maxPair];
    if (svTotalResult[lastPair].total == 0)
        svTotalRankToPair[m_maxPair] = 0; // pair is absent, so no rank


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
    m_txtFileResultTotal.AddLine(FMT(_("Total result%s"), bWeightedAvg ? _(" (weighted average)") : ES));
    m_txtFileResultTotal.AddLine(ES);
    wxString tmp = _("rank pairname                       ");
    for (session=1; session <= maxSession; ++session)
    {
        tmp += FMT(_("  S%-4u"),session);
    }
    tmp += FMT(_("  %s  %savg. %s"), bWeightedAvg ? wxString(" ") : _("total "), bBonus ? _("bonus  ") : ES, GetGroupResultString(0, &indexSessionPairnr, false));
    m_txtFileResultTotal.AddLine(tmp);

    for (UINT rank = 1; rank < svTotalRankToPair.size(); ++rank)
    {
        pair        = svTotalRankToPair[rank];
        totalScore  = svTotalResult[pair].total;
        if (totalScore == 0)
            continue;                   // pair did not play at all
        tmp = FMT(" %3u %s ", svTotalPairToRank[pair], DottedName(names::PairnrGlobal2GlobalText(pair)));
        for (session=1; session <= maxSession; ++session)
        {
            tempscore = sessionResults[session][pair].score;
            if (tempscore == 0)
                tmp+= FMT(" %5sa", LongToAscii2(svTotalResult[pair].average));
            else
                tmp+=FMT(" %5s ", LongToAscii2(tempscore));
        }
        wxString bonusString;
        if (bBonus)                         // yes, we have a bonus!
        {
            totalBonus = svTotalResult[pair].bonus;
            if (totalBonus)                // and this pair has it
                bonusString = FMT("%6s ",LongToAscii2(totalBonus));
            else                           // and this pair not
                bonusString = FMT("%6s ",ES);
        }
        wxString totalString;
        if (bWeightedAvg)
        {
            if (svTotalResult[pair].bWeightedAvg)
                totalString = '+';      // average mean gives a difference
            else
                totalString = ' ';      // no dif
        }
        else
            totalString = FMT("%6s%%", LongToAscii2(totalScore));
        tmp += FMT("  %s %s%5s %s",
            totalString,
            bonusString,
            LongToAscii2( RoundLong(totalScore,maxSession)),
            GetGroupResultString( names::PairnrGlobal2SessionPairnr(pair)));
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
        case 3:                 // both within limits, so simply compare (inrange) scores
            return left.score > right.score;
        default:                // both NOT within limits, compare totalscore
            return left.totalScore > right.totalScore;
    }
}   // CompareClubs()

void CalcScore::CalcClub( bool a_bTotal)
{   // results for clubs, if you have assigned pairnames to a club
    UINT                maxClubIndex= 0;
    std::vector<UINT>&  rankToPair  = a_bTotal ? svTotalRankToPair       : svSessionRankToPair;
    MyTextFile&         txtFile     = a_bTotal ? m_txtFileResultClubTotal : m_txtFileResultClubSession;
    UINT                session     = cfg::GetActiveSession();
    UINT                maxSession  = a_bTotal ? session : 1;
    wxString            header      = a_bTotal ? _("Total result") : session ? FMT(_("Result of session %u"), session): _("Session result");
    wxString            fileName    = cfg::ConstructFilename(a_bTotal ? cfg::EXT_CLUB_TOTAL : cfg::EXT_SESSION_CLUB);

    std::vector<CLUB_DATA> club;
    club.resize(cfg::MAX_CLUBNAMES+1ULL);

    UINT maxClubCount = cfg::GetMaxClubcount();
    UINT minClubCount = cfg::GetMinClubcount();
    for (UINT rank = 1; rank < rankToPair.size(); ++rank) // sum first N scores per club
    {
        UINT pair = rankToPair[rank];
        if (pair)                                       // a pair that has played
        {
            // for total, use sum of session-results for better accuracy
            // like: (x.01+x.00)/2 = x.01   and (x.01+x.01)/2=x.01, but its 'more'!
            long score = a_bTotal ? svTotalResult[pair].total : svSessionResult[pair].score;                 // score bepalen
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
    txtFile.AddLine(ES);

    tmp = FMT(_("rank count  %-*s   score     avg.  totalscore"), cfg::MAX_CLUB_SIZE, _("club-name"));
    txtFile.AddLine(tmp);

    UINT rank = 1;
    for (UINT clubIndex = 1; clubIndex <= maxClubIndex; ++clubIndex)
    {
        UINT clubCount = club[clubIndex].clubCount;
        if (clubCount == 0) continue;      // this club was not present in match

        if (clubCount > maxClubCount)
            clubCount = maxClubCount;
        tmp = FMT("%3u  %4u   %-*s  %7s%%  %5s  %7s%% (%2u, %5s%%)",
                    rank,
                    clubCount,
                    cfg::MAX_CLUB_SIZE,names::GetClubName(club[clubIndex].clubId),
                    LongToAscii2(club[clubIndex].score),
                    LongToAscii2(RoundLong(club[clubIndex].score, clubCount*maxSession)),
                    LongToAscii2(club[clubIndex].totalScore),
                    club[clubIndex].clubCount,
                    LongToAscii2(RoundLong(club[clubIndex].totalScore,club[clubIndex].clubCount))
                );
        txtFile.AddLine(tmp);
        ++rank; // we COULD check if next/previous score is equal, and then emit equal rank....
    }

    if (club[0].clubCount)
    {   // pairs, not a member of a club
        txtFile.AddLine(ES);
        tmp = FMT(_("individual pairs: %u, score: %6s%% (%5s)"),
            club[0].clubCount,
            LongToAscii2(club[0].totalScore),
            LongToAscii2(RoundLong(club[0].totalScore,club[0].clubCount*maxSession))
        );
        txtFile.AddLine(tmp);
    }

    txtFile.Flush();    // write to disk
}   // CalcClub()

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

    if (svSessionResult[pair].maxScore == 0)
    {
        ADDLINE(_("Pair has not played (yet)"));
        m_choiceResult = ResultPair;
        ShowChoice();
        return;
    }
    UINT sumPoints  = 0;
    UINT sumTops    = 0;
    for (UINT game = 1; game <= maxGame; ++game)
    {
        tmp.Printf(_("Game %2u: "), game+firstGame);
        PlayerInfo playerInfo;
        if (!GetPlayerInfo(pair, game, playerInfo))
        {
            ADDLINE(tmp + _(" not played"));
            if ((game % setSize) == 0)
                ADDLINE(ES);
            continue;
        }
        UINT top    = playerInfo.bIsNS ? svGameTops[game].topNS : svGameTops[game].topEW;
        if  (top == 0) continue;    // no top yet, only playd once??
        long score  = playerInfo.score;
        long points = GetSetResult(pair, game, 1);
        sumPoints  += points;
        sumTops    += top;
        tmp        += playerInfo.bIsNS ? _("NS") : _("EW");
        tmp        += FMT(_(", score: %5s"), score::ScoreToString(score)  );
        tmp        += FMT(_(", points%6s" ), LongToAscii1(points)         );
        tmp        += FMT(  " %3ld%%"      , (points*10+top/2)/top        );

        if ((game % setSize) == 1)  // first game of set, show opponent
            tmp += FMT(_(" Played against (%s): %s"), names::PairnrSession2SessionText(playerInfo.opponent), names::PairnrSession2GlobalText(playerInfo.opponent));
        if ((game % setSize) != 0)
        {
            ADDLINE(tmp);   // output info about this game
        }
        else 
        {   // last game of a set: show total mp's and score for this set
            ADDLINE( tmp + FMT(_(" set-score: %s(%u%%)"), LongToAscii1(sumPoints), (sumPoints*10+sumTops/2)/sumTops));
            sumPoints   = 0;
            sumTops     = 0;
            ADDLINE(ES);
        }
    }

    long        score       = svSessionResult[pair].score;
    wxString    corrections = GetSessionCorrectionString(pair);
    corrections.Replace(" ", ES, true);
    if (!corrections.empty()) corrections = ' ' + corrections;
    ADDLINE(FMT(_("points: %s (%i), endscore: %s%%%s, rank: %u"),
        LongToAscii1(svSessionResult[pair].points),
                        svSessionResult[pair].maxScore,
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

    const auto& scores      = (*score::GetScoreData())[game];   // scores for this game
    int         sets        = scores.size();
    auto        setsPerLine = std::min(3, sets);                // max n scores/line
    wxString    tmp;

    for (int count = 1; count <= setsPerLine; ++count)
    {
        tmp += FMT(_("  NS   EW  SCORE       "));
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
