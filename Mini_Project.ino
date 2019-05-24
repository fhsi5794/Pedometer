#include <Time.h>
#include <TimeLib.h>
#include <HttpClient.h>
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LBattery.h>
#include <LGPS.h>
#include <LDateTime.h>
#include <Wire.h>


#define WIFI_AP "I-HOME"
#define WIFI_PASSWORD "SandyL11"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define SITE_URL "api.mediatek.com"
#define DEVICEID "DIJoVbTM"
#define DEVICEKEY "bL7lysMI7SqIzSzz"

#define DEVICE (0x53)    //ADXL345 device address
#define TO_READ (6)        //num of bytes we are going to read each time (two bytes for each axis)

#define offsetX   -10.5       // OFFSET values
#define offsetY   -2.5
#define offsetZ   -4.5

#define gainX     257.5        // GAIN factors
#define gainY     254.5
#define gainZ     248.5

#define uploadPeriod 30


LWiFiClient content;

unsigned int rtc;
unsigned int lrtc;
unsigned int rtc1;
unsigned int lrtc1;
char connection_info[40];
int val = 0;

gpsSentenceInfoStruct info;
char GPSbuff[256];
char nors[2], eorw[2];
String latitude_str, longitude_str, altitude_str;

byte buff[TO_READ] ;    //6 bytes buffer for saving data read from the device
char str[512];                      //string buffer to transform data before sending it to the serial port

int x,y,z;
int xavg, yavg,zavg, steps=0, flag=0;
int xval[15]={0}, yval[15]={0}, zval[15]={0};
int threshhold = 20.0;
int accel = 0, c = 0;

String Battery;
String GPS;
String StepData;
int BatteryLength;
int GPSLength;
int StepLength;

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */
  double latitude;
  double longitude;
  double altitude;
  int tmp, hour, minute, second, num ;
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    sprintf(GPSbuff, "UTC timer %2d-%2d-%2d", hour, minute, second);
      
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(3, GPGGAstr);
    strncpy(nors,  &GPGGAstr[tmp], 1);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(5, GPGGAstr);
    strncpy(eorw,  &GPGGAstr[tmp], 1);
    convertCoords(latitude, longitude, nors, eorw);
    
    sprintf(GPSbuff, "latitude = %10.4f, longitude = %10.4f", latitude, longitude);
    Serial.println("(after) latitudet = " + latitude_str + " longitude = " + longitude_str);
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(GPSbuff, "satellites number = %d", num);
 
    tmp = getComma(9, GPGGAstr);
    altitude = getDoubleNumber(&GPGGAstr[tmp]);
    sprintf(GPSbuff, "altitude = %f", altitude);    
    altitude_str = (String)altitude;
  }
  else
  {
    Serial.println("Not get data"); 
  }
}

void convertCoords(float tmplat, float tmplong, const char* n_or_s, const char* e_or_w) { 
  /*
  Latitude  5213.2930,N --> 52d 13.2930' N
  52 degrees 13.2930 minutes NORTH
  52 + (13.2930 / 60) = 52.22155
  Because it is north of the equator, the number remains positive.
  +52.22155
  */
  
  float lat_return, lon_return;
  int lat_deg_int = int(tmplat / 100); //extract the first 2 chars to get the latitudinal degrees
  float latitude_float = tmplat - (lat_deg_int * 100); //remove the degrees part of the coordinates - so we are left with only minutes-seconds part of the coordinates
  lat_return = lat_deg_int + latitude_float / 60; //add back on the degrees part, so it is decimal degrees
  
  //Check if it is N or S, S will turn the value negative
  
  if (*n_or_s == 'S'){
    Serial.println("is South");
    lat_return *= -1;
  }

  sprintf(GPSbuff, "%.7f", lat_return);   
  latitude_str = GPSbuff;
  
  /*
  Longitude  00004.5337,W  --> 00d 04.5337' W
  00 degrees 4.5337 minutes WEST
  00 + (4.5337 / 60) = 0.0755616
  Because it is West, the number becomes negative.
  -0.0755616
  */
  
  int lon_deg_int = int(tmplong / 100);
  float longitude_float = tmplong - lon_deg_int * 100;
  lon_return = lon_deg_int + longitude_float / 60;  //add back on the degrees part, so it is decimal degrees
  if (*e_or_w == 'W'){
    Serial.println("is West");
    lon_return *= -1;
  }

  sprintf(GPSbuff, "%.7f", lon_return); 
  longitude_str = GPSbuff;
}

