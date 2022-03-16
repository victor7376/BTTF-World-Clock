//==========================================
//  Difference between ESP32 and ESP8266
//  ESP32 is more powerful with faster WiFi
//==========================================

//default libraries
#include "TimeDB.h"
#include "SevenSegmentTM1637.h"
#include "SevenSegmentExtended.h"
#include <WiFi.h>
#include <Timezone.h>    // https://github.com/JChristensen/Timezone
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>

//======== Your WIFI Network Details =========
const char* ssid     = "YOURSSIDHERE";
const char* password = "YOURSSIDPASSWORDHERE";
//============================================

//================================ TimezoneDB Settings ================================
String TIMEDBKEY = "YOURKEYHERE"; // Your API Key from https://timezonedb.com/register
String LAT = "YOURLATHERE"; // You location which can be taken from TimezoneDB site
String LON = "YOURLONHERE"; // Same as above
//=====================================================================================

int backlight = 25; // The brightness setting
int maxbacklight = 50; // The brightness setting
int year_red = 2128; // The year you wish to see on the top section
int year_orange = 1976; // The year you wish to see on the bottom section

//===============================================
// Displays Section
//===============================================
// Connections (saving scrolling to see which pins:
// Red: 18, 19, 21, 23 - 22 (AM) & 12 (PM)
// Green: 15, 2, 4, 16 - 17 (AM) & 5 (PM)
// Orange: 14, 27, 26, 33 - 32 (AM) & 35 (PM)
//===============================================
//
// Defining pins
//
// Red Clock Pin
const byte PIN_CLK_Red = 18;   // define CLK pin  
// Green Clock Pin
const byte PIN_CLK_Green = 15;   // define CLK pin 
// Orange Clock Pin
const byte PIN_CLK_Orange = 14;   // define CLK pin 

//===============================================
// GREEN Displays - Main clock
//===============================================
const byte PIN_DIO_G1 = 2;
SevenSegmentExtended      green1(PIN_CLK_Green, PIN_DIO_G1);
const byte PIN_DIO_G2 = 4;
SevenSegmentTM1637       green2(PIN_CLK_Green, PIN_DIO_G2);
const byte PIN_DIO_G3 = 16;
SevenSegmentExtended     green3(PIN_CLK_Green, PIN_DIO_G3);
int greenAM = 17;
int greenPM = 5;

//===============================================
// RED Displays - Top Clock
//===============================================
const byte PIN_DIO_R1 = 19;
SevenSegmentExtended      red1(PIN_CLK_Red, PIN_DIO_R1);
const byte PIN_DIO_R2 = 21;
SevenSegmentTM1637       red2(PIN_CLK_Red, PIN_DIO_R2);
const byte PIN_DIO_R3 = 23;
SevenSegmentExtended     red3(PIN_CLK_Red, PIN_DIO_R3);
int redAM = 22;
int redPM = 12;

//===============================================
// ORANGE Displays - Bottom Clock
//===============================================
const byte PIN_DIO_O1 = 27;   // define DIO pin (any digital pin)
SevenSegmentExtended      orange1(PIN_CLK_Orange, PIN_DIO_O1);
const byte PIN_DIO_O2 = 26;   
SevenSegmentTM1637        orange2(PIN_CLK_Orange, PIN_DIO_O2);
const byte PIN_DIO_O3 = 33;   
SevenSegmentExtended       orange3(PIN_CLK_Orange, PIN_DIO_O3); 
int orangeAM = 32;
int orangePM = 25;
//===============================
// End displays
//===============================

int minutesBetweenDataRefresh = 15;  // Time in minutes between data refresh (default 15 minutes)
boolean IS_24HOUR = false; // 23:00 millitary 24 hour clock
int minutesBetweenRefreshing = 1; 
//boolean flashOnSeconds = true; // when true the : character in the time will flash on and off as a seconds indicator

// LED Settings
const int offset = 1;
int refresh = 0;

// OTA
boolean ENABLE_OTA = true;    // this will allow you to load firmware to the device over WiFi (see OTA for ESP8266)
String OTA_Password = "";     // Set an OTA password here -- leave blank if you don't want to be prompted for password
#define HOSTNAME "BTTF-"

// TimeDB - do NOT alter
TimeDB TimeDB("");
String lastMinute = "xx";
int displayRefreshCount = 1;
long lastEpoch = 0;
long firstEpoch = 0;
long displayOffEpoch = 0;
boolean displayOn = true;


