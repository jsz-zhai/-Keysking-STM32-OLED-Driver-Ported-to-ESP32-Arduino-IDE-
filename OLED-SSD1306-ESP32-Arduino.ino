#include "oled.h"
#include "IIC_1.h"
void setup() {
  // put your setup code here, to run once:
  IIC_init();
  OLED_Init();
}

void loop() {
  // put your main code here, to run repeatedly:
  OLED_NewFrame();
  OLED_DrawImage(0, 16, &bilibiliImg, OLED_COLOR_NORMAL);
  OLED_PrintString(0, 0, "波特律动hello", &font16x16, OLED_COLOR_NORMAL);
  OLED_ShowFrame();
  delay(500);
}
