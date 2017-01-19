#include <LGPS.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>

#define Drv LSD           // use SD card
LFile myFile;
LGPRSClient client;
gpsSentenceInfoStruct info;

char server[] = "dweet.io";
int port = 80;
char* thing ="aumgps%2Fanongsincar%2Fcar6445";
int i = 0;
char buff[256],data[256],response[256];

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
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    latitude *= 0.01;                  // change xxxx.xxxx to xx.xxxxxx
    lat01 = (int)latitude+0.000001;    // change xxxx.xxxx to xx.xxxxxx
    lat02 = latitude-lat01;            // change xxxx.xxxx to xx.xxxxxx
    latitude = lat01 + lat02*100/60;   // change xxxx.xxxx to xx.xxxxxx   
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    longitude *= 0.01;                 // change xxxxx.xxxx to xxx.xxxxxx
    lon01 = (int)longitude+0.000001;   // change xxxxx.xxxx to xxx.xxxxxx
    lon02 = longitude-lon01;           // change xxxxx.xxxx to xxx.xxxxxx
    longitude = lon01 + lon02*100/60;  // change xxxxx.xxxx to xxx.xxxxxx 
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);
    hour += 7;                         // My Time Zone +7
    if (hour >= 24) hour -= 24;        // reset hour between 0 to 23     
    sprintf(buff,"UTC:%2d-%2d-%2d lat=%9.6f lon=%10.6f sat=%d", hour, minute, second, latitude, longitude, num);
    sprintf(data,"lat=%9.6f&lon=%10.6f", latitude, longitude); //Serial.println(buff); 
  }else{
    sprintf(buff,"Not get data"); 
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(10, OUTPUT);
  Drv.begin();
  if (Drv.exists("gpsdata.txt")) {// Check to see if the file exists:
    Drv.remove("gpsdata.txt"); // delete the file:
  }
  LGPS.powerOn();
  Serial.println("LGPS Power on");
  Serial.println("Attach to GPRS network by auto-detect APN setting");
  while (!LGPRS.attachGPRS("everywhere", "eesecure", "secure"))
  {
    delay(500);
  }
}

void loop() {
  LGPS.getData(&info);
  parseGPGGA((const char*)info.GPGGA);
  myFile = Drv.open("gpsdata.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(buff);
    myFile.close();
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
  } else Serial.println("error open file gpsdata.txt");
  while (client.available())
  {
    char c = client.read();   //client.read()
    response[i] = c;
    i++;
  }
  if(!client.connected()) client.stop();
  delay(4000);
}
