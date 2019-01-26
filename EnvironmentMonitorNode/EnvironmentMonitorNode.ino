//Measurments readings format:

//T:-39.98;H:48.55;P:324523;B:3.2
//T - temperature
//H - humidity
//P - pressure
//B - battery level

//message packet format
//KS;01;T:-39.98;H:48.55;P:324523;B:3.2

//encrypted message packet format
//KS:01:<encrypted message packet format from above>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
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

#define BME_SCK 22    //SCL
#define BME_MISO 12   //SDO
#define BME_MOSI 21   //SDI   //SDA
#define BME_CS 13     //CSB

//Logic configuration
#define SEND_READINGS_DELAY_S 15
#define DISPLAY_OFF_DELAY_S 90
#define SEALEVELPRESSURE_HPA (1013.25)
//Endpoint identification
//instalation identifier
String netID = "KS";
//endpoint identifier
String nodeID = "01";

unsigned int counter = 0;
int displayTimeout = 0;
int sendReadingsDelay = 0;

Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

bool bmeSensorPresent = true;
char* myValues[]={"This is string 1", "This is string 2", "This is string 3",
"This is string 4", "This is string 5","This is string 6"};
   
SSD1306 display (0x3c, 4, 15);

// Interrupt Service Routine - Keep it short!
void IRAM_ATTR handleButtonInterrupt() {
  if(displayTimeout == 0) {
    displayTimeout = DISPLAY_OFF_DELAY_S;
  }
}

String readSensors() {
    String resultBuild ="T:"+String(bme.readTemperature())+";H:"+String(bme.readHumidity())+";P:"+String(bme.readPressure() / 100.0F)+";B:3.9;C:3.9;";
    return resultBuild; 
}


String preparePayload(String readings, bool encryptionEnabled) {
  String result;
  if(encryptionEnabled) {
    return netID + ";" + nodeID + ";" + encryptPayload(readings);
  } else {
    return netID + ";" + nodeID + ";" + readings;
  }
}

String encryptPayload(String payload) {
  //no encryption for now
  return payload;  
}

void transmitResults(String data) {
  digitalWrite (2, HIGH); // turn the LED on (HIGH is the voltage level)
  LoRa.beginPacket ();
  LoRa.print (data);
  LoRa.endPacket ();
  delay (900); // wait for a second
  digitalWrite (2, LOW); // turn the LED off by making the voltage LOW
  LoRa.sleep();
}

String getValue(String dataIN, String index) {
    String nodeValues = "KS;01;"+dataIN;
    char string[nodeValues.length()];
    nodeValues.toCharArray(string, nodeValues.length());
  //  char string[] = "KS;01;T:-39.98;H:48.55;P:32453;B:3.2";
    char delimiter[] = ";";
    char delimiter2[] = ":";
    char* ptr = strtok(string, delimiter);
    int pointerIndex = 0;
    while(ptr != NULL) {
      if(index.equals("NET") && pointerIndex == 0) {
        return String(ptr); 
      } else if(index.equals("NODE") && pointerIndex == 1) {
        return String(ptr); 
      } else if(index.equals(String(ptr[0]))){
        char* ptr2 = strtok(ptr, delimiter2);
        while(ptr2 != NULL) {
          if(!String(ptr2).equals(index)) return String(ptr2);
          ptr2 = strtok(NULL, delimiter2);
        }
      }
      ptr = strtok(NULL, delimiter);
      pointerIndex++;
    }
    return "";
}

void displayResults(String data, bool txInd, int countdown) {
  display.clear();
  display.drawString(0, 0, "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯");
  display.drawString(0, 2, "______________________");
  display.drawString(0, 4, "______________________");
  display.drawString(0, 0, "             |                 |       |");
  if(!data.equals("")) {
    display.drawString(0, 1, "NET:"+getValue(data, "NET"));
    display.drawString(45, 1, "NODE:"+getValue(data, "NODE"));
    display.drawString(98, 1, String(countdown));
    display.drawString(4, 17, "Temp");
    display.drawString(56, 17, "[ºC]");
    display.drawString(86, 17, getValue(data, "T"));
    display.drawString(4, 27, "Humidity");
    display.drawString(57, 27, "[%]"); 
    display.drawString(86, 27, getValue(data, "H"));   
    display.drawString(4, 37, "Pressure");
    display.drawString(52, 37, "[hPa]"); 
    display.drawString(86, 37, getValue(data, "P"));   
    display.drawString(4, 49, "Battery");
    display.drawString(57, 49, "[V]");
    display.drawLine(103,50,103, 62); 
    display.drawString(84, 49, getValue(data, "B") + "   " + getValue(data, "C"));
  } else {
    display.drawString(0, 1, "NET:--");
    display.drawString(45, 1, "NODE:--");
  }
  display.drawString(0, 52, "______________________");
  display.drawLine(0,16,0, 62);
  display.drawLine(48,16,48, 62); 
  display.drawLine(79,16,79, 62);
  display.drawLine(127,16,127, 62);
  if(txInd) {
    display.drawCircle(124, 7, 3);
    display.drawCircle(124, 7, 1);
  }
  display.display();
}

void setup () {
  Serial.begin (115200);
 // Wire.begin();

    bool status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin();  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    
  displayTimeout = DISPLAY_OFF_DELAY_S;
  pinMode (16, OUTPUT);
  pinMode (2, OUTPUT);
  pinMode (BUTTON, INPUT);
  digitalRead(0);
  digitalWrite (16, LOW); // set GPIO16 low to reset OLED
  delay (50);
  digitalWrite (16, HIGH); // while OLED is running, GPIO16 must go high
  

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

  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButtonInterrupt, FALLING);
  displayResults("", false, 0);

  delay (1500);
}

void loop () {
  String reading;
  String displayData=readSensors();
  Serial.println ("cycle");
  if(displayTimeout > 0 || sendReadingsDelay == 0) {
    reading = readSensors();
  }
  if(displayTimeout > 0) {
    displayData = preparePayload(reading, false);
    if(sendReadingsDelay == 0) {
      displayResults(displayData, true, displayTimeout);
    }
  }
  if(sendReadingsDelay == 0) {
    transmitResults(preparePayload(reading, true));
    sendReadingsDelay = SEND_READINGS_DELAY_S;
  }
  if(displayTimeout > 0) {
    displayResults(displayData, false, displayTimeout);
    --displayTimeout; 
    if(displayTimeout == 0) {
      Serial.println ("clear display only once");
      display.clear();
      display.display();
    }
    delay (1000);
  } else {
    delay (1000); 
  }
  sendReadingsDelay--;
  counter ++;
}
