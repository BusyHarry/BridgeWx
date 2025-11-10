/***
   Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
   Distributed under the MIT License (http://opensource.org/licenses/MIT)
***/
/* all javascript variables and functions used in slip.php */
let gameSelected    = false;
let activeGameCell  = null;
let activeGameIndex = 0;
let backGroundColor = "";
let activeGameRow   = null;
let bDoLog          = false;     // if true, extra logging to debugconsole in browser

/**
  The S* vars are arrays of S(trings).
  The E* vars are meant as E(nums), to be used to index the S-vars.
**/
//REMARK: don't change order of EDeclarer, code depends on this order!
const EDeclarer = {DUMMY: "0", NP:"1", PASS:"2", N   : "3", E  : "4", S   : "5", W : "6" };
// 'SDeclarer' created by php: const SDeclarer = [""        , sBidNP, sBidPass, sBidNorth, sBidEast, sBidSouth, sBidWest];

const EContract = {ONE : "1", TWO : "2", THREE: "3", FOUR: "4", FIVE:"5", SIX:"6", SEVEN:"7"};
const SContract = []; // we can directly use the enum-value

const ESuit     = {CLUBS:"0", DIAMONDS:"1", HEARTS:"2", SPADES:"3", NT:"4"     };
// 'SSuit' created by php: const SSuit     = ["&clubs;", "&diams;"   , "&hearts;", "&spades;", sBidNoTrump];

// M... = minus, P... = plus
const ETricks   = {M13:"0", M12:"1",M11:"2",M10:"3",M9:"4",M8:"5",M7:"6",
                   M6:"7", M5:"8", M4:"9", M3:"10", M2:"11", M1:"12",
                   P0:"13", P1:"14", P2:"15", P3:"16", P4:"17", P5:"18", P6:"19"
                  };
const STricks   = ["-13", "-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1",
                   "="  , "+1" , "+2" , "+3" , "+4", "+5", "+6"]; 

const EDoubled  = {X0:"0", X1:"1", X2:"2"};
const SDoubled  = [ "-"  , "X"   , "XX"  ];

/**
 - gdS(tring) and gdI(nteger) are used as short-hand for slipDataX[y]
 - initialised in InitSlipData() or restored from sessiondata
**/
let gdS;
let gdI;
let slipDataI;
let slipDataS;
const LOG       = "log";        // id's for sessionStorage
const COUNT     = "count"; 
const SLIPTABLE = "sliptable";
const SLIPI     = "slipI";
const SLIPS     = "slipS";
const EMPTY     = "emptyslip";

let declarerNoScore = false;    // true, if we have a declarer without gamedata

//const tableHeader = ["", sDeclarer    , sContract    , sSuit    , "+/- " + sTricks, sDoubled    , sResultNs];
const                      colDeclarer=1, colContract=2, colSuit=3,      colTricks=4, colDoubled=5, colResultNs=6;

function GetSessionLog()
{
   return sessionStorage.getItem(LOG) || "";
}  // GetSessionLog()

function DoLog( msg )
{
   if ( !bDoLog ) return;
   // get session-data
   let log = sessionStorage.getItem(LOG) || "";
   let cnt = Number(sessionStorage.getItem(COUNT));
   if ( cnt > 10 )
   {
     cnt = 0;
     log = "";
   }
   cnt += 1;
   log += cnt + " " + msg + ", active round: " + sessionStorage.getItem(ROUND) + "\n";
   // update session-data
   sessionStorage.setItem(LOG, log);
   sessionStorage.setItem(COUNT, cnt);
   console.log(log);
}  // DoLog()

function OnSignOk()
{
  let bAllReady =    ValidateGameData()
                  && document.getElementById("IDC_pairNS").checked 
                  && document.getElementById("IDC_pairEW").checked;
   /* testing */ bAllReady = true;
   //console.log(gdI);
  document.getElementById("IDB_submit").style.display = bAllReady ? "" : "none";
}  // OnSignOk()

function ValidateGameData()
{ // check if all games have valid/complete data
  for ( let game = 0; game < slipDataS.length; game++)
  {
     let dataS = slipDataS[game];
     let dataI = slipDataI[game];
     if ( dataS.declarer == "" )
       return false;  // nothing entered yet
     if (    (dataI.declarer >= EDeclarer.N)
          && ( (dataS.contract == "") || (dataS.suit == "") )
        )  // no contractlevel or suit entered
        return false;
  }
  return true;
}  // ValidateGameData()

