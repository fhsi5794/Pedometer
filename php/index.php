<html lang="zh-tw">
<head>
  <meta charset="utf-8">
  <script src="//apps.bdimg.com/libs/jquery/1.10.2/jquery.min.js"></script>
  <script src="//apps.bdimg.com/libs/jqueryui/1.10.4/jquery-ui.min.js"></script>
  <link rel="stylesheet" href="//apps.bdimg.com/libs/jqueryui/1.10.4/css/jquery-ui.min.css">
  <link rel="stylesheet" href="jqueryui/style.css">
  <title>Pedometer</title>
  <?PHP
  // 建立MySQL連結
  $link = mysqli_connect("127.0.0.1","root","an334an334","miniPro")
  or die("無法開啟MySQL資料庫連接!<br/>");
  ?>
  <style type="text/css" media="screen">
  html {
    height: 100%;
    width: 100%;
  }
  #map
  {
    position: relative;
    height: 50%;
    width: 50%;
  }
  #body
  {
    height: 100%;
    width: 100vw;
    position: relative;
    top: 0;
    left: 25%;
  }
</style>
</head>
<body>
  <div class="body">
    <h1>My pedometer</h1>
    <font size="5">現在位置</font><br>
    <div id="map"></div><p>
    <div id="MCS"></div>
    <div id="getSteps">
      <form action="index.php" method="post" >
        <div>
          <br>
          <label for="date"><font size="5">查看歷史步數</font></label><br>
          <input type="date" name="date" id="date" required autofocus/>
          <input type="submit" value="送出"/><br>
        </div>
      </form>
     </div>
    </div>
    <!--建立 google Map -->
    <script>
    function initMap() {
      var Station_latlng = { lat: 25.0136906, lng: 121.5406792 }; // 經緯度
      var map = new google.maps.Map(document.getElementById('map'), {
        zoom: 14, //放大的倍率
        center: Station_latlng //初始化的地圖中心位置
      });
      //建立一個mark
      var marker = new google.maps.Marker({
        position: Station_latlng, //marker的放置位置
        map: map //這邊的map指的是第四行的map變數
      });
    }
    </script>
    <!--記得輸入 google Map 金鑰-->
    <script async defer src="https://maps.googleapis.com/maps/api/js?key=AIzaSyBLQoezVVh9hBrCNc22FZOvQsOVA1Q6Ol8&callback=initMap">
    </script>
    <!--抓MCS資料-->
    <script type="text/javascript">
    $(document).ready(function(){
      $('#MCS').hide();
      //接收MCS資料 
      $.ajax({
        type: "GET",
        url: "https://api.mediatek.com/mcs/v2/devices/Df0bQ60Z/datachannels/clear/datapoints",
        headers: { deviceKey: "k8a8hTRy7MCjuDGE" },
        contentType: "application/json"
      })
      //收到資料
      .done(function(data){
        $('#MCS').show();
        $('#MCS').append("MCS connect.<br>");
        $('#MCS').append("----------<br>");
        var dd = data["dataChannels"][0]["dataPoints"][0]["values"]["value"];
        $('#MCS').append("清空步數:",dd,"<br>==========<br>");
    })
      $.ajax({
        type: "GET",
        url: "https://api.mediatek.com/mcs/v2/devices/Df0bQ60Z/datachannels/GPS/datapoints",
        headers: { deviceKey: "k8a8hTRy7MCjuDGE" },
        contentType: "application/json"
      })
      .done(function(data){
        $('#MCS').show();
        $('#MCS').append("----------<br>");
        var dd = data["dataChannels"][0]["dataPoints"][0]["values"]["value"];
        $('#MCS').append("位置:",dd,"<br>==========<br>");
    })
      $.ajax({
        type: "GET",
        url: "https://api.mediatek.com/mcs/v2/devices/Df0bQ60Z/datachannels/battery_level/datapoints",
        headers: { deviceKey: "k8a8hTRy7MCjuDGE" },
        contentType: "application/json"
      })
      .done(function(data){
        $('#MCS').show();
        $('#MCS').append("----------<br>");
        var dd = data["dataChannels"][0]["dataPoints"][0]["values"]["value"];
        $('#MCS').append("電池電量:",dd,"<br>==========<br>");
    })
      $.ajax({
        type: "GET",
        url: "https://api.mediatek.com/mcs/v2/devices/Df0bQ60Z/datachannels/today_step/datapoints",
        headers: { deviceKey: "k8a8hTRy7MCjuDGE" },
        contentType: "application/json"
      })
      .done(function(data){
        $('#MCS').show();
        $('#MCS').append("----------<br>");
        var dd = data["dataChannels"][0]["dataPoints"][0]["values"]["value"];
        $('#MCS').append("今日步數:",dd,"<br>==========<br>");
    })
      
    });

    </script>
    <!-- 日期選擇器 -->
    <script>
    $(function() {
      $( "#date" ).datepicker({dateFormat: 'yy-mm-dd'});
    });
    </script>
    <!-- 從DB找到對應日期的步數  -->
    <?PHP
    $date = "";
    $total_records=0;
    // 取得表單欄位值
    if ( isset($_POST["date"]) )
    $date= $_POST["date"];
    //查找資料
    if ($date != "") {
      mysqli_query($link, 'SET NAMES utf8');   //送出UTF8編碼的MySQL指令
      $sql = "SELECT * FROM stepRec WHERE date='";  // 建立SQL指令字串
      $sql.= $date."'";
      $result = mysqli_query($link, $sql);  // 執行SQL查詢
      $total_records = mysqli_num_rows($result);  //row數
      // 是否有查詢到使用者記錄
      if ( $total_records > 0 )
      {
        for ($i=0;$i<$total_records;$i++)
        {
          $row = mysqli_fetch_assoc($result);
          echo "<h1>你走了 ";
          echo $row['step'];
          echo " 步<br><font>";
        }
      }
      else
      {
        echo "<font color='red'>";
        echo "<h1>查無資料!<br><font>";
      }
      mysqli_close($link);  // 關閉資料庫連接
    }
    ?>
  </body>
  </html>