//=========================
// Testing world clock
//=========================
// Australia Eastern Time Zone (Sydney, Melbourne)
TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    // UTC + 11 hours
TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    // UTC + 10 hours
Timezone ausET(aEDT, aEST);

// UTC
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule);

// US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone usPT(usPDT, usPST);

// Timezone
time_t time_local, local, utc ; 
TimeChangeRule *tcr;

void printDateTime(Timezone tz, time_t utc, const char *descr)
{
    char buf[40];
    char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
    TimeChangeRule *tcr;        // pointer to the time change rule, use to get the TZ abbrev

    time_t t = tz.toLocal(utc, &tcr);
    strcpy(m, monthShortStr(month(t)));
    sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d %s",
        hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t), tcr -> abbrev);
    Serial.print(buf);
    Serial.print(' ');
    Serial.println(descr);
}

//=========================
// end world clock settings
//=========================

// Main program
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  
  delay(10);
 //New Line to clear from start garbage
  Serial.println();

  // Booting up the displays
  pinMode(PIN_CLK_Red, OUTPUT);
  pinMode(PIN_CLK_Green, OUTPUT);
  pinMode(PIN_CLK_Orange, OUTPUT);
  pinMode(PIN_DIO_O1, OUTPUT);
  pinMode(PIN_DIO_O2, OUTPUT);
  pinMode(PIN_DIO_O3, OUTPUT);
  pinMode(PIN_DIO_G1, OUTPUT); 
  pinMode(PIN_DIO_G2, OUTPUT);
  pinMode(PIN_DIO_G3, OUTPUT); 
  pinMode(PIN_DIO_R1, OUTPUT);
  pinMode(PIN_DIO_R2, OUTPUT);
  pinMode(PIN_DIO_R3, OUTPUT);
  pinMode(greenAM, OUTPUT);
  pinMode(greenPM, OUTPUT);
  pinMode(redAM, OUTPUT);
  pinMode(redPM, OUTPUT);
  pinMode(orangeAM, OUTPUT);
  pinMode(orangePM, OUTPUT);

  orange1.begin();            // initializes the display
  orange2.begin();
  orange3.begin();              
  green1.begin();
  green2.begin();
  green3.begin();            
  red1.begin();
  red2.begin();
  red3.begin();            
  orange1.setBacklight(backlight);  // set the brightness to 100 %
  orange1.setColonOn(true); 
  orange2.setBacklight(backlight);
  orange2.setColonOn(0); // Switch off ":" for Orange "year"
  orange3.setBacklight(backlight);  
  green1.setBacklight(maxbacklight);
  green1.setColonOn(true); 
  green2.setBacklight(maxbacklight);
  green2.setColonOn(0); // Switch off ":" for Green "year"
  green3.setBacklight(maxbacklight);
  red1.setColonOn(true); 
  red1.setBacklight(backlight);
  red2.setBacklight(backlight);
  red3.setBacklight(backlight);  
  red2.setColonOn(0); // Switch off ":" for Red "year"

  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println();

  ArduinoOTA.setHostname(HOSTNAME);
  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // initialize dispaly
}

