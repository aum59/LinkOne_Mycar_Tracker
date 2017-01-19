#include <LGPS.h>
#include <LDateTime.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>

#define Drv LSD           // use SD card
datetimeInfo t;
LFile myFile;
LGPRSClient client;
gpsSentenceInfoStruct info;
unsigned int rtc,rtcOld,myDate,Timer;
char buff[256],data[256],response[256],stName[40];
double latOld,longOld,latNew,longNew;

char server[] = "dweet.io";
int port = 80;
char* thing ="aumgps%2Fanongsincar%2Fcar6445";
const unsigned int centerDate = 1483228801;
const unsigned int delayMode8 = 15;   //delay TEST Mode 15sec.
const unsigned int delayMode7 = 86400;//delay 86400sec. = 1 day
const unsigned int delayMode6 = 43200;//delay 43200sec. = 12 hour
const unsigned int delayMode5 = 43200;//delay 21600sec. = 6 hour
const unsigned int delayMode4 = 43200;//delay 10800sec. = 3 hour
const unsigned int delayMode3 = 43200;//delay  3600sec. = 1 hour
const unsigned int delayMode2 = 43200;//delay  1800sec. = 30 min.
const unsigned int delayMode1 = 900;  //delay   900sec. = 15 min.
const unsigned int delayMode0 = 0;    // = 0 delay in Loop 2sec.

int count,i = 0;

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
  double latitude,lat01,lat02;
  double longitude,lon01,lon02;
  int tmp, hour, minute, second, num;
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');   
    hour += 7;    // My Time Zone +7
    if (hour >= 24) hour -= 24; // hour between 0 to 23     
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    latitude *= 0.01;// change 1435.9785 to 14.359785    
    lat01 = (int)latitude+0.000001;    // change 1435.9785 to 14.599641
    lat02 = latitude-lat01;            // change 1435.9785 to 14.599641
    latitude = lat01 + lat02*100/60;   // change 1435.9785 to 14.599641    
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    longitude *= 0.01;      // change 10021.7643 to 100.217643
    lon01 = (int)longitude+0.000001;   // change 10021.7643 to 100.362738
    lon02 = longitude-lon01;           // change 10021.7643 to 100.362738
    longitude = lon01 + lon02*100/60;  // change 10021.7643 to 100.362738  
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);
    sprintf(buff,"UTC=%2d:%2d:%2d lat=%9.6f lon=%10.6f sat=%d", hour, minute, second, latitude, longitude, num); //Serial.println(buff); 
    sprintf(data,"lat=%9.6f&lon=%10.6f", latitude, longitude); //Serial.println(buff); 
    latNew = latitude;
    longNew = longitude;
  }else{
    sprintf(buff,"Not get data"); 
  }
}

void setup() { 
  Serial.begin(115200);
  pinMode(10, OUTPUT);
  Drv.begin();
  //Drv.exists("gpsdata.txt"); // Check the file exists
  //Drv.remove("gpsdata.txt"); // delete the file
  LGPS.powerOn();
  Serial.println("LGPS Power on");
  Serial.println("Attach to GPRS network by auto-detect APN setting");
  while (!LGPRS.attachGPRS("everywhere", "eesecure", "secure"))
  {
    delay(500);
  }
  Serial.print("Get Time rtc from GPS ");
  do{
    Serial.print(".");
    LDateTime.getRtc(&rtc);
    myDate=(rtc-centerDate)/86400+1; //myDate is a day from 1/1/2017
    delay(1000);
  }while(myDate < 4000);
}

void loop() {
  LDateTime.getRtc(&rtc);
  myDate=(rtc-centerDate)/86400+1; //mydate is a day from 1/1/2017
  sprintf(stName,"car6445date%d.txt",myDate);
  LGPS.getData(&info);
  parseGPGGA((const char*)info.GPGGA);
/*  if(latOld == latNew && longOld == longNew) count++;// gps fix same place
    else{ // gps don't fix same place
      count = 0;
      latOld = latNew;
      longOld = longNew;
    }   */
  myFile = Drv.open(stName, FILE_WRITE);
    if (myFile) {
      myFile.println(buff);
      myFile.close();
    } else Serial.println("error open file car6445dateXX.txt");
/*  if(count <= 3){//if gps fix same place 3time don't writ to SD card
    
  }   */
  Timer = rtc-rtcOld;  
  Serial.println(Timer);
  if(rtc-rtcOld >= delayMode8){
    Serial.print("Connect to ");
    Serial.println(server);
    while (!client.connect(server, port))
    {
    Serial.print(".");
    delay(500);
    }
    Serial.println("connected");
    String url = String("/dweet/for/")+ thing +"?" + data;
    client.println(String("GET ") + url + " HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    Serial.println(data);
    client.println("Connection: close");
    client.println();
    rtcOld = rtc;        
  } 
  while (client.available())
  {
    char c = client.read();   //client.read()
    response[i] = c;
    i++;
  }
  if(!client.connected()) client.stop();
  delay(1000);
}
