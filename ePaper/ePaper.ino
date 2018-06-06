//#include <SPI.h>
#include <epd2in9.h>
#include <epdpaint.h>

/* 
 * Arduino Uno
 * BUSY 7 
 * RST  8
 * DC   9
 * CS   10
 * CLK  13
 * DIN  11
 */

 /*
  * ESP8266
  * BUSY 2
  * DC   15
  * MISO 14
  * CLK  12
  * CS   13
  */

#define BUSY_PIN 2s
#define RST_PIN  20
#define CS_PIN   13
#define DC_PIN   15

unsigned char image[1024];
Paint paint(image, 0, 0);
Epd epaper;

#define COLORED     0
#define UNCOLORED   1

int score = 0;

void setup() {
  Serial.begin(115200);

  // Initialize display
  if (epaper.Init(lut_full_update) != 0) {
    Serial.print("e-Paper init failed");
    return;
  }
  
  epaper.ClearFrameMemory(0xFF);
  epaper.DisplayFrame();
  epaper.ClearFrameMemory(0xFF);
  epaper.DisplayFrame();

  updateScoreDisplay();
  delay(2000);
}

void loop() {
  score++;

  Serial.print("Number of matches found: ");
  Serial.println(score);

  updateScoreDisplay();
  
  delay(2000);
}

void updateScoreDisplay() {
  paint.SetRotate(ROTATE_270);
  paint.SetWidth(32);
  paint.SetHeight(256); // width
  
  paint.Clear(UNCOLORED);
//  // Position within frame memory block (horizontal, vertical)
  paint.DrawStringAt(0, 8, "NUMBER OF         ", &Font20, COLORED);
//  // Position on screen? (vertical, horizontal) coordinate of bottom left corner
//    // horizontal increasing goes more left
//    // vertical increasing goes down
  epaper.SetFrameMemory(paint.GetImage(), 32, 24, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 8, "MATCHES FOUND         ", &Font20, COLORED);
  epaper.SetFrameMemory(paint.GetImage(), 56, 24, paint.GetWidth(), paint.GetHeight());

  paint.SetRotate(ROTATE_270);
  paint.SetWidth(32);
  paint.SetHeight(32);

  paint.Clear(UNCOLORED);
  char str[] = {'0', '\0'};
  str[0] = score + '0';
  Serial.println(str);
  paint.DrawStringAt(0, 8, str, &Font24, COLORED);
  epaper.SetFrameMemory(paint.GetImage(), 40, 16, paint.GetWidth(), paint.GetHeight());

  epaper.DisplayFrame();
}

