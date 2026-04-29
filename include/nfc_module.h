#ifndef NFC_MODULE_H
#define NFC_MODULE_H

#include <Arduino.h>
#include <Adafruit_PN532.h>
#include <U8g2lib.h>
#include <SD.h> // Necesario para las funciones que leen la SD

// --- REFERENCIAS EXTERNAS ---
// Asegúrate de que este tipo (SH1106) coincida EXACTAMENTE con el de tu main.cpp
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C display; 


void setupNFC();

void flujoNFC(bool &dentroDeOpcion);


void mostrarSenialesGuardadas();


bool capturarUID();


String obtenerUltimoUID();

bool clonarTarjeta();

#endif