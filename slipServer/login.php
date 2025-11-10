<!--
   Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
   Distributed under the MIT License (http://opensource.org/licenses/MIT)
-->
<?php
  if ( session_status() === PHP_SESSION_NONE )
  {
    session_start();
    include "language.php";
  }
  $bForced = basename($_SERVER['SCRIPT_FILENAME']) == "login.php";
?>
<!DOCTYPE html>
<html lang="nl en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
    <meta http-equiv="Pragma" content="no-cache" />
    <meta http-equiv="Expires" content="0" />
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title><?php echo $ins_title;?></title>
</head>
<script>
    const ROUND       = "<?php echo $ins_sessionIdRound  ?>"; // index for sessionStore
    const TABLE       = "<?php echo $ins_sessionIdTable  ?>";
    const FROUND      = "<?php echo $ins_sessionIdFRound ?>";
    const GROUP       = "<?php echo $ins_sessionIdGroup  ?>";
    let bForced       = "<?php echo $bForced             ?>";

    if ( !bForced && Number(sessionStorage.getItem(ROUND)) >= 1 )
    { // can't go to selection again! If really needed, restart browser.
      window.stop();
      history.forward();
    }
</script>
<body id="ID_body">

<h1><?php echo $ins_description;?></h1>
<h3 id="info"></h3>
<?php
   if ( isSet($inputError) )
     echo '<h2 style="color:red"> ' . $inputError . '</h2>';
?>
<h2><?php echo $ins_selectGroupTable;?></h2>

<style>
/* body, .select, select, button, .input {font-size: 3vw}*/
body {background:Gainsboro;}
 .flex-container {
  display: flex;
  flex-direction: row;
/*  background-color: DodgerBlue;*/
  width:50%;
/*  border: 1px solid black;*/
}
/*
.flex-container > div {
  background-color: #f1f1f1;
  width: 40%;
  margin: 2px;
  text-align: left;
  font-size: 100%;
}*/
</style>

<form action="/nextSlip.php" method="post">
  <input hidden type="text" name="round" value="0">
<?php

  if ( $bForced )
  {  // possibility to add/change the next round
    echo '
      <div class="flex-container">
         <div style="width:15ex">' . $ins_roundColumn . '</div>
         <div><input id="ID_fRound" class="input" oninput="OnForcedRound(this)" style="width:15%" type="text" name="forcedRound" value="0">  </div>
      </div><br><br><br>
    ';
  }

  // $ins_groupNames is one based, so entry 0 is a dummy
  if ( count($ins_groupNames)-1 > 1 )  // more then one group, add group-selection
  {  echo '
<div class="flex-container">
  <div style="width:15ex">'; echo $ins_groupColumn; echo '</div>
  <div>
     <select id="ID_groupSelect" onchange="OnGroupChanged()" name="group" required class="select" > 
        <option value="" style="display: none"></option>'; // <- end echo(), php now active!
        for ( $group = 1; $group <= count($ins_groupNames)-1; $group++)
        {  // append the real groupnames
           echo '<option value="' . $group . '">' . $ins_groupNames[$group] . '</option>';
        } echo '
     </select>
  </div>
</div>
<br>';}
else
echo '<input hidden type="text" name="group" value="1"></input>';
?>

<div class="flex-container">
  <div style="width:15ex"><?php echo $ins_tableColumn;?></div>
  <div>
     <select id="ID_tableSelect" onchange="OnTableChanged()" name="table" required class="select" >
        <option value="" style='display: none'></option>
<?php
    for ( $table = 1; $table <= MAX_TABLES; $table++)
       echo "<option value=\"$table\">$table</option>";
?>
     </select>
  </div>
</div>
<br><br>
    <input hidden type="text" name="slipresult" value="{}">
    <button id="ID_submit" type="submit"><?php echo $ins_submitLogin;?></button>
</form>
<br><br><br><br><br><br><br><br><br><br><br><br>

<script>
<?php
  $comma = '';
  echo '  let groupTables = [';
  for ( $groups = 1; $groups < count($slipData); $groups++)
  {
     echo $comma . count($slipData[$groups][1])-1;
     $comma = ',';
  }
  echo ']; /* nr of tabels in each group */
'
?>
  OnInitMatch();
  OnResize();
  window.addEventListener('resize', OnResize, true);

  function OnResize()
  { // if screenwidth is greater then screenheight (laptop??) then adapt scale a bit for better view
    let width  = window.screen.width;
    let height = window.screen.height;
    let scale  = height/width;
    if ( scale > 1.0 ) scale = 1.0;
    let data = 'width='+ width*scale + ',initial-scale=' + scale;
    //document.getElementById("info").innerHTML="scale=" + scale;

    let elements = [ "ID_body", bForced ? "ID_fRound" : "",  "ID_groupSelect", "ID_submit", "ID_tableSelect" ];
    for (const id of elements)
    {
      if ( id != "" )
        document.getElementById(id).style.fontSize = scale*4 + 'vw';
    }
    if (0) for ( let index = 0; index < elements.length; ++index)
    {
      let id = elements[index];
      if ( id != "" )
        document.getElementById(id).style.fontSize = scale*4 + 'vw';
    }
  }  // OnResize()

  function OnGroupChanged()
  {
    let group       = document.getElementById("ID_groupSelect").value;
    let maxTable    = groupTables[group-1];
    let tableSelect = document.getElementById("ID_tableSelect");
    if ( tableSelect.value > maxTable ) // check max table for this group
      tableSelect.value = 0;	        // reset selection */
    sessionStorage.setItem(GROUP, group);
  }

  function OnTableChanged()
  {
    let group = document.getElementById("ID_groupSelect").value;
    if ( group > 0 )
    {
      let maxTable    = groupTables[group-1];
      let tableSelect = document.getElementById("ID_tableSelect");
      if ( tableSelect.value > maxTable ) // check max table for this group
        tableSelect.value = maxTable;	  // top selection */
      sessionStorage.setItem(TABLE, tableSelect.value);
    }
  }

  function OnForcedRound(evt)
  {
     sessionStorage.setItem(FROUND, evt.value);
  }  // OnChangedRound()

  function OnInitMatch()
  {
    {
       let sessionId = "BridgeWx";
       let date = new Date();
       let id   =   (date.getFullYear()).toString() + '-'
                  + (date.getMonth()+1) .toString() + '-'
                  + (date.getDate())    .toString() + '-'
                  + <?php echo '"' . $ins_match . '"';?>
                ;
       let idSaved = sessionStorage.getItem(sessionId) || "";
       let bFirstRun = (idSaved != id);
       if ( 0 ) // !bFirstRun )
       {  // data is for this match
         let round       = sessionStorage.getItem(ROUND);
         let table       = sessionStorage.getItem(TABLE);
         let group       = sessionStorage.getItem(GROUP);
         let forcedRound = sessionStorage.getItem(FROUND);
         let x=0;
       }
       else
       {  // new match data
          sessionStorage.clear()
          sessionStorage.setItem(sessionId, id);
          sessionStorage.setItem(ROUND    , 0);  // initialised later through 'slip.php' in 'nextSlip.php'
          sessionStorage.setItem(TABLE    , 0);  // initialised later
          sessionStorage.setItem(GROUP    , 0);  // initialised later
          sessionStorage.setItem(FROUND   , 0);  // initialised later ?
       }
    }
  }
</script>
</body>
</html>