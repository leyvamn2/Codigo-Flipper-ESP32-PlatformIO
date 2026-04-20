#ifndef SD_MODULE_H
#define SD_MODULE_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 5

bool setupSD();
void guardarTarjeta(String uid);
String leerListaTarjetas();
void borrarTodasLasTarjetas();

#endif