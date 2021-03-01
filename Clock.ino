//    /** The MIT License (MIT)
//
//  Copyright (c) 2018 David Payne
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//*/

// The code for this clock has been taken from two brilliant coders:
// Qrome and his:
// Marquee scroller:
// https://www.thingiverse.com/thing:2867294
// Code on github:
// https://github.com/Qrome/marquee-scroller
//
// jeje95 and his original:
// Delorean clock - Back to the future style
// https://www.thingiverse.com/thing:2980120
//
// This code just puts them both together to make a WiFi clock
//=====================================================================================
//  Difference between ESP32 and ESP8266 - ESP32 is more powerful with faster WiFi
//=====================================================================================

//default libraries
#include "TimeDB.h"
#include "SevenSegmentTM1637.h"
#include "SevenSegmentExtended.h"
#include <WiFi.h>           // For ESP32 WiFi connection
#include <Timezone.h>    // https://github.com/JChristensen/Timezone
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>

//=========================== Your WIFI Network Details ===============================
const char* ssid     = "YOUR-SSID-HERE";
const char* password = "YOUR-SSID-PASSWORD-HERE";
//=====================================================================================

//================================ TimezoneDB Settings ================================
String TIMEDBKEY = "YOUR-API-KEY-HERE"; // Your API Key from https://timezonedb.com/register
String LAT = "YOUR-LAT-HERE"; // You location which can be taken from TimezoneDB site
String LON = "YOUR-LON-HERE"; // Same as above
//=====================================================================================

int backlight = 25; // The brightness setting
int maxbacklight = 50; // The brightness setting
int year_red = 2128; // The year you wish to see on the top section
int year_orange = 1976; // The year you wish to see on the bottom section

//=====================================================================================
// Pins are as follows:
// RED CLOCK:
// CLK - 18, Displays: 19, 21, 23, 22, 12
// AM - 17 / PM - 5
// GREEN CLOCK:
// CLK - 15, Displays: 2, 4, 16, 17 & 5
// AM - 22 / PM - 12
// Orange Clock:
// CLK - 14, Displays: 27, 26, 33, 32 & 35
// AM - 32 / PM - 25
//=====================================================================================

//=====================================================================================
// Displays Section
//=====================================================================================
// Red Clock Pin
const byte PIN_CLK_Red = 18;   // define CLK pin  
// Green Clock Pin
const byte PIN_CLK_Green = 15;   // define CLK pin 
// Orange Clock Pin
const byte PIN_CLK_Orange = 14;   // define CLK pin 

//=====================================================================================
// GREEN Displays - Main clock
//=====================================================================================
const byte PIN_DIO_G1 = 2;
SevenSegmentExtended      green1(PIN_CLK_Green, PIN_DIO_G1);
const byte PIN_DIO_G2 = 4;
SevenSegmentTM1637       green2(PIN_CLK_Green, PIN_DIO_G2);
const byte PIN_DIO_G3 = 16;
SevenSegmentExtended     green3(PIN_CLK_Green, PIN_DIO_G3);
int greenAM = 17;
int greenPM = 5;

//=====================================================================================
// RED Displays - Top Clock
//=====================================================================================
const byte PIN_DIO_R1 = 19;
SevenSegmentExtended      red1(PIN_CLK_Red, PIN_DIO_R1);
const byte PIN_DIO_R2 = 21;
SevenSegmentTM1637       red2(PIN_CLK_Red, PIN_DIO_R2);
const byte PIN_DIO_R3 = 23;
SevenSegmentExtended     red3(PIN_CLK_Red, PIN_DIO_R3);
int redAM = 22;
int redPM = 12;

//=====================================================================================
// ORANGE Displays - Bottom Clock
//=====================================================================================
const byte PIN_DIO_O1 = 27;   // define DIO pin (any digital pin)
SevenSegmentExtended      orange1(PIN_CLK_Orange, PIN_DIO_O1);
const byte PIN_DIO_O2 = 26;   
SevenSegmentTM1637        orange2(PIN_CLK_Orange, PIN_DIO_O2);
const byte PIN_DIO_O3 = 33;   
SevenSegmentExtended       orange3(PIN_CLK_Orange, PIN_DIO_O3); 
int orangeAM = 32;
int orangePM = 25;
//=====================================================================================
// End displays
//=====================================================================================

