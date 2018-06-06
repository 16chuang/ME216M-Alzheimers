#include <epd.h>

const int wake_up = 2;
const int reset = 0;

void setup() {
  Serial.begin(115200);

  epd_init(wake_up,reset);
  epd_wakeup(wake_up);
  epd_set_memory(MEM_NAND);
  epd_set_color(BLACK, WHITE);

  epd_draw_line(0, 300, 799, 300);
  epd_udpate();
}

void loop() {
  // put your main code here, to run repeatedly:

}