function OnSubmit(evt)
{
  // prepare data to submit, called just before the 'post' is done
  let results = BuildResults();  // get score-results as string to be submitted
  let dummy = document.getElementById("IDT_slipresult");
  document.getElementById("IDT_slipresult").value = results;
  DoLog("OnSubmit()");
} // OnSubmit()

function OpenFullscreen()
{
  let elem = document.documentElement;
  if (elem.requestFullscreen)
  {
    elem.requestFullscreen();
  }
  else
  {
    if (elem.webkitRequestFullscreen)
    { /* Safari */
      elem.webkitRequestFullscreen();
    }
    else
    {
      if (elem.msRequestFullscreen)
      { /* IE11 */
        elem.msRequestFullscreen();
      }
    }
  }
}  // OpenFullscreen()

function IsVulnerable( game, ns )
{ // game is 1-based, vulnerability repeats each 16 tables
  // ns-tables:  2,4,5,7,10,12,13,15
  // ew-tables:  3,4,6,7,9,10,13,16
  const nsData = [0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0];
  const ewData = [0,0,1,1,0,1,1,0,1,1,0,0,1,0,0,1];
  game = (game - 1) & 0xf;
  let vulnerable = ns ? nsData[game] : ewData[game];
  return vulnerable;
}  // IsVulnerable()

function OnOk()
{  // set active row to not-selected
  if ( gameSelected ) // && (activeGameRow != null) )
  {
    gameSelected = false;
    activeGameRow.style.backgroundColor = backGroundColor;
  }
}  // OnOk()

function OnGameSelect(a_gameCell)
{ // when clicked on a game in the slip-table
  if ( activeGameCell == null )  // save once original background
     backGroundColor = a_gameCell.parentNode.style.backgroundColor;
  else if ( activeGameCell != a_gameCell )
  { // restore original background
    activeGameRow.style.backgroundColor = backGroundColor;
    gameSelected = false;
  }
  activeGameRow = a_gameCell.parentNode;
  gameSelected = !gameSelected;
  activeGameCell = a_gameCell;
  activeGameRow.style.backgroundColor = gameSelected ? "cyan" : backGroundColor;
  activeGameIndex = a_gameCell.id;
  gdI = slipDataI[activeGameIndex];   // this is a reference!
  gdS = slipDataS[activeGameIndex];   // this is a reference!
  UpdateDeclarerNoScore();
}  // OnGameSelect()

function SaveEmptySlipTable()
{  // called when sliptable is empty at startup of page
   // used for clearing the slip when page is hiding
   let empty = document.getElementById(ID_SLIP).innerHTML;
   sessionStorage.setItem(EMPTY, empty);
}  // SaveEmptySlipTable()

function InitSlipData()
{ // prepare an empty slip
  gdS = { declarer:"", contract:"", suit:"", tricks:"", doubled:"", vulnerable:"", score:""};
  gdI = { declarer:0 , contract:0 , suit:0 , tricks:0 , doubled:0 , vulnerable:0 , score:0 };
  slipDataI = [];
  slipDataS = [];

  for (let game = 1; game <= ini_nrOfGames; game++)
  {  // initialise slip data, use 'structuredClone()' to get a 'deep' clone iso a reference!
    slipDataI.push(structuredClone(gdI));
    slipDataS.push(structuredClone(gdS));
  }

  document.getElementById("IDC_pairNS").checked = false;
  document.getElementById("IDC_pairEW").checked = false;
}  // InitSlipData()

function SessionSlipClear()
{ // remove all slipdata when page is hiding (preventing 'go back')
  sessionStorage.removeItem(SLIPTABLE);
  sessionStorage.removeItem(SLIPI);
  sessionStorage.removeItem(SLIPS);
}  // SessionSlipClear()

function SessionSlipRestore()
{ // re-initialize current data or clear it
  let slip = sessionStorage.getItem(SLIPTABLE) || "";
  if ( slip != "" )
  {  // we have stored data, rebuild sliptable and the variables slipDataI/slipDataS
    document.getElementById(ID_SLIP).innerHTML = slip;
    let slipI = sessionStorage.getItem(SLIPI);
    let slipS = sessionStorage.getItem(SLIPS);
    slipDataI = JSON.parse(slipI);
    slipDataS = JSON.parse(slipS);
  }
  else
    InitSlipData();  // just clear our data
  let x = 1;
}  // SessionSlipRestore()

