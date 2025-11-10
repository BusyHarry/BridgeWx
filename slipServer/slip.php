<!--
   Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
   Distributed under the MIT License (http://opensource.org/licenses/MIT)
-->
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1"> 
    <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
    <meta http-equiv="Pragma" content="no-cache" />
    <meta http-equiv="Expires" content="0" />
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <link  href="./slip.css" crossorigin="anonymous" rel="stylesheet" type="text/css"/>
    <script src="./slip.js">     </script>   <!-- pagedata and methods used  -->
    <script src="./calcScore.js"></script>   <!-- score calculation for a given bidding -->
</head>
<body>
<script>
    <?php /* initialisation by php */ ?>
    /* input data for the different tables 'in' = input, 'i' = integer, 's' = string */
    let ini_round     =  <?php echo $round               ?>;
    let ini_session   =  <?php echo $session             ?>;
    let ini_nrOfGames =  <?php echo $setSize             ?>;
    let ini_firstGame =  <?php echo $firstGame           ?>;
    let ini_table     =  <?php echo $table               ?>;
    const ROUND       = "<?php echo $ins_sessionIdRound  ?>"; // index for serssionStore
    const TABLE       = "<?php echo $ins_sessionIdTable  ?>";
    const FROUND      = "<?php echo $ins_sessionIdFRound ?>";
    const GROUP       = "<?php echo $ins_sessionIdGroup  ?>";
    const ID_SLIP     = "ID_slip"; // id of slip-table

    const SDeclarer = /* keep order same as bidding table! */
    [""<?php echo
       ', "' . $ins_bidNP 
    . '", "' . $ins_bidPass
    . '", "' . $ins_bidNorth
    . '", "' . $ins_bidEast
    . '", "' . $ins_bidSouth
    . '", "' . $ins_bidWest
    . '"];';?>

    const SSuit = ["&clubs;", "&diams;", "&hearts;", "&spades;", "<?php echo $ins_bidNoTrump;?>"];
    /* end of input data */

    window.scrollTo(0,0);
    let tbl       = Number(sessionStorage.getItem(TABLE));
    let round     = Number(sessionStorage.getItem(ROUND));
    let bNewRound = false;
    if ( ini_round > round )
    {  // starting new round, so clear old data
       SessionSlipClear();       
       bNewRound = true;
       sessionStorage.setItem(ROUND, ini_round);
       round = ini_round;
       sessionStorage.setItem(TABLE, ini_table);
       DoLog("body script is running new round");
    }
    else if ( ini_round < round )
         {
            DoLog("body script is running an older round: " + ini_round);
            window.stop();
            history.forward();
         }
         else
            DoLog("body script is running latest round");
</script>

<?php /******
<!-- ********* begin tooltip example *********** add a '-'to enable tooltip example ->
<p>
    <a  class="tooltip">
    I am a 
        <span> (This website rocks) </span></a>&nbsp; a developer.
</p>

 <a class="tooltip">  <button>test</button><span>hallo </span> </a>
<p>
    <a  class="tooltip">
    nr2 
        <span> 2e website </span></a>&nbsp; a friend.
</p>
< !-- ********* end tooltip example *********** -->
******/ ?>
<h1 id="ID_match"  style="font-size: 2.7vw"><?php echo $ins_description?></h1>
<h2 id="ID_header" style="font-size: 2.7vw"><?php echo $sScoreEntry . ' ' . $ins_table . ' ' . $table . ' ' . $ins_round . ' ' . $round ?></h2>
<table id="ID_slip" class="slip">
  <caption><?php echo $ins_slipCaption ?> </caption>
  <tr> <!-- tableHeader: don't change order, the col* vars depend on it -->
    <th></th>
    <th>    <?php echo $ins_declarer ?></th>
    <th>    <?php echo $ins_contract ?></th>
    <th>    <?php echo $ins_suit     ?></th>
    <th>+/- <?php echo $ins_tricks   ?></th>
    <th>    <?php echo $ins_doubled  ?></th>
    <th>    <?php echo $ins_resultNs ?></th>
  </tr>
  <?php
     for ($game = 0; $game < $setSize ; ++$game)
     {
       echo
'  <tr>
    <td onclick="OnGameSelect(this)" id="' . $game . '">' . $ins_game . ' ' . $game + $firstGame . ':</td>
    <td></td>
    <td></td>
    <td></td>
    <td></td>
    <td></td>
    <td></td>
  </tr>
';
    } ?>
</table>
<br>

<!-- ************** begin entry of bidding-data ************* -->
<div class="flexStyle">
  <!-- ok/clear+check -->
  <div class="ok_clear_check">
    <!-- buttons ok/clear -->
    <div class="spacerSmall"> </div>    <button class="btnOkClear" id="IDB_apply" title="<?php echo $ins_applyTip?>" OnClick="OnOk()"><?php echo $ins_apply?></button>
    <div class="spacer"> </div>
    <button class="btnOkClear" id="IDB_clear" OnClick="OnClearContract()" \><?php echo $ins_clear?></button>
    <button id="IDB_fullScreen" style="display:none" onclick="openFullscreen()"><?php echo $ins_fullScreen?></button>
  </div>  <!-- buttons ok/clear -->
  <fieldset id="IDC_checkOk" style="width:85%; border:solid black 1px;" >
    <!-- entry for signing ok -->
  <legend id="IDC_agree"><?php echo $ins_checkAgree?></legend>
  <div>
    <input type="checkbox" style="transform:scale(1.5)" id="IDC_pairNS" name="IDC_pairNS" onclick="OnSignOk()"/>
    <label id="IDL_pairNS" for="IDC_pairNS"><?php echo $ns?></label>
  </div> <!-- with <div>, checkboxes are on separate lines -->

  <div>
    <input type="checkbox" style="transform:scale(1.5)" id="IDC_pairEW" name="IDC_pairEW" onclick="OnSignOk()"/>
    <label id="IDL_pairEW" for="IDC_pairEW"><?php echo $ew?></label>
  </div>

    <form action="/nextSlip.php" method="post">
  <!-- TODO determine real data -->
  <input hidden type="text" name="round"      value=<?php echo $round;       ?> >  
  <input hidden type="text" name="table"      value=<?php echo $table;       ?> >
  <input hidden type="text" name="group"      value=<?php echo $group;       ?> >
  <input hidden type="text" name="slipresult" value="{}" id="IDT_slipresult">
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <button id="IDB_submit" onclick="OnSubmit();" style="display:none" ><?php echo $ins_submit;?></button>
    </form>
  </fieldset>
  <br>
</div> <!-- ok/clear+check -->
<br>

 <table id="IDT_bidding" class = "bidding" >
  <tr>
    <th                  >   <?php echo $ins_declarer?></th>
    <th                  >   <?php echo $ins_contract?></th>
    <th                  >   <?php echo $ins_suit    ?></th>
    <th style="width:30%">+/-<?php echo $ins_tricks  ?></th>
    <th                  >   <?php echo $ins_doubled ?></th>
  </tr>
  <tr>
    <td> <button class="buttonDeclarer"  onclick="OnDeclarer(EDeclarer.NP   )"><?php echo $ins_bidNP?>     </button> </td>
    <td> <button class="buttonContract"  onclick="OnContract(EContract.ONE  )">1                           </button> </td>
    <td> <button class="buttonSuit"      onclick="OnSuit    (ESuit.CLUBS    )">&clubs;                     </button> </td>
    <td> <div class="div_p0m7"><button class="button_p0m7" onclick="OnTricks(ETricks.P0 )">=               </button> <button class="button_p0m7"                                                                          onclick="OnTricks(ETricks.M7 )">-7 </button></div></td>
    <td> <button class="buttonDoubled"   onclick="OnDoubled (EDoubled.X0    )">-                           </button> </td>
  </tr>
  <tr>
    <td> <button class="buttonDeclarer"  onclick="OnDeclarer(EDeclarer.PASS )"><?php echo $ins_bidPass?>   </button> </td>
    <td> <button class="buttonContract"  onclick="OnContract(EContract.TWO  )">2                           </button> </td>
    <td> <button class="buttonSuit"      onclick="OnSuit    (ESuit.DIAMONDS )" style="color:red">&diams;   </button> </td>
    <td> <button class="buttonTricks"    onclick="OnTricks  (ETricks.P1     )">+1                          </button> <button class="buttonTricks" onclick="OnTricks(ETricks.M1)">-1</button> <button class="buttonTricks" onclick="OnTricks(ETricks.M8 )">-8 </button></td>
    <td> <button class="buttonDoubled"   onclick="OnDoubled (EDoubled.X1    )">X                           </button> </td>
  </tr>
  <tr>
    <td> <button class="buttonDeclarer"  onclick="OnDeclarer(EDeclarer.N    )"><?php echo $ins_bidNorth?>  </button> </td>
    <td> <button class="buttonContract"  onclick="OnContract(EContract.THREE)">3                           </button> </td>
    <td> <button class="buttonSuit"      onclick="OnSuit    (ESuit.HEARTS   )" style="color:red">&hearts;  </button> </td>
    <td> <button class="buttonTricks"    onclick="OnTricks  (ETricks.P2     )">+2                          </button> <button class="buttonTricks" onclick="OnTricks(ETricks.M2)">-2</button> <button class="buttonTricks" onclick="OnTricks(ETricks.M9 )">-9 </button></td>
    <td> <button class="buttonDoubled"   onclick="OnDoubled (EDoubled.X2    )">XX                          </button> </td></td>
  </tr>
  <tr>
    <td> <button class="buttonDeclarer"  onclick="OnDeclarer(EDeclarer.E    )"><?php echo $ins_bidEast?>   </button> </td>
    <td> <button class="buttonContract"  onclick="OnContract(EContract.FOUR )">4                           </button> </td>
    <td> <button class="buttonSuit"      onclick="OnSuit    (ESuit.SPADES   )">&spades;                    </button> </td>
    <td> <button class="buttonTricks"    onclick="OnTricks  (ETricks.P3     )">+3                          </button> <button class="buttonTricks" onclick="OnTricks(ETricks.M3)">-3</button> <button class="buttonTricks" onclick="OnTricks(ETricks.M10)">-10</button></td>
  </tr>
  <tr>
    <td> <button class="buttonDeclarer"  onclick="OnDeclarer(EDeclarer.S    )"><?php echo $ins_bidSouth?>  </button> </td>
    <td> <button class="buttonContract"  onclick="OnContract(EContract.FIVE )">5                           </button> </td>
    <td> <button class="buttonSuit"      onclick="OnSuit    (ESuit.NT       )"><?php echo $ins_bidNoTrump?></button> </td>
    <td> <button class="buttonTricks"    onclick="OnTricks  (ETricks.P4     )">+4                          </button> <button class="buttonTricks" onclick="OnTricks(ETricks.M4)">-4</button> <button class="buttonTricks" onclick="OnTricks(ETricks.M11)">-11</button></td>
  </tr>
  <tr>
    <td> <button class="buttonDeclarer"  onclick="OnDeclarer(EDeclarer.W    )"><?php echo $ins_bidWest?>   </button></td>
    <td> <button class="buttonContract"  onclick="OnContract(EContract.SIX  )">6                           </button> </td>
    <td></td>
    <td> <button class="buttonTricks"    onclick="OnTricks  (ETricks.P5     )">+5                          </button> <button class="buttonTricks" onclick="OnTricks(ETricks.M5)">-5</button> <button class="buttonTricks" onclick="OnTricks(ETricks.M12)">-12</button></td>
  </tr>
  <tr>
    <td></td>
    <td> <button class="buttonContract"  onclick="OnContract(EContract.SEVEN)">7                           </button> </td>
    <td></td>
    <td> <button class="buttonTricks"    onclick="OnTricks  (ETricks.P6     )">+6                          </button> <button class="buttonTricks" onclick="OnTricks(ETricks.M6)">-6</button> <button class="buttonTricks" onclick="OnTricks(ETricks.M13)">-13</button></td>
  </tr>
</table> 

<!-- ************** end definition of bidding-data ************* -->

<script>
SaveEmptySlipTable();
if ( bNewRound )
   InitSlipData();
else
   SessionSlipRestore();
if (1) window.onpageshow = function(event)
{
    if ( event.persisted )
    {
       DoLog("onpageshow(bf-cache)");
    }
    else
    {
       DoLog("onpageshow(no bf-cache)");
    }
    history.forward();
}  // onpageshow()

if (1) window.onpagehide = function(event)
{
  SessionSlipSave();
  DoLog("onpagehide");
  history.forward();
}  // onpagehide()

/************* testing: see bak\slip_test.php ******/

</script>
</body>
</html>
