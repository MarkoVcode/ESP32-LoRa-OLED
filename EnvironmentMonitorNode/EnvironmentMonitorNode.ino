#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"

//IOs
#define SCK 5 // GPIO5 - SX1278's SCK
#define MISO 19 // GPIO19 - SX1278's MISO
#define MOSI 27 // GPIO27 - SX1278's MOSI
#define SS 18 // GPIO18 - SX1278's CS
#define RST 14 // GPIO14 - SX1278's RESET
#define DI0 26 // GPIO26 - SX1278's IRQ (interrupt request)
#define BAND 868E6 // 915E6
#define BUTTON 0

//Logic configuration
#define SEND_READINGS_DELAY_S 60
#define DISPLAY_OFF_DELAY_S 60

//Endpoint identification
//instalation identifier
String netID = "KS";
//endpoint identifier
String nodeID = "02";

unsigned int counter = 0;
int displayTimeout = 0;
int sendReadingsDelay = 0;

SSD1306 display (0x3c, 4, 15);

String readSensors() {
  String result = "T39.98H48.55P324523";
  //read sensors here
  Serial.println ("Sensors: " + result);
  return result;
}

void transmitResults(String data) {
  digitalWrite (2, HIGH); // turn the LED on (HIGH is the voltage level)
  // send packet
  LoRa.beginPacket ();
  LoRa.print (data);
  LoRa.endPacket ();
  delay (900); // wait for a second
  digitalWrite (2, LOW); // turn the LED off by making the voltage LOW
  LoRa.sleep();
}

void displayResults(String data, bool txInd, int countdown) {
  display.clear();
  display.drawString(0, 0, "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯");
  display.drawString(0, 2, "______________________");
  display.drawString(0, 4, "______________________");
  display.drawString(0, 0, "             |                 |       |");
  display.drawString(0, 1, "NET:"+netID);
  display.drawString(45, 1, "NODE:"+nodeID);
  display.drawString(98, 1, String(countdown));
  display.drawString(2, 17, "Temp");
  display.drawString(57, 17, "[ºC]");
  display.drawString(92, 17, "-26.89");
  display.drawString(2, 27, "Humidity");
  display.drawString(57, 27, "[%]"); 
  display.drawString(92, 27, "58.65"); 
  display.drawString(2, 37, "Pressure");
  display.drawString(57, 37, "[hPa]");
  display.drawString(92, 37, "1068"); 
  //display.drawString(2, 47, "Dew point");
  //display.drawString(57, 47, "[ºC]");
  //display.drawString(92, 47, "-26.89");
  display.drawString(0, 52, "______________________");
  display.drawLine(0,16,0, 62);
  display.drawLine(52,16,52, 62);
  display.drawLine(85,16,85, 62);
  display.drawLine(127,16,127, 62);
//  display.drawString (0, 50, data);
  if(txInd) {
    display.drawCircle(124, 7, 3);
    display.drawCircle(124, 7, 1);
  }
  display.display ();
}

void setup () {
  displayTimeout = DISPLAY_OFF_DELAY_S;
  pinMode (16, OUTPUT);
  pinMode (2, OUTPUT);
  pinMode (BUTTON, INPUT);
  digitalRead(0);
  digitalWrite (16, LOW); // set GPIO16 low to reset OLED
  delay (50);
  digitalWrite (16, HIGH); // while OLED is running, GPIO16 must go high
  
  Serial.begin (9600);
  while (! Serial);
  Serial.println ();
  Serial.println ("LoRa Sender Test");
  
  SPI.begin (SCK, MISO, MOSI, SS);
  LoRa.setPins (SS, RST, DI0);
  if (! LoRa.begin (BAND)) {
    Serial.println ("Starting LoRa failed!");
    while (1);
  }

  Serial.println ("init ok");
  
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  delay (1500);
}

void loop () {
    Serial.println (displayTimeout);
  if(!digitalRead(BUTTON)) {
      displayTimeout = DISPLAY_OFF_DELAY_S;
  }
  
  String reading = readSensors();
  if(displayTimeout > 0) {
    displayResults(reading, true, counter);
  }
  transmitResults(reading);
  if(displayTimeout > 0) {
    displayResults(reading, false, counter);
    --displayTimeout; 
    if(displayTimeout == 0) {
      Serial.println ("clear display only once");
      display.clear();
      display.display ();
    }
  }
  
  counter ++;
  delay (1000); // wait for a second
}
