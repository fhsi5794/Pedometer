<?php
$link = mysqli_connect("127.0.0.1","root","1013","miniPro")
or die("無法開啟MySQL資料庫連接!<br/>");
if (isset($_POST["time"])) {

  /*抓不到啦
  $time=$_POST["time"];
  echo "上次更新時間：".$time;
  echo "<p>";
  echo date('Y-m-d',$time);
  */

  $time=time();
  $step=$_POST["step"];
  $date=date('Y-m-d',$time);
  echo "更新時間：".$date."<br>";
  $total_records=0;
  mysqli_query($link, 'SET NAMES utf8');   //送出UTF8編碼的MySQL指令
  $sql = "SELECT * FROM stepRec WHERE date='";  // 建立SQL指令字串
  $sql.= $date."'";
  $result = mysqli_query($link, $sql);  // 執行SQL查詢
  $total_records = mysqli_num_rows($result);  //row數
  // 是否有查詢到使用者記錄
  if ( $total_records > 0 )
  {
  $sql = "UPDATE stepRec SET steps='";
  $sql.= $step."'";
  $sql.= "  WHERE date='";  // 建立SQL指令字串
  $sql.= $date."'";
  $link->query($sql);
  mysqli_close($link);  // 關閉資料庫連接
}
else
{
  $sql = "INSERT INTO stepRec(date, steps) values('";
  $sql.=$date."','";
  $sql.=$step."')";
  $link->query($sql);
  mysqli_close($link);  // 關閉資料庫連接
  }
}


?>
