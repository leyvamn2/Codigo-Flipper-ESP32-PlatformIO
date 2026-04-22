#include <Arduino.h>
#include <Wire.h>
#include "joystick_module.h"
#include "display_module.h"
#include "nfc_module.h"
#include "sd_module.h" 

const int NUM_ITEMS = 4;
String menuItems[NUM_ITEMS] = {"RFID / NFC", "Opcion 2", "Opcion 3", "Opcion 4"};

int indiceActual = 0;      
bool dentroDeOpcion = false; 

void setup() {
  Wire.begin(25, 26);
  setupJoystick();
  setupDisplay();
  setupNFC();
  setupSD(); 
}

void loop() {
  actualizarJoystick(indiceActual, dentroDeOpcion, NUM_ITEMS);

  if (dentroDeOpcion && indiceActual == 0) {
      flujoCapturaRFID(dentroDeOpcion);
  } else {
      dibujarPantalla(indiceActual, dentroDeOpcion, menuItems, NUM_ITEMS);
  }
}