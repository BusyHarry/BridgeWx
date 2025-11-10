<!--
   Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
   Distributed under the MIT License (http://opensource.org/licenses/MIT)
-->
<?php
  /**
     this file should only be activated through a 'post' action
     REMARK: $group/$round/$table are 1 based: for easier reading/handling!
     REMARK: $slipData[g][r][t] is also 1 based: so group N is at $slipData[N] etc
     be sure that, on start of a match, the $dataFile does not exist, else new data is appended!
  **/
  session_start();
  include "language.php";                           // get generated strings/values for this match
  $setSize                 = SET_SIZE;              // doesn't change
  $session                 = ACTIVE_SESSION;        // doesn't change
  $logFile                 = $ins_logFile;          // default file for info/error msg's
  $dataFile                = $ins_slipResultsFile;  // default file for the submitted data
  $clientId                = "?.?";                 // id for a client 'group.table'
  $bTestLock               = false;                 // set to true to log construction/destruction
  $pLocker                 = new MyLocker();        // create a lock for this file to prevent concurrent access to $dataFile

  const ERROR_NETWORK      = -1;                    // no network active
  const ERROR_TABLE_IN_USE = 1;                     // login for already logged-in table
  const ERROR_BAD_RESULT   = 2;                     // combination of result-data not ok
  const CONNECTION         = "pConnection";         // index in $_SESSION[]
  InitOutputFiles();                                // set '$dataFile' and $logFile to usable location

  if (    !isset($_POST['table'])
       || !isset($_POST['round'])
       || !isset($_POST['group'])
       || !isset($_POST['slipresult'])
     )
  { // something wrong: just restart!
    $inputError = "'" . htmlspecialchars(basename($_SERVER["PHP_SELF"])) . "' " . $ins_noDirectStart;
    DoLog($inputError);
    include 'index.php';
    return;
  }

  $table = +filter_var($_POST['table'], FILTER_VALIDATE_INT);  // doesn't change anymore
  $group = +filter_var($_POST['group'], FILTER_VALIDATE_INT);  // doesn't change anymore
  $round = +filter_var($_POST['round'], FILTER_VALIDATE_INT);

/*** check if values inrange **/
  if (  // slipData: [Groups][Rounds][Tables]
          $group < 1 || $group >= count($slipData)
       || $round < 0 || $round >= count($slipData[$group])
       || $table < 1 || $table >= count($slipData[$group][1])
     )
  { // something wrong: just restart!
    $inputError = "$ins_unexpectedInput: $ins_table: $table, $ins_group: $group, $ins_round: $round";
    DoLog($inputError);
    include 'index.php';
    return;
  }

  $clientId  = "$group.$table";

  if ( $round < 1 )
  { // we are 'posted' from login.php/index.php
    // table/group/forcedRound are selected in there
    // in other rounds, these are just 'passed-through'
    CheckNetworkConnection();
    $forcedRound = isset($_POST['forcedRound']) ? +filter_var($_POST['forcedRound'], FILTER_VALIDATE_INT) : 0;
    $result = AppendResult("login for group: " . $group
                             . ", table: "     . $table
                             . ", groups: "    . count($slipData)-1
                             . ", fRound: "    . $forcedRound
                             . ", rounds: "    . NR_OF_ROUNDS
                             . ", games: "     . NR_OF_ROUNDS*SET_SIZE
                          );
    if ( $result != SLIP_E_NONE && $result != ERROR_NETWORK )
    { // do something....
      DoLog("$ins_error $result: $ins_group $group, $ins_table $table, $ins_round $round");
      // echo "$ins_badInput: roep arbiter<br>";
      // $round = NR_OF_ROUNDS; // force end
    }
    $round = ( $forcedRound == 0 ) ? 0 : ($forcedRound - 1);
    // group/table/round defined now
  }
  else
  { // round >= 1, gameresults expected
    // verify expected values
    if (    $_SESSION[$ins_sessionIdGroup] != $group
         || $_SESSION[$ins_sessionIdTable] != $table
         || $_SESSION[$ins_sessionIdRound] != $round
       )
    {
      $inputError = "$ins_tableErrorInfo: $ins_group: $group $ins_table: $table $ins_round: $round";
      if ( $round == $_SESSION[$ins_sessionIdRound] - 1 )
      {
        DoLog("$ins_browserRefresh: $inputError");
      }
      else
      {
        DoLog($inputError);
        echo '<h2 style="color:red"> ' . $inputError . '</h2>';
        // redo input for this table and use backuped values...
      }

      $round = $_SESSION[$ins_sessionIdRound] - 1;
      $table = $_SESSION[$ins_sessionIdTable];
      $group = $_SESSION[$ins_sessionIdGroup];
    }
    else
    {
      $slipresult = $_POST['slipresult'];  //FILTER_SANITIZE_STRING);
      $nl         = "\n";
      $nsId = $slipData[$group][$round][$table][INDEX_NS];
      $ewId = $slipData[$group][$round][$table][INDEX_EW];
      $msg =    'session: '    . $session
            . ', group: '      . $group
            . ', table: '      . $table
            . ', round: '      . $round
            . ', ns: '         . $nsId
            . ', ew: '         . $ewId
            . ', slipresult: ' . $slipresult
            ;
      // store posted game-data
      AppendResult($msg);
    }
  }   // Round >= 1

  // variables to be calculated for next round:
  //    $firstGame, $ns, $ew, $round, $set
  $round += 1; // prepare info for next round
  for (;;)
  { // dummy loop, so you can easily restart/exit
    if ( $round > NR_OF_ROUNDS )
    {
       AppendResult("ready session: $session, group: $group, table: $table, round: " . NR_OF_ROUNDS);
       include 'ready.php';
       break;
    }

    $set = $slipData[$group][$round][$table][INDEX_SET];
    if ( $set == 0 )
    {  // no-play table
       $round++;
       continue;
    }
    $nsId        = $slipData[$group][$round][$table][INDEX_NS];
    $ewId        = $slipData[$group][$round][$table][INDEX_EW];
    $ns          = $pairNames[$nsId];
    $ew          = $pairNames[$ewId];
    $firstGame   = (($set-1) * $setSize) + 1;
    $sScoreEntry = $ins_scoreEntry;
    if ( count($ins_groupNames)-1 > 1 )
       $sScoreEntry .= ' ' . $ins_group . ' ' . $ins_groupNames[$group] . ' ';

    // save values, so we can check them when we get here again
    $_SESSION[$ins_sessionIdGroup] = $group;	// we expect these values on next call
    $_SESSION[$ins_sessionIdTable] = $table;
    $_SESSION[$ins_sessionIdRound] = $round;
    DoLog("$ins_setSessionData $ins_group($group), $ins_table($table), $ins_round($round)");
    include 'slip.php'; //setup next slip for this group/table
    break;
  }  // for(;;)

  return;  // end of php code

  // only function definitions follow
  function InitOutputFiles()
  { // if default '$dataFile'/'$logFile does not exist, create and initialise it (on error, create/init it locally)
    // assume same location for both files!
    global $dataFile;
    global $logFile;
    $sessionFileData = "datafile";	// can't have 'const' in a function...
    $sessionFileLog  = "logfile";

    if ( isset($_SESSION[$sessionFileData]) )
    {
       $dataFile = $_SESSION[$sessionFileData];
       $logFile  = $_SESSION[$sessionFileLog ];
       return;
    }

    if ( !file_exists($dataFile) )
    { // (try to) create it
      $fmt = ";slipresult format: {<gamenr>, <declarer>, <level>, <suit>, <over/under tricks>, <doubled>, <NSscore>}";
      AppendResult($fmt);
      if ( !file_exists($dataFile) )
      {  // can't create file on wanted location, create it locally
         $dataFile = basename($dataFile);
         if ( !file_exists($dataFile) )
         {  // assume it can be created locally
            AppendResult($fmt);
         }
         $logFile = basename($logFile); // assume same for logfile...
      }
    }

    // set sessiondata outside of loop to be sure its set!
    $_SESSION[$sessionFileData] = $dataFile;
    $_SESSION[$sessionFileLog ] = $logFile;
  }  // InitOutputFiles()

  function CheckResult($msg)
  { /**
      check if result is acceptable:
      - login: table is not loged-in yet
      - scores: if combination of session/round/table/ns/ew/games is ok
      - ready: if all data is present
    **/
    
    return SLIP_E_NONE;
  }  // CheckResult()

  function AppendResult($msg)
  {
     global $dataFile;
     global $clientId;
     global $ins_result;

     $msg2 = sprintf("%s %s %s %s\n", date("Y.m.d"), date("H:i:s"), $clientId, $msg );
     $result = CheckResult($msg2); // check data to be submitted
     file_put_contents($dataFile, $msg2, FILE_APPEND);
     $result = Send2Laptop($msg2); // send over network, if connection present
     DoLog("$ins_result=$result: '$msg'");
     return $result;
  }  // AppendResult()

  function DoLog($msg)
  {
     global $logFile;
     global $clientId;
     $msg = sprintf("%s %s %s %s\n", date("Y.m.d"), date("H:i:s"), $clientId, $msg );
     file_put_contents($logFile, $msg, FILE_APPEND);
  }  // DoLog()

  function GetHash($id)
  {
    return password_hash($id, PASSWORD_DEFAULT);
  }  // GetHash()

  function VerifyHash($id, $hash)
  {
    return  password_verify($id, $hash);
  }  // VerifyHash()

  function GetLocalIp4()
  { // "Pinging laptop-BTO17 [192.168.2.46] with 32 bytes of data:"
    $ip = "ip not found!";
    $tmp = `ping -4 -n 1 laptop-BTO17`;
    $pos1 = strpos($tmp,"[");
    if ( $pos1 != false )
    {
      ++$pos1;
      $pos2 = strpos($tmp,"]");
      $ip   = substr($tmp, $pos1, $pos2-$pos1);
    }
    return $ip;
  }  // GetLocalIp4()

  function Send2Laptop($msg)
  {
    global $ins_laptopName;
    if (    !isset($_SESSION[CONNECTION])
         || "" ==  $_SESSION[CONNECTION]
       ) return ERROR_NETWORK;  // no connection setup
    //DoLog("Send2Laptop(): connection possible");
    $fp = @fsockopen($ins_laptopName, SERVER_PORT, $errno, $errstr, 2.0);
    if ( !$fp ) return ERROR_NETWORK; // connection lost, just continue using result-file
    stream_set_timeout($fp, 2);
    // msg: <id>,<len>,<msg>, <len> is length of <msg>
    $out = chr(SERVER_MSG_ID) . chr(+strlen($msg)) . $msg;
    fwrite($fp, $out);
    $error  = SLIP_E_BAD_CMD;
    $result = "";
    $len    = 0;
    // incoming msg: <id>,<err>,<len>,<msg>, where <len> is length of <msg>
    $id = ord(fread($fp, 1));
    if ( $id == SERVER_MSG_ID )
    {
      $error = ord(fread($fp, 1));
      $len   = ord(fread($fp, 1));
      if ( $len ) $result = fread($fp, $len);
      if ( $error != SLIP_E_NONE )
        DoLog("$ins_error: $error, $msg -> $result");
    }
    else
    {
       $result = $ins_errorNetworkRespons;
       DoLog("$msg -> $result: id= $id");
    }

    //echo "server response: id=$id, error=$error, len=$len --> $result<br>";
    fclose($fp);
    return $error;
  } // Send2Laptop()

  function CheckNetworkConnection()
  {
    global $ins_laptopName;
    global $ins_errorNetworkOpen;

    $_SESSION[CONNECTION] = "";  // default no network connection present/possible
    $fp = @fsockopen($ins_laptopName, SERVER_PORT, $errno, $errstr, 2.0);
    if ( $fp )
    {
      $_SESSION[CONNECTION] = "ok";
      fclose($fp);
    }
    else
    {
      $msg = "<br>$ins_errorNetworkOpen: $ins_laptopName $errstr ($errno)<br>";
      // echo $msg;
      DoLog($msg);
    }
    // $xx = $_SESSION[CONNECTION];
    // echo "CheckNetworkConnection(): $xx <br>";
  } // CheckNetworkConnection()

  class MyLocker 
  { // locks this .php file till it exits somehow
    // so we can safely access our data-file(s)
    private $fp;

    function __construct()
    {
      global $bTestLock;
      if ( $bTestLock ) print "Constructing " . __CLASS__ . "...";
      $lockFile = "lock.txt";
      if ( !file_exists($lockFile) )
      {  // create the file so we can open it lateron for read-only
         $this->fp = fopen($lockFile, 'w');
         if ( $this->fp ) fclose($this->fp);
      }
      // now the file exists, created by us or someone else....
      $this->fp = fopen($lockFile, 'r');
      if ( $this->fp )
      { // should be ok: acquire an exclusive lock and update database
        if ( !flock($this->fp, LOCK_EX) )
        {
          $msg = "Couldn't get the lock for lock.txt!";
          echo $msg;
        }
      }
      if ( $bTestLock ) print "done<br>";
    }  // MyLocker()

    function __destruct()
    {  // NB runs at end of .php file, NOT when leaving scope!
       // last created, first destroyed!
       // NO file actions are executed anymore (so no logging possible)!
       global $bTestLock;
       if ( $bTestLock ) print "Destroying " . __CLASS__ . "...";
       flock($this->fp, LOCK_UN);
       fclose($this->fp);
       if ( $bTestLock ) print "done<br>";
    } // ~MyLocker
  }   // MyLocker
?>
