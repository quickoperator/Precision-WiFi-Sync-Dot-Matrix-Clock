#include <NTPClient.h>
#include <ESP8266WiFi.h> //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <WiFiUdp.h> //handles sending and receiving of UDP packages
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

WiFiClient client;

#define NUM_MAX 4

// for NodeMCU 1.0    Vcc rouge   Gnd brun
#define CS_PIN  0  // D3 jaune
#define CLK_PIN 14  // D5 vert
#define DIN_PIN 13  // D7 orange

#include "max7219.h"
#include "fonts.h"
#define HOSTNAME "WIFI_CLOCK"
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "ca.pool.ntp.org", -18000,60000);//-18000 = -5 heures EST 60000 est l'interval d'update

void setup() 
{
  Serial.begin(115200);
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN,1);
  sendCmdAll(CMD_INTENSITY,5);
  int set = 10;
  Serial.println(" ");
  Serial.println("Connecting WiFi ");
  printStringWithShift("Connection",15);
  while (WiFi.status() != WL_CONNECTED) {
    set--;
    delay(500);
    Serial.print(".");
      if(set<=0) {
        printStringWithShift("             ",15);
        delay(1000);
        printStringWithShift("Setup",15);
        delay(1000);
        printStringWithShift("             ",15);
        delay(1000);
        printStringWithShift("Please connect to : Wifi Clock Setup",30);
        delay(1000);
        printStringWithShift("             ",15);
        delay(1000);
        printStringWithShift("NoWiFi",15);
        WiFiManager wifiManager;
        wifiManager.resetSettings();
        wifiManager.autoConnect("WiFi Clock Setup");
      }  
   }

  Serial.println("");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
  printStringWithShift("             ",15);
  printStringWithShift((String("  IP: ")+WiFi.localIP().toString()).c_str(), 15);
  printStringWithShift("             ",15);
  delay(1500);
}


// =============================DEFINE VARS==============================
#define MAX_DIGITS 20
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int h,m,s;
// =======================================================================

void loop()
{
  if(updCnt<=0) { // every 10 scrolls, ~450s=7.5m
    updCnt = 600;
    printStringWithShift("Ajustement de l'heure...",15);
    printStringWithShift("             ",15);
    timeClient.update();
    Serial.println("Heure Serveur NTP");
    Serial.println(timeClient.getFormattedTime());
    clkTime = millis();
  }
 
  if(millis()-clkTime > 60000 && !del && dots) {
    updCnt--;
    clkTime = millis();
  }
  if(millis()-dotTime > 500) {
    dotTime = millis();
    dots = !dots;
  }
  AjustTime();
  showAnimClock();
}

// =======================================================================

void AjustTime()
{
  if (digitalRead(5) == HIGH) {
  h = timeClient.getHours(); // Heure normale (hiver)
  } else {
  h = timeClient.getHours()+1; // Heure avancé (été)
  }
  if (h == 24) {
  h = 0;  
  }
  m = timeClient.getMinutes();
  s = timeClient.getSeconds();
}

// =======================================================================

void showAnimClock() 
{
  byte digPos[6]={0,8,17,25,34,42};
  int digHt = 12;
  int num = 6; 
  int i;
  if(del==0) {
    del = digHt;
    for(i=0; i<num; i++) digold[i] = dig[i];
    dig[0] = h/10 ? h/10 : 10;
    dig[1] = h%10;
    dig[2] = m/10;
    dig[3] = m%10;
    dig[4] = s/10;
    dig[5] = s%10;
    for(i=0; i<num; i++)  digtrans[i] = (dig[i]==digold[i]) ? 0 : digHt;
  } else
    del--;
  
  clr();
  for(i=0; i<num; i++) {
    if(digtrans[i]==0) {
      dy=0;
      showDigit(dig[i], digPos[i], dig6x8);
    } else {
      dy = digHt-digtrans[i];
      showDigit(digold[i], digPos[i], dig6x8);
      dy = -digtrans[i];
      showDigit(dig[i], digPos[i], dig6x8);
      digtrans[i]--;
    }
  }
  dy=0;
  setCol(15,dots ? B00100100 : 0);
  setCol(32,dots ? B00100100 : 0);
  refreshAll();
  delay(30);
}

// =======================================================================
//Sous routines requises par showAnimClock
// =======================================================================

void showDigit(char ch, int col, const uint8_t *data) 
{
  if(dy<-8 | dy>8) return;
  int len = pgm_read_byte(data);
  int w = pgm_read_byte(data + 1 + ch * len);
  col += dx;
  for (int i = 0; i < w; i++)
    if(col+i>=0 && col+i<8*NUM_MAX) {
      byte v = pgm_read_byte(data + 1 + ch * len + 1 + i);
      if(!dy) scr[col + i] = v; else scr[col + i] |= dy>0 ? v>>dy : v<<-dy;
    }
}

void setCol(int col, byte v) 
{
  if(dy<-8 | dy>8) return;
  col += dx;
  if(col>=0 && col<8*NUM_MAX)
    if(!dy) scr[col] = v; else scr[col] |= dy>0 ? v>>dy : v<<-dy;
}

// =======================================================================
//Sous routines requises par printStringWithShift
// =======================================================================

int showChar(char ch, const uint8_t *data) 
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  scr[NUM_MAX*8 + i] = 0;
  return w;
}

void printCharWithShift(unsigned char c, int shiftDelay) 
{
  if (c < ' ' || c > '~'+25) return;
  c -= 32;
  int w = showChar(c, font);
  for (int i=0; i<w+1; i++) {
    delay(shiftDelay);
    scrollLeft();
    refreshAll();
  }
}

void printStringWithShift(const char* s, int shiftDelay)
{
  while (*s) {
    printCharWithShift(*s, shiftDelay);
    s++;
  }
}

// =======================================================================
