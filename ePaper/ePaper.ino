#include <SPI.h>
#include <epd2in9.h>
#include <epdpaint.h>

unsigned char image[1024];
Paint paint(image, 0, 0);
Epd epaper;

#define COLORED     0
#define UNCOLORED   1

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

//  // Height and width need to be multiples of 8?
//  paint.SetWidth(32); // vertical height
//  paint.SetHeight(504); // horizontal width (max 504?)
//  paint.SetRotate(ROTATE_0);
//
//  paint.Clear(COLORED);
//  // Position within frame memory block (horizontal, vertical)
//  paint.DrawStringAt(0, 8, "START 2 PLAYER GAME", &Font16, UNCOLORED);
//  // Position on screen? (vertical, horizontal) coordinate of bottom left corner
//  epaper.SetFrameMemory(paint.GetImage(), 40, 0, paint.GetWidth(), paint.GetHeight());
//
//  paint.Clear(COLORED);
//  // Position within frame memory block (horizontal, vertical)
//  paint.DrawStringAt(0, 8, "START 1 PLAYER GAME", &Font20, UNCOLORED);
//  // Position on screen? (vertical increasing upward, horizontal)
//  epaper.SetFrameMemory(paint.GetImage(), 8, 0, paint.GetWidth(), 288);

  paint.SetRotate(ROTATE_90);
  paint.SetWidth(32);
  paint.SetHeight(EPD_HEIGHT);

//  paint.Clear(COLORED);
//  paint.DrawStringAt(0, 8, "START 1 PLAYER GAME", &Font16, UNCOLORED);
//  epaper.SetFrameMemory(paint.GetImage(), 50, 0, paint.GetWidth(), paint.GetHeight());

  paint.Clear(COLORED);
  // Position within frame memory block (horizontal, vertical)
  paint.DrawStringAt(0, 8, "START 2 PLAYER GAME", &Font16, UNCOLORED);
  // Position on screen? (vertical, horizontal) coordinate of bottom left corner
  epaper.SetFrameMemory(paint.GetImage(), 10, 0, paint.GetWidth(), paint.GetHeight());

  epaper.DisplayFrame();
}

void loop() {
  // put your main code here, to run repeatedly:

}
