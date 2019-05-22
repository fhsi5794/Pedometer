<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <title>DBtaster</title>
  <?PHP
  // 建立MySQL連結
  $link = mysqli_connect("127.0.0.1","root","1013","miniPro")
  or die("無法開啟MySQL資料庫連接!<br/>");
  ?>
</head>
<body>
  <!-- action="submit後要跳轉的頁面 " -->
  <form action="DBtaster.php" method="post" >
    <div>
      <br>
      <label for="date">選擇日期</label><br>
      <input type="date" name="date" value="2017-02-03" id="date" required autofocus/>
      <br>
      <input type="submit" value="送出"/><p>
      </div>
    </form>
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
          echo $row['steps'];
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
