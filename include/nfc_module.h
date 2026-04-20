#ifndef NFC_MODULE_H
#define NFC_MODULE_H

#include <Arduino.h>
#include <Adafruit_PN532.h>
#include <U8g2lib.h>

// Le decimos al módulo que la variable "display" existe en otro archivo
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;

void setupNFC();
bool capturarUID();
String obtenerUltimoUID();
bool clonarTarjeta();
void flujoCapturaRFID(bool &dentroDeOpcion);

#endif