char* getInfo(String dataChannel, int* dataSize){
  HttpClient http(content);
  // keep retrying until connected to website
  Serial.println("Connecting to WebSite");
  while (0 == content.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }

  //calling RESTful API to get TCP socket connection
  content.print("GET /mcs/v2/devices/");
  content.print(DEVICEID);
  content.print("/datachannels/");
  content.print(dataChannel);
  content.println("/datapoints.csv HTTP/1.1");
  content.print("Host: ");
  content.println(SITE_URL);
  content.print("deviceKey: ");
  content.println(DEVICEKEY);
  content.println("Connection: close");
  content.println();
  
  delay(500);

  int errorcount = 0;
  while (!content.available())
  {
    Serial.println("waiting HTTP response: ");
    Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      content.stop();
      return 0;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  Serial.print("Content length is: ");
  Serial.println(bodyLen);
  int index = 0; 
  while (content)
  {
    int v = content.read();   
    if (v != -1)
    {
      Serial.print((char)v);
      connection_info[index] = (char)v;
      index++;
    }
    else
    {
      Serial.println(" ");
      Serial.println("no more content, disconnect");
      content.stop();
    }    
  }
  *dataSize = index;
  return connection_info;
}

boolean disconnectedMsg = false;

void sendValue(String data, int dataLength){

  int errCount = 0;
  while (0 == content.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    errCount++;
    if(errCount > 5)
      return;
    delay(1000);
  }
  
  content.print("POST /mcs/v2/devices/");
  content.print(DEVICEID);
  content.println("/datapoints.csv HTTP/1.1");
  content.println("Host: api.mediatek.com");
  content.print("deviceKey: ");
  content.println(DEVICEKEY);
  content.print("Content-Length: ");
  content.println(dataLength);
  content.println("Content-Type: text/csv");
  content.println("Connection: close");
  content.println();
  content.println(data);
  
  while (content)
  {
    int v = content.read();
    if (v != -1)
    {
      Serial.print((char)v);
    }
    else
    {
      Serial.println("no more content, disconnect");
      content.stop();
    }
  }

  if (!disconnectedMsg)
  {
    Serial.println("disconnected by server");
    disconnectedMsg = true;
  }
  delay(500);
}


//---------------- Functions
//Writes val to address register on device
void writeTo(int device, byte address, byte val) {
   Wire.beginTransmission(device); //start transmission to device 
   Wire.write(address);        // send register address
   Wire.write(val);        // send value to write
   Wire.endTransmission(); //end transmission
}

//reads num bytes starting from address register on device in to buff array
void readFrom(int device, byte address, int num, byte buff[]) {
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission
  
  Wire.beginTransmission(device); //start transmission to device
  Wire.requestFrom(device, num);    // request 6 bytes from device
  
  int i = 0;
  while(Wire.available())    //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}


//Get pedometer.

int ArduinoPedometer(){
    int acc=0;
    int totvect[15]={0};
    int totave[15]={0};
    int xaccl[15]={0};
    int yaccl[15]={0};
    int zaccl[15]={0};
    for (int i=0;i<15;i++)
    {
      xaccl[i]= x;
      delay(1);
      yaccl[i]= y;
      delay(1);
      zaccl[i]= z;
      delay(1);
      totvect[i] = sqrt(((xaccl[i]-xavg)* (xaccl[i]-xavg))+ ((yaccl[i] - yavg)*(yaccl[i] - yavg)) + ((zval[i] - zavg)*(zval[i] - zavg)));
      totave[i] = (totvect[i] + totvect[i-1]) / 2 ;
      delay(150);
      Serial.print("totave:");
      Serial.println(totave[i]);
      Serial.print("accel:");
      Serial.println(accel);
      if( (totave[i] - accel) > threshhold)
        steps++;
      accel = totave[i];
      return(steps);
    }
  delay(100); 
 }
//更新當下時間
 void updateTime(){
  //利用傳送一次資訊來更新成當下時間
  int index = 0, comma;
  char* info = getInfo("today_step", &index);
  String tmp="";
  comma = getComma(2, info);
  for(int i = comma; i < index;i++)
    tmp += connection_info[i];
  steps = tmp.toInt();
  tmp = "today_step,," + (String)steps;
  sendValue(tmp, tmp.length());
  Serial.println("#update current time.");
 }
 
//比對日期
void compareDate(){
  int index = 0;
  char* info = getInfo("newDate", &index);
  int lastUpdateDate, lastUpdateStepDate;
  time_t StepDate;
  int comma1,comma2;
  String tmp ="";
  //上次上傳的時間
  comma2 = getComma(2, info);
  for(int i = comma2; i < index; i++)
    tmp += info[i];
  lastUpdateDate = tmp.toInt();
  //這次上傳的時間
  index = 0;
  info = getInfo("today_step", &index);
  Serial.print("index:");
  Serial.println(index);
  comma1 = getComma(1, info);
  comma2 = getComma(2, info);
  tmp ="";
  for(int i = comma1; i < comma2 - 4; i++)
    tmp += info[i];
  StepDate = tmp.toInt();
  Serial.println(hour(StepDate));
  if(hour(StepDate)+8 >= 24)
    lastUpdateStepDate = day(StepDate) + 1;
  else
    lastUpdateStepDate = day(StepDate);
  Serial.print("Last update date:");
  Serial.println(lastUpdateDate);
  Serial.print("Last update steps date:");
  Serial.println(lastUpdateStepDate);
  //不一樣的話更新上傳時間，步數歸零
  if(lastUpdateDate != lastUpdateStepDate){
    String updateDate = "newDate,," + (String)lastUpdateStepDate;
    sendValue(updateDate, updateDate.length());
    steps = 0;
    Serial.println("#Steps reset.");
  }
  //一樣的話繼續增加步數
  else{
    tmp="";
    for(int i = comma2; i < index;i++)
      tmp += connection_info[i];
    steps = tmp.toInt();
    Serial.println("#Steps start counting.");
  }
  
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin();        // join i2c bus (address optional for master)  
  LTask.begin();
  LWiFi.begin();
  Serial.begin(9600);
  
//Turning on the ADXL345
  writeTo(DEVICE, 0x2D, 0);      
  writeTo(DEVICE, 0x2D, 16);
  writeTo(DEVICE, 0x2D, 8);
  LGPS.powerOn();
  Serial.println("LGPS Power on, and waiting ..."); 
  delay(100);

  // keep retrying until connected to AP
  Serial.println("Connecting to AP");
  while (!LWiFi.connectWPA(WIFI_AP,WIFI_PASSWORD))
  {
    Serial.println("Wifi connect faild.");
    delay(100);
  }

  
  //先上傳一次來更新當下時間
  updateTime();
  delay(5000);
  //做日期比對及更新步數初始值
  compareDate();
  delay(1000);
}

void loop()
{
  int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345  
  readFrom(DEVICE, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345  
   //each axis reading comes in 10 bit resolution, ie 2 bytes.  Least Significat Byte first!!
   //thus we are converting both bytes in to one int
  x = (((int)buff[1]) << 8) | buff[0];   
  y = (((int)buff[3])<< 8) | buff[2];
  z = (((int)buff[5]) << 8) | buff[4];
 
  c = ArduinoPedometer();
  Serial.print("Steps:");
  Serial.println(c);
  delay(50);
  Serial.print("GPS:"); 
  LGPS.getData(&info);
  Serial.println((char*)info.GPGGA); 
  parseGPGGA((const char*)info.GPGGA);
  delay(50);
  Serial.print("Battery level:");
  Serial.println(LBattery.level());
  delay(50);
 
  //Check for report datapoint status interval
  LDateTime.getRtc(&rtc1);
  if ((rtc1 - lrtc1) >= uploadPeriod) {
    StepData = "today_step,,"+(String)c;
    StepLength = StepData.length();
    sendValue(StepData, StepLength);
    delay(100);
    GPS = "GPS,," + latitude_str + "," + longitude_str + "," + altitude_str;
    //GPS = "GPS,,49.734,15.014,104.5";
    GPSLength = GPS.length();
    sendValue(GPS, GPSLength);
    delay(100);
    Battery = "battery_level,,"+(String)LBattery.level();
    BatteryLength = Battery.length();
    sendValue(Battery, BatteryLength);
    delay(100);
    Serial.println("#Send values.");
    lrtc1 = rtc1;
  }
  
  
  delay(100);  
}
