#include <Arduino.h>
#include <Wire.h>
#include "joystick_module.h"
#include "display_module.h"
#include "nfc_module.h"
#include "ir_module.h"
#include "sd_module.h"
#include "wifi_module.h"
#include "bt_module.h"
#include "battery_module.h"
#include "sleep_module.h"

const int NUM_ITEMS = 4;
String menuItems[NUM_ITEMS] = {"RFID / NFC", "Infrarrojo IR", "WiFi", "Bluetooth"};

int indiceActual = 0;      
bool dentroDeOpcion = false; 

void setup() {
  Serial.begin(115200);
  Wire.begin(25, 26);
  setupBattery();
  setupJoystick();
  setupDisplay();
  setupNFC();
  setupSD(); 
  setupIR();
}

void loop() {
  checkSleepMode();  // Verifica switch de sleep
  
  actualizarJoystick(indiceActual, dentroDeOpcion, NUM_ITEMS);

  if (dentroDeOpcion && indiceActual == 0) {
      flujoCapturaRFID(dentroDeOpcion);
  }
  else if (dentroDeOpcion && indiceActual == 1) {
      flujoInfrarrojo(dentroDeOpcion);
  }
  else if (dentroDeOpcion && indiceActual == 2) {
      flujoWiFi(dentroDeOpcion);
  }
  else if (dentroDeOpcion && indiceActual == 3) {
      flujoBluetooth(dentroDeOpcion);
  }
  else if (dentroDeOpcion && indiceActual == 4) {
      flujoBadUSB(dentroDeOpcion);   // Solo si usas ESP32-S3
  }
  else {
      // Menú principal
      dibujarPantalla(indiceActual, dentroDeOpcion, menuItems, NUM_ITEMS);
      
      // Dibuja batería en esquina superior derecha (x=110, y=0)
      drawBatteryIcon(display, 110, 0);
      
      // Envía TODO junto: menú + batería
      display.sendBuffer();
      
      delay(50);
  }
}