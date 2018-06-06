#include <SPI.h>
#include <GxEPD.h>
#include <GxGDEH029A1/GxGDEH029A1.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// BUSY -> 4, RST -> 2, DC -> 0, CS -> 15, CLK -> 14, DIN -> 13, GND -> GND, 3.3V -> 3.3V

//#define BUSY 4
//#define DC   0
//#define DIN  13
//#define CLK  14
//#define CS   15
//#define RST  2

// 15, 2, 13, 14, 12
// BUSY -> 4, RST -> 2, DC -> 0, CS -> 15, CLK -> 14, DIN -> 13, GND -> GND, 3.3V -> 3.3V
#define RST  -1
#define DIN  13
#define CLK  14
#define BUSY 12
#define CS   15
#define DC   2

GxIO_Class io(SPI, CS, DC, RST); 
GxEPD_Class display(io, RST, BUSY);

void setup() {
  Serial.begin(115200);
  display.init();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(3);
  
  display.setFont(&FreeSans12pt7b);
  display.setCursor(30, 60);
  display.println("NUMBER OF");
  display.setCursor(30, 90);
  display.println("MATCHES");

  display.setFont(&FreeSansBold24pt7b);
  display.setCursor(230, 82);
  display.println("6");
  display.update();
}

void loop() {
//  display.drawCornerTest();
//  delay(2000);
//  display.drawPaged(showFontCallback);
//  delay(2000);
}
