/*
 *     Maker      : DIYSensors                
 *     Project    : Stand-alone air quality monitor
 *     Version    : 1.0
 *     Date       : 11-2024
 *     Programmer : Ap Matteman
 *     
 */    

#include <Wire.h>
#include "SparkFun_ENS160.h"
#include <SparkFun_Qwiic_Humidity_AHT20.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "Icons.h"

SparkFun_ENS160 myENS160; 
AHT20 myAHT2x;
int ensStatus; 
// Connecting: EMS160
//   Board:                       SDA         SCL
//   ESP8266 ESP-01:..............GPIO0/D5    GPIO2/D3
//   NodeMCU 1.0, WeMos D1 Mini...GPIO4/D2    GPIO5/D1

// Declaration for an SSD1306 OLED display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float fTemperatureC;
float fHumidity;
int iGAQ;
int iGStatus;
int iGVOC;
int iGCO2;
String sGAQ;

unsigned long lPmillis = 0;       
const long lInterval = 1000; // 1000 ms => 1 Second





void setup()
  {
    Serial.begin(115200);
    Wire.begin();

    if( !myENS160.begin() ) { Serial.println("Could not communicate with the ENS160. Please check wiring."); }
    if (!myAHT2x.begin() )  { Serial.println("Could not communicate with theAHT20. Please check wiring.");   }

    // Reset the indoor air quality sensor's settings.
    if( myENS160.setOperatingMode(SFE_ENS160_RESET) )
      Serial.println("ENS160 Ready.");

    delay(100);

    myENS160.setOperatingMode(SFE_ENS160_IDLE); // Device needs to be set to idle to apply any settings.
    myENS160.setOperatingMode(SFE_ENS160_STANDARD); // Set to standard operation

    ensStatus = myENS160.getFlags();
    Serial.print("Gas Sensor Status Flag (0 - Standard, 1 - Warm up, 2 - Initial Start Up): ");
    Serial.println(ensStatus);

    // setup OLED Display
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
      Serial.println(F("SSD1306 allocation failed"));
    }
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
  }


void loop()
  {

  unsigned long lCmillis = millis();

  if (lCmillis - lPmillis >= lInterval) {
    lPmillis = lCmillis;
    
      //If a new measurement is available
      if (myAHT2x.available() == true)
      {
        //Get the new temperature and humidity value
        fTemperatureC = myAHT2x.getTemperature();
        fHumidity = myAHT2x.getHumidity();
        
        //Print the results
        Serial.print("Temperature: ");
        Serial.print(fTemperatureC, 2);
        Serial.println(" C\t");
        Serial.print("Humidity: ");
        Serial.print(fHumidity, 2);
        Serial.println("% RH");

        // Give values to Air Quality Sensor for compensating.
        myENS160.setTempCompensationCelsius(fTemperatureC);
        myENS160.setRHCompensationFloat(fHumidity);

        Serial.println();
      }



      if( myENS160.checkDataStatus() ) //If a new measurement is available
      {
        iGAQ = myENS160.getAQI();
        iGStatus = myENS160.getFlags();
        iGVOC = myENS160.getTVOC();
        iGCO2 = myENS160.getECO2();

        switch (iGAQ) {
          case 1:
            sGAQ = "Excellent";
            break;
          case 2:
            sGAQ = "Good";
            break;
          case 3:
            sGAQ = "Moderate";
            break;
          case 4:
            sGAQ = "Poor";
            break;
          case 5:
            sGAQ = "Unhealty";
            break;
        }

        // 1=Excellent, 2=Good, 3=Moderate, 4=Poor, 5=Unhealty
        Serial.print("Air Quality Index (1-5) : ");
        Serial.println(iGAQ);

        Serial.print("Total Volatile Organic Compounds: ");
        Serial.print(iGVOC);
        Serial.println("ppb");

        Serial.print("CO2 concentration: ");
        Serial.print(iGCO2);
        Serial.println("ppm");

        Serial.print("Gas Sensor Status Flag (0 - Standard, 1 - Warm up, 2 - Initial Start Up): ");
        Serial.println(iGStatus);

        Serial.println();

        fTemperatureC = myAHT2x.getTemperature();
        fHumidity = myAHT2x.getHumidity();

      }
      
      display.clearDisplay(); 
      display.setFont(&FreeMonoBold9pt7b);
      display.setCursor(0,10);
      display.print(" CO2   VOC");
      //display.drawBitmap(10,20, Picto_Healthy, 40, 40, WHITE);
      // display.drawBitmap(74,20, Picto_Healthy, 40, 40, WHITE);

      //CO2 display
      int CO2_Level=6;
      if (iGCO2 < 600) { CO2_Level = 1; }
      if ((iGCO2 >= 600) && (iGCO2 < 1000)) {  CO2_Level = 2; }
      if ((iGCO2 >= 1000) && (iGCO2 < 1200)) {  CO2_Level = 3; }
      if ((iGCO2 >= 1200) && (iGCO2 < 2000)) {  CO2_Level = 4; }
      if ((iGCO2 >= 2000) && (iGCO2 < 5000)) {  CO2_Level = 5; }

      switch (CO2_Level) {
        case 6:
          display.drawBitmap(10,20, Icon_sick, 40, 40, WHITE);
          break;
        case 5:
          display.drawBitmap(10,20, Icon_Sad, 40, 40, WHITE);
          break;
        case 4:
          display.drawBitmap(10,20, Icon_not_good, 40, 40, WHITE);
          break;
        case 3:
          display.drawBitmap(10,20, Icon_Neutral, 40, 40, WHITE);
          break;
        case 2:
          display.drawBitmap(10,20, Icon_Happy, 40, 40, WHITE);
          break;
        case 1:
          display.drawBitmap(10,20, Icon_Super_Happy, 40, 40, WHITE);
          break;
      }

      /VOC display
      int VOC_Level=6; // > 3000
      if(iGVOC < 250) { VOC_Level = 1; }
      if ((iGVOC >= 250) && (iGVOC < 500)) { VOC_Level = 2; }
      if ((iGVOC >= 500) && (iGVOC < 1000)) { VOC_Level = 3; }
      if ((iGVOC >= 1000) && (iGVOC < 3000)) { VOC_Level = 4; }

      switch (VOC_Level) {
        case 6:
          display.drawBitmap(74,20, Icon_sick, 40, 40, WHITE);
          break;
        case 5:
          display.drawBitmap(74,20, Icon_Sad, 40, 40, WHITE);
          break;
        case 4:
          display.drawBitmap(74,20, Icon_not_good, 40, 40, WHITE);
          break;
        case 3:
          display.drawBitmap(74,20, Icon_Neutral, 40, 40, WHITE);
          break;
        case 2:
          display.drawBitmap(74,20, Icon_Happy, 40, 40, WHITE);
          break;
        case 1:
          display.drawBitmap(74,20, Icon_Super_Happy, 40, 40, WHITE);
          break;
      }  
 
      display.display();

    }
  }

  