/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

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

// TimeDB - do NOT alter
TimeDB TimeDB("");
String lastMinute = "xx";
int displayRefreshCount = 1;
long lastEpoch = 0;
long firstEpoch = 0;
long displayOffEpoch = 0;
boolean displayOn = true;

int getMinutesFromLastRefresh() {
  int minutes = (now() - lastEpoch) / 60;
  return minutes;
}

//==================================================================================
// Testing World Clock Section
// choose your timezones from
// https://github.com/JChristensen/Timezone
//==================================================================================

// Australia Eastern Time Zone (Sydney, Melbourne) - RED CLOCK
TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    // UTC + 11 hours
TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    // UTC + 10 hours
Timezone ausET(aEDT, aEST);

// UTC - This is NOT for Green Clock, this is set between Red & Orange
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule);

// US Pacific Time Zone (Las Vegas, Los Angeles) - ORANGE CLOCK
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone usPT(usPDT, usPST);

// Timezone
time_t time_local, local, utc ; 
TimeChangeRule *tcr;

//==================================================================================
// End World Clock Section
//==================================================================================

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

//===============================================================================
// End of Settings Section
//===============================================================================

// Main program

void setup() {
    // Setup Section
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

    orange1.begin();                    // initializes the display
    orange2.begin();
    orange3.begin();              
    green1.begin();
    green2.begin();
    green3.begin();            
    red1.begin();
    red2.begin();
    red3.begin();            
    orange1.setBacklight(backlight);    // set the brightness to 100 %
    orange1.setColonOn(true); 
    orange2.setBacklight(backlight);
    orange2.setColonOn(0);              // Switch off ":" for Orange "year"
    orange3.setBacklight(backlight);  
    green1.setBacklight(maxbacklight);
    green1.setColonOn(true); 
    green2.setBacklight(maxbacklight);
    green2.setColonOn(0);               // Switch off ":" for Green "year"
    green3.setBacklight(maxbacklight);
    red1.setColonOn(true); 
    red1.setBacklight(backlight);
    red2.setBacklight(backlight);
    red3.setBacklight(backlight);  
    red2.setColonOn(0);                 // Switch off ":" for Red "year"

    // Starting up WiFi
    // Serial.println();
    // Serial.println();
    // Serial.print("Connecting to ");
    // Serial.println(ssid);

    // WiFi.begin(ssid, password);

    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }

    // Serial.println("");
    // Serial.println("WiFi connected");
    // Serial.println("IP address: ");
    // Serial.println(WiFi.localIP());

    Serial.println("Booting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }

    Serial.println();

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
  

     Serial.println();
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

    //========================================================================
    time_t utc = now(); // testing world clock

    time_t red = ausET.toLocal(utc, &tcr);      // Setting RED Clock to your First Timezone ie: ausET
    time_t orange = usPT.toLocal(utc, &tcr);    // Setting Orange Clock to 2nd Timezone ie: usPT

    printDateTime(ausET, utc, "Sydney");        // Print time of first clock to monitor
    printDateTime(usPT, utc, " Los Angeles");   // Print time of second clock to monitor
    //========================================================================

        // Green AM & PM

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

        //This needs testing  first
        // Red AM & PM
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

        // Orange AM & PM
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
      
        //=================================================================================
        // Now Clocks part
        //=================================================================================
        // Red Timezone displays -  Destination TIME 10.21 2128 same 10:07
        red1.printTime(10, 21, true);                           // October 21st or any date
        red2.print(year_red, true);                             // 2128 or any year (see setup section)
//      red3.printTime(10, 07, true);                           // 10:07am unncomment to display a certain time
  
        red3.printTime(hour(red), minute(red), true);           // Red Timezone

    
        // Green Displays - Present TIME - Your Timezone
        green1.printTime(month(), day(), true); 
        green2.print(year());
        green3.printTime(hour(), minute(), true);
    
        // Orange Displays - Last time departed 11.25 1976 01:24                      
        orange1.printTime(11, 25, true);                        // November 25th or any date                
        orange2.print(year_orange, true);                       // 1976 or any year (see setup section)
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

//======================================================
// May nnot be required, but put it in just incasae
//======================================================
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