function SessionSlipSave()
{ // save slipinfo in sessiondata to use it when SAME page is re-shown
  OnOk();  // de-select a selected game

  let slipTable = document.getElementById(ID_SLIP).innerHTML;
  sessionStorage.setItem(SLIPTABLE, slipTable);

  let slipI = JSON.stringify(slipDataI);
  let slipS = JSON.stringify(slipDataS);
  sessionStorage.setItem(SLIPI, slipI);
  sessionStorage.setItem(SLIPS, slipS);

  // now data is saved: clear data so each new show will begin 'fresh'
  document.getElementById(ID_SLIP).innerHTML = sessionStorage.getItem(EMPTY);  // clear slip
  InitSlipData(); // clear slipdata

/****
  let t1 = document.getElementById(ID_SLIP);
  let t2 = t1.children;
  let t3 = t2[1];  //tbody
  let t4 = t3.children;  // n rows
  let t5 = t4[1].cells;  // 7 columns
  let slipRows = document.getElementById(ID_SLIP).children[1].children;
  let slipInfo=[];
  let rowCount = slipRows.length;
  let cellCount = slipRows[0].children.length;
  for ( let row = 1; row < rowCount; ++row)
  {  // 0 = header
     let cellInfo=[];
     for ( let cell = 1; cell < cellCount; ++cell)
     {  // 0 = gameId
        cellInfo.push(slipRows[row].children[cell].innerHTML);
     }
     slipInfo.push(structuredClone(cellInfo));
  }
  let test = JSON.stringify(slipInfo);
  let x = 0;
****/
}  // SessionSlipSave()

function ClearSlipData(msg)
{ if (1) return;
  if (sessionStorage.getItem(ROUND) == ini_round )
    return;	// last/active round
  let cnt = sessionStorage.getItem(COUNT) || 0;
  cnt += 1; sessionStorage.setItem(COUNT, cnt);
  let log = sessionStorage.getItem(LOG) || "";
  log += "\n"+ cnt + ' ' + msg+' ' + ini_round ;
  sessionStorage.setItem(LOG, log);
  console.log(log);

  document.getElementById("ID_match" ).innerHTML = sMatch + " counter: " + cnt;

  ClearContractData();
  slipDataI = [];
  slipDataS = [];
  slipTable = document.getElementById(ID_SLIP);
  for (let i=1; i <= ini_nrOfGames; i++)
  {  // initialise slip data, use 'structuredClone()' to get a 'deep' clone iso a reference!
    slipDataI.push(structuredClone(gdI));
    slipDataS.push(structuredClone(gdS));
    for (let col = colDeclarer; col <= colResultNs; col++)
    {
       // childNodes[0] == caption, childNodes[1] == header, childNodes[2] == first game
       slipTable.childNodes[i+1].cells[col].innerHTML = "";
    }
  }
  document.getElementById("IDC_pairNS").checked = false;
  document.getElementById("IDC_pairEW").checked = false;
  // history.forward();
} // ClearSlipData()

function ClearContractData()
{  // only clear the data, no slip-update
  gdS.declarer = gdS.contract = gdS.suit = gdS.tricks = gdS.doubled = gdS.vulnerable = gdS.score = "";
  gdI.declarer = gdI.contract = gdI.suit = gdI.tricks = gdI.doubled = gdI.vulnerable = gdI.score = 0;
} // ClearContractData()

function OnClearContract()
{
  ClearContractData();
  UpdateDeclarerNoScore();
  UpdateScoreSlip();
  OnSignOk();
}  // OnClearContract()