// Now for the loop which will run and run....
void loop() {
  ArduinoOTA.handle();
  // Retrieving Date & Time from TimezoneDB
  if ((getMinutesFromLastRefresh() >= minutesBetweenDataRefresh) || lastEpoch == 0) {
    getTimeData();
  }
  

  if (lastMinute != TimeDB.zeroPad(minute())) {
    lastMinute = TimeDB.zeroPad(minute());
    
    displayRefreshCount --;
    // Check to see if we need to Scroll some Data
    if (displayRefreshCount <= 0) {
      displayRefreshCount = minutesBetweenRefreshing;
         
    }
  }

  //==================
  time_t utc = now(); // testing world clock
  time_t red = ausET.toLocal(utc, &tcr);
  time_t orange = usPT.toLocal(utc, &tcr); 
  printDateTime(ausET, utc, "Sydney");
  printDateTime(usPT, utc, " Los Angeles");
  //==================

      // Green Clock AM / PM
      if(hour()>=13){
        digitalWrite(greenAM,0);
        digitalWrite(greenPM,1);
      } else if (hour()==12){ 
        digitalWrite(greenAM,0);
        digitalWrite(greenPM,1);
      }
        else  {
        digitalWrite(greenAM,1);
        digitalWrite(greenPM,0);
      }

      // Red Clock AM / PM
      if(hour(red)>=13){
        digitalWrite(redAM,0);
        digitalWrite(redPM,1);
      } else if (hour(red)==12){ 
        digitalWrite(redAM,0);
        digitalWrite(redPM,1);
      }
        else  {
        digitalWrite(redAM,1);
        digitalWrite(redPM,0);
      }

      // Orange Clock AM / PM
      if(hour(orange)>=13){
        digitalWrite(orangeAM,0);
        digitalWrite(orangePM,1);
      } else if (hour(orange)==12){ 
        digitalWrite(orangeAM,0);
        digitalWrite(orangePM,1);
      }
        else  {
        digitalWrite(orangeAM,1);
        digitalWrite(orangePM,0);
      }

      //Powersave
    if ((hour() == 22) && (minute() == 30)) { // Turns displays off at given time
      green1.off();
      green2.off();
      green3.off();
      red1.off();
      red2.off();
      red3.off();
      orange1.off();
      orange2.off();
      orange3.off();
      //Serial.println("Powersave"); 
    }
    else if ((hour() == 9) && (minute() == 30)){ // Turns displays on at given time
      green1.on();
      green2.on();
      green3.on();
      red1.on();
      red2.on();
      red3.on();
      orange1.on();
      orange2.on();
      orange3.on();
      //Serial.println("No Powersave");    
    }
      
      // ===========================
      // Now Clocks part
      // ===========================
      // Red Timezone displays -  Destination TIME 10.21 2128 same 10:07
      red1.printTime(10, 21, true);                         // October 21st
      red2.print(year_red, true);                           // 2128
//      red3.printTime(10, 07, true);                         // 10:07am unncomment to display a certain time
  
      red3.printTime(hour(red), minute(red), true);         // Red Timezone if can get it to work

    
      // Green Displays - Present TIME - Your Timezone
      green1.printTime(month(), day(), true); 
      green2.print(year());
      green3.printTime(hour(), minute(), true);
    
      // Orange Displays - Last time departed 11.25 1976 01:24                      
      orange1.printTime(11, 25, true);                        // November 25th                
      orange2.print(year_orange, true);                       // 1976
//      orange3.printTime(01, 24, true);                        // 01:24am Uncomment to display certain time

      orange3.printTime(hour(orange), minute(orange), true);  // Orange Timezone if can get it to work

}

String hourMinutes(boolean isRefresh) {
  if (IS_24HOUR) {
    return hour() + TimeDB.zeroPad(minute());
  } else {
    return hourFormat12() + TimeDB.zeroPad(minute());
  }
}

void getTimeData() //client function to send/receive GET request data.
{
  Serial.println();

  // only pull the weather data if display is on
    
  Serial.println("Updating Time...");
  //Update the Time
  
  TimeDB.updateConfig(TIMEDBKEY, LAT, LON);
  time_t currentTime = TimeDB.getTime();
  if(currentTime > 5000 || firstEpoch == 0) {
    setTime(currentTime);
  } else {
    Serial.println("Time update unsuccessful!");
  }
  lastEpoch = now();
  if (firstEpoch == 0) {
    firstEpoch = now();
    Serial.println("firstEpoch is: " + String(firstEpoch));
  }
  
  
  Serial.println();
}

int getMinutesFromLastDisplay() {
  int minutes = (now() - displayOffEpoch) / 60;
  return minutes;
}

int getMinutesFromLastRefresh() {
  int minutes = (now() - lastEpoch) / 60;
  return minutes;
}

String getTimeTillUpdate() {
  String rtnValue = "";

  long timeToUpdate = (((minutesBetweenDataRefresh * 60) + lastEpoch) - now());

  int hours = numberOfHours(timeToUpdate);
  int minutes = numberOfMinutes(timeToUpdate);
  int seconds = numberOfSeconds(timeToUpdate);

  rtnValue += String(hours) + ":";
  if (minutes < 10) {
    rtnValue += "0";
  }
  rtnValue += String(minutes) + ":";
  if (seconds < 10) {
    rtnValue += "0";
  }
  rtnValue += String(seconds);

  return rtnValue;
}

time_t compileTime()
{
    const time_t FUDGE(10);     // fudge factor to allow for compile time (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char chMon[4], *m;
    tmElements_t tm;

    strncpy(chMon, compDate, 3);
    chMon[3] = '\0';
    m = strstr(months, chMon);
    tm.Month = ((m - months) / 3 + 1);

    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);
    time_t t = makeTime(tm);
    return t + FUDGE;           // add fudge factor to allow for compile time
}

