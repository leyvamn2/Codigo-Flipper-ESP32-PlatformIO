#ifndef BATTERY_MODULE_H
#define BATTERY_MODULE_H

#include <Arduino.h>
#include <U8g2lib.h>

void setupBattery();
int getBatteryPercentage();
void drawBatteryIcon(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &display, int x, int y);

#endif