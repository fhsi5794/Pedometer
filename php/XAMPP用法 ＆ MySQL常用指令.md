XAMPP用法 ＆ MySQL常用指令
===
# XAMPP用法
1. 下載
2. start，Apache、MySQL伺服器記得要開
3. 初次使用mysql
    (1) 打開xampp的終端
    (2) 輸入指令
    ```    %mysql -u root```
    ```%use mysql;```
    ```%update user set password=PASSWORD("你的密碼") where User='root'; ```
    *注意！設置密碼時，要記得加上引號
   ``` %flush privileges;```
    ```%quit```
4. 之後使用mysql
    (1) 打開xampp的終端
    (2)```%mysql -u root -p ```
    (3)輸入你要做的事 ex: ```insert into stepRec(date, steps) values(‘2017–02-11’,6633); ``` 
5. 找到xampp\htdocs\，在資料夾下建資料夾，裡面放index.php,xxx.php......
6. 網址為
    http://localhost/資料夾名稱/		   （index.php是主頁可以不用打）
    or
    http://localhost/資料夾名稱/xxx.php 

---
# mySQL常用指令

登陸
```$mysql -u root -p ```

加資料到table裡面
```$insert into stepRec(date, steps) values(‘2017–02-11’,6633); ```

從table選出全部(*)
```$select * from tableName;```

可參考：
http://note.drx.tw/2012/12/mysql-syntax.html
