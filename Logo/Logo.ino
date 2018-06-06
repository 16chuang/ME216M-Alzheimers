#include <Wire.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <SPI.h>
#include <GxEPD.h>
#include <GxGDEH029A1/GxGDEH029A1.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include "GxGDEP015OC1/BitmapExamples.h"

//Byte array of bitmap of 320 x 92 px:
    
const unsigned char PROGMEM bitmapLogo [] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x80, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x3f, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf8, 
0x7, 0xf0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x30, 0x0, 0x3, 0x86, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0xb0, 0x0, 0x2, 0x40, 0x60, 0x0, 0x0, 0x0, 0x0, 0x2, 0x28, 0x0, 
0x2, 0x20, 0xc, 0x0, 0x0, 0x0, 0x0, 0x18, 0x24, 0x0, 0x2, 0x10, 0x0, 0x80, 0x0, 
0x0, 0x0, 0x60, 0x22, 0x0, 0x2, 0x8, 0x0, 0xc0, 0x0, 0x0, 0x1, 0x0, 0x21, 0x0, 
0x2, 0x4, 0x1, 0x0, 0x0, 0x3, 0x84, 0x0, 0x20, 0x80, 0x2, 0x2, 0x1, 0x0, 0x0, 
0x3, 0xf0, 0x0, 0x20, 0x40, 0x0, 0x1, 0x3, 0x8, 0x0, 0x7, 0xe0, 0x0, 0x20, 0x20, 
0x2, 0x0, 0x82, 0x4, 0x0, 0x7, 0xc0, 0x0, 0x20, 0x10, 0x2, 0x0, 0x4c, 0x2, 0x0, 
0x3, 0xe0, 0x0, 0x20, 0x8, 0x2, 0x0, 0x3e, 0x1, 0x0, 0x1, 0x30, 0x0, 0x20, 0x4, 
0x2, 0x0, 0x3e, 0x0, 0x88, 0x0, 0x8, 0x0, 0x20, 0x2, 0x2, 0x0, 0x3f, 0x0, 0x7e, 
0x0, 0x4, 0x0, 0x20, 0x1, 0x2, 0x0, 0x7f, 0xfc, 0x7f, 0x0, 0x3, 0x0, 0x20, 0x0, 
0x82, 0x1, 0x9c, 0x7, 0xff, 0x0, 0x1, 0x80, 0x20, 0x0, 0x42, 0x2, 0x4, 0x0, 0x7f, 
0x0, 0x0, 0x40, 0x20, 0x0, 0x22, 0xc, 0x2, 0x0, 0x3e, 0x0, 0x0, 0x20, 0x20, 0x0, 
0x10, 0x30, 0x2, 0x0, 0x3e, 0x0, 0x0, 0x18, 0x20, 0x0, 0xc, 0x40, 0x3, 0x0, 0x18, 
0x0, 0x0, 0x4, 0x60, 0x0, 0x7, 0x80, 0x1, 0x0, 0x0, 0x0, 0x0, 0x3, 0xf8, 0x0, 
0x1f, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x1, 0xf8, 0xf, 0xc2, 0x80, 0x0, 0x80, 0x0, 
0x0, 0x0, 0x1, 0xff, 0xe0, 0x4, 0x60, 0x0, 0x80, 0x0, 0x0, 0x0, 0x1, 0xfc, 0x0, 
0x4, 0x10, 0x0, 0xc0, 0x0, 0x0, 0x0, 0x7, 0xfc, 0x0, 0x4, 0xc, 0x0, 0x40, 0x0, 
0x0, 0x0, 0x79, 0xf8, 0x0, 0x8, 0x2, 0x0, 0x40, 0x0, 0x0, 0x3, 0xc0, 0xfc, 0x0, 
0x8, 0x1, 0x80, 0x20, 0x0, 0x0, 0x3c, 0x0, 0x2, 0x0, 0x8, 0x0, 0x40, 0x20, 0x0, 
0x43, 0xc0, 0x0, 0x1, 0x0, 0x10, 0x0, 0x30, 0x10, 0x0, 0xfc, 0x0, 0x0, 0x0, 0x80, 
0x10, 0x0, 0x8, 0x10, 0x0, 0xf0, 0x0, 0x0, 0x0, 0x40, 0x20, 0x0, 0x6, 0x18, 0x0, 
0xcf, 0xc0, 0x0, 0x0, 0x20, 0x20, 0x0, 0x1, 0x8, 0x0, 0x40, 0x3f, 0x0, 0x0, 0x10, 
0x20, 0x0, 0x0, 0xc8, 0x0, 0x0, 0x0, 0x7e, 0x0, 0x8, 0xc0, 0x0, 0x0, 0x24, 0x0, 
0x0, 0x0, 0x3, 0xf0, 0x7, 0xe0, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x0, 0x7, 0xe7, 
0xe0, 0x0, 0x0, 0x5e, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1f, 0xff, 0xff, 0xff, 0xec, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x7, 0xf0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1f, 
0xf8, 0x0, 0x0, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe3, 0xc6, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x7, 0x2, 0x43, 0x80, 0x0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x18, 0x4, 
0x0, 0xc0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x4, 0x20, 0x30, 0x2, 0x0, 0x0, 
0x0, 0x0, 0x7, 0x0, 0x8, 0x20, 0xc, 0x4, 0x0, 0x0, 0x0, 0x0, 0x38, 0x0, 0x8, 
0x20, 0x3, 0x8, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x0, 0x10, 0x0, 0x0, 0xf0, 0x0, 0x0, 
0x0, 0x6, 0x0, 0x0, 0x10, 0x10, 0x0, 0xe0, 0x0, 0x0, 0xc, 0x30, 0x0, 0x0, 0x20, 
0x10, 0xf0, 0x0, 0x0, 0x0, 0x1f, 0xc0, 0x0, 0x0, 0x40, 0x30, 0x0, 0x0, 0x0, 0x0, 
0x1f, 0x0, 0x0, 0x0, 0x40, 0x60, 0x0, 0x0, 0x0, 0x0, 0x1f, 0x0, 0x0, 0x0, 0x81, 
0xc0, 0x0, 0x0, 0x0, 0x0, 0x1f, 0xc0, 0x0, 0x0, 0x82, 0x0, 0x0, 0x0, 0x0, 0x0, 
0xc, 0x30, 0x0, 0x1, 0xc, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x1, 0x18, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x80, 0x2, 0x61, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x20, 0x4, 0xc2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc, 0x1f, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x3e, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x7e, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3e, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3c, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
 };

//      Display pins     //
#define RST  -1
#define DIN  13
#define CLK  14
#define BUSY 12
#define CS   15
#define DC   2

GxIO_Class display_io(SPI, CS, DC, RST); 
GxEPD_Class display(display_io, RST, BUSY);

void setup() {
  display.init();
  delay(1000);
  displayLogo();
}

void loop() {
  
}

void displayLogo() {
  display.fillScreen(GxEPD_WHITE);
  display.setRotation(3);
  display.drawBitmap(bitmapLogo, 25, 30, 80, 72, GxEPD_BLACK);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(120, 75);
  display.println("Synapse");
  display.update();
}
