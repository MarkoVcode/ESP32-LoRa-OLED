//https://github.com/osresearch/esp32-ttgo/tree/master/demo/Weather
#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"

#define SCK 5 // GPIO5 - SX1278's SCK
#define MISO 19 // GPIO19 - SX1278's MISO
#define MOSI 27 // GPIO27 - SX1278's MOSI
#define SS 18 // GPIO18 - SX1278's CS
#define RST 14 // GPIO14 - SX1278's RESET
#define DI0 26 // GPIO26 - SX1278's IRQ (interrupt request)
#define BAND 868E6 // 915E6
  
static unsigned char splashing[] =
{  B11000000, B00000000,
   B11000000, B00000001,
   B11000000, B00000001,
   B11100000, B00000011,
   B11100000, B11110011,
   B11111000, B11111110,
   B11111111, B01111110,
   B10011111, B00110011,
   B11111100, B00011111,
   B01110000, B00001101,
   B10100000, B00011011,
   B11100000, B00111111,
   B11110000, B00111111,
   B11110000, B01111100,
   B01110000, B01110000,
   B00110000, B00000000 };
 
SSD1306 display (0x3c, 4, 15);

String getValue(char string[], String index) {
   // char string[] = "KS;01;T:-39.98;H:48.55;P:32453;B:3.2";
        Serial.println("-");
     Serial.println(String(string));
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

void displayResults(char inbound[], bool txInd, int countdown, String rssi) {
          Serial.println("--");
     Serial.println(String(inbound));
  display.clear();
  display.drawString(0, 0, "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯");
  display.drawString(0, 2, "______________________");
  display.drawString(0, 4, "______________________");
  display.drawString(0, 0, "             |                 |       |");
  if(sizeof(inbound) != 0) {
    display.drawString(0, 1, "NET:"+getValue(inbound, "NET"));
    display.drawString(45, 1, "NODE:"+getValue(inbound, "NODE"));
    display.drawString(98, 1, rssi);
    display.drawString(4, 17, "Temp");
    display.drawString(57, 17, "[ºC]");
    display.drawString(92, 17, getValue(inbound, "T"));
    display.drawString(4, 27, "Humidity");
    display.drawString(57, 27, "[%]"); 
    display.drawString(92, 27, getValue(inbound, "H"));   
    display.drawString(4, 37, "Pressure");
    display.drawString(57, 37, "[hPa]"); 
    display.drawString(92, 37, getValue(inbound, "P"));   
    display.drawString(4, 47, "Battery");
    display.drawString(57, 47, "[V]");
    display.drawString(92, 47, getValue(inbound, "B"));
  } else {
    display.drawString(0, 1, "NET:--");
    display.drawString(45, 1, "NODE:--");
  }
  display.drawString(0, 52, "______________________");
  display.drawLine(0,16,0, 62);
  display.drawLine(52,16,52, 62);
  display.drawLine(85,16,85, 62);
  display.drawLine(127,16,127, 62);
  if(txInd) {
    display.drawCircle(124, 7, 3);
    display.drawCircle(124, 7, 1);
  }
  display.display();
}

void cbk (int packetSize) {
  char inbound[packetSize];
  memset(inbound, 0, sizeof(inbound));
  for (int i = 0; i < packetSize; i++) {
    char ss = (char) LoRa.read();
 //   Serial.println(ss);
    inbound[i] = ss;
  }
  Serial.println(packetSize);
  Serial.println(sizeof(inbound));
  Serial.println(String(inbound));
  displayResults(inbound, false, 0, String(LoRa.packetRssi(), DEC));
}

void setup () {
  pinMode (16, OUTPUT);
  digitalWrite (16, LOW); // set GPIO16 low to reset OLED
  delay (50);
  digitalWrite (16, HIGH); // while OLED is running, GPIO16 must go high,
  
  Serial.begin (115200);
  while (! Serial);
  Serial.println ();
  Serial.println ("LoRa Receiver Callback");
  SPI.begin (SCK, MISO, MOSI, SS);
  LoRa.setPins (SS, RST, DI0);
  if (! LoRa.begin (BAND)) {
    Serial.println ("Starting LoRa failed!");
    while (1);
  }
  //LoRa.onReceive(cbk);
  LoRa.receive ();
  Serial.println ("init ok");
  display.init ();
  display.flipScreenVertically ();
  display.setFont (ArialMT_Plain_10);
  display.drawXbm(30, 16, 16, 16, splashing);
  display.display();
  delay (1500);
}

void loop () {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {cbk (packetSize); }
  delay (10);
}
