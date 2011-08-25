<?php

#error_reporting(E_ALL);
#ini_set("display_errors", 1);

include "Dashboard.php";
$dash = new Dashboard();
$dash->print_header("Condor Build and Test Dashboard");

?>

<script type="text/javascript">

  function reset() {
      $("#menu1").css("background-color", "#dddddd");
      $("#menu2").css("background-color", "#dddddd");
      $("#menu3").css("background-color", "#dddddd");
      $("#menu4").css("background-color", "#dddddd");
      $("#menu5").css("background-color", "#dddddd");    

      $("#main1").hide();
      $("#main2").hide();
      $("#main3").hide();
      $("#main4").hide();
      $("#main5").hide();
  }

  $(document).ready(function(){

    $("#loading").ajaxStart(function(){
      $(this).show();
    }).ajaxStop(function(){
      $(this).hide();
    });

    $("#main1").load('continuous.php');
    $("#menu1").css("background-color", "#ff0000");
    
    $("#menu1").click(function(){
      $("#main1").load('continuous.php');
      
      reset();
      $(this).css("background-color", "#ff0000");
      $("#main1").show();
    });
    
    $("#menu2").click(function(){
      $("#main2").load('nightly.php');

      reset();
      $(this).css("background-color", "#ff0000");
      $("#main2").show();
    });
    
    $("#menu3").click(function(){
      $("#main3").load('one-off.php');
      
      reset();
      $(this).css("background-color", "#ff0000");
      $("#main3").show();
    });
    
    $("#menu4").click(function(){
      $("#main4").load('nmi-status.php');
      
      reset();
      $(this).css("background-color", "#ff0000");
      $("#main4").show();
    });

    $("#menu5").click(function(){
      $("#main5").load('calendar.php');
      
      reset();
      $(this).css("background-color", "#ff0000");
      $("#main5").show();
    });
    
  });
</script>

</head>
<body>

<div id="top" class="top">

  <div id="menu1" class="menu">
  Continuous Runs
  </div>

  <div id="menu2" class="menu">
  Nightly runs
  </div>
    
  <div id="menu3" class="menu">
  Personal Runs
  </div>
  
  <div id="menu4" class="menu">
  NMI Status
  </div>
  
  <div id="menu5" class="menu">
  Release Calendar
  </div>

</div>

<div id="main1" style="clear:both">
&nbsp;
</div>

<div id="main2" style="clear:both">
&nbsp;
</div>

<div id="main3" style="clear:both">
&nbsp;
</div>

<div id="main4" style="clear:both">
&nbsp;
</div>

<div id="main5" style="clear:both">
&nbsp;
</div>

<div id="loading">
  <p><img src="loading.gif" />Loading...</p>
</div>


</body>
</html>