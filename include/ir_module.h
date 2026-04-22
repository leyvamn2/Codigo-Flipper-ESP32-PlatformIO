#ifndef IR_MODULE_H
#define IR_MODULE_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <U8g2lib.h>

// Definición de pines según tu esquemático
#define IR_TX_PIN 4
#define IR_RX_PIN 13

// Para poder usar la pantalla desde este módulo
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
// OPCIÓN B: Para la pantalla más grande de 1.2" o 1.3" (Chip SH1106)
//extern U8G2_SH1106_128X64_NONAME_F_HW_I2C display

void setupIR();
void flujoInfrarrojo(bool &dentroDeOpcion);
bool recibirCodigoIR(String &resultadoHex, uint32_t &codigoCrudo);
void enviarCodigoIR(uint32_t codigo);
void mostrarOpcionesIR(String codigoHex, uint32_t codigoCrudo, bool &dentroDeOpcion);

#endif