function UpdateScoreSlip()
{ // update slip with new data as result of new/changed declarer/level/suit/tricks/doubled
  if ( !gameSelected ) return;

  if ( 7 + gdI.contract + gdI.tricks < 0 )
  {  // force silently data in range
     gdI.tricks = -(7+gdI.contract);  // can only loose as many tricks as the contract is!
     gdS.tricks = gdI.tricks;
  }
  if ( 7 + gdI.contract + gdI.tricks > 13 )
  {  // force silently data in range
     gdI.tricks = 13 - (7 + gdI.contract);  // can only have max 13 tricks!
     gdS.tricks = "+" + gdI.tricks;
  }

  if ( !declarerNoScore )
  { // check if we have a declarer, else simulate 'Nord'
    let decl = (gdI.declarer != 0) ? (gdI.declarer - EDeclarer.N) : 0;
    try
    {
      gdI.score = CalcScore(gdI.contract, gdI.suit, gdI.doubled, decl, gdI.vulnerable, gdI.tricks);
    } catch(err) {gdI.score = 0;}
    gdS.score = gdI.score;
  }
  activeGameRow.cells[colDeclarer].innerHTML = gdS.declarer; // + (gdI.vulnerable ? "*" : "");
  activeGameRow.cells[colContract].innerHTML = gdS.contract;
  activeGameRow.cells[colSuit]    .innerHTML = gdS.suit;
  activeGameRow.cells[colTricks]  .innerHTML = gdS.tricks;
  activeGameRow.cells[colDoubled] .innerHTML = gdS.doubled;
  activeGameRow.cells[colResultNs].innerHTML = gdS.score;
  activeGameRow.cells[colDeclarer].style.backgroundColor = gdI.vulnerable ? "RED" : backGroundColor; //"BLACK";
  activeGameRow.cells[colSuit]    .style.color = ( gdS.suit == SSuit[ESuit.DIAMONDS] || gdS.suit == SSuit[ESuit.HEARTS] ) ? "RED" : "BLACK";
}  // UpdateScoreSlip()

function UpdateDeclarerNoScore(a_declarer)
{
  let isDefined = (typeof a_declarer) != "undefined";
  let declarer = isDefined ? a_declarer : gdI.declarer;
  declarerNoScore = (declarer == EDeclarer.PASS) || (declarer == EDeclarer.NP || (declarer == EDeclarer.DUMMY) );
  if ( declarerNoScore & isDefined )
     ClearContractData();
  if ( declarer == EDeclarer.PASS )
    gdS.score = "0";
}  // UpdateDeclarerNoScore()

function OnDeclarer(a_declarer)
{
  if ( !gameSelected ) return;
  UpdateDeclarerNoScore( a_declarer );
  gdS.declarer = SDeclarer[a_declarer];
  gdI.declarer = Number(a_declarer);
  if ( gameSelected && !declarerNoScore )
  {
    let ns = !!(gdI.declarer & 1);
    gdS.vulnerable = IsVulnerable( Number(activeGameIndex) + Number(ini_firstGame), ns);
    gdI.vulnerable = Number(gdS.vulnerable);
  }
  UpdateScoreSlip();
  OnSignOk();
}  // OnDeclarer()

function OnContract(a_contract)
{
  if ( !gameSelected ) return;
  if ( declarerNoScore ) return;
  gdS.contract = a_contract;
  gdI.contract = a_contract - 1;
  UpdateScoreSlip();
  OnSignOk();
} // OnContract()

function OnSuit(a_suit)
{
  if ( !gameSelected ) return;
  if ( declarerNoScore ) return;
  gdS.suit = SSuit[a_suit];
  gdI.suit = Number(a_suit);	// force to number!
  UpdateScoreSlip();
  OnSignOk();
}  // OnSuit()

function OnTricks(a_tricks)
{
  if ( !gameSelected ) return;
  if ( declarerNoScore ) return;
  gdS.tricks = STricks[a_tricks];
  if ( a_tricks == ETricks.P0 )
    gdI.tricks = 0;  // P0 shows as "=", so its not a number...
  else
    gdI.tricks = Number(gdS.tricks);
  UpdateScoreSlip();
// console.log(gdI);
}  // OnTricks()

function OnDoubled(a_doubled)
{
  if ( !gameSelected ) return;
  if ( declarerNoScore ) return;
  gdS.doubled = SDoubled[a_doubled];
  gdI.doubled = Number(a_doubled);
  UpdateScoreSlip();
  //console.log(gdI);
}  // OnDoubled()

function GetBuildResultFormat()
{
  return "{<gamenr>,<declarer>,<level>,<suit>,<over/under tricks>,<doubled>,<NSscore>}";
}  // GetBuildResultFormat()

function BuildResults()
{  // create resultstring for entered scores
   // format per game: "{<gamenr>,<declarer>,<level>,<suit>,<over/under tricks>,<doubled>,<NSscore>}"
   // all values number/enum
   let result = "";
   let separator  = "";
   for ( let game = 0; game < ini_nrOfGames; game++)
   {
      let slip = slipDataI[game];
      result += separator +`{${game+ini_firstGame}, ${slip.declarer}, ${slip.contract+1}, ${slip.suit+1}, ${slip.tricks}, ${slip.doubled}, ${slip.score}}`;
      separator = '@';  
   }
   return result;
} // BuildResults()