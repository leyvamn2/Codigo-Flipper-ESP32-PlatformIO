#ifndef SD_MODULE_H
#define SD_MODULE_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 15

bool setupSD();
void guardarTarjeta(String uid);
String leerListaTarjetas();
void borrarTodasLasTarjetas();

//función RFID
void guardarIR(String codigo);

#endif