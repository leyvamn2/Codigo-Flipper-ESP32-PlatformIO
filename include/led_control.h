#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>

// Definimos el pin (puedes cambiar el 2 por el que uses)
#define LED_PIN 2 

// Prototipos: Solo avisamos que estas funciones existen
void setupLED();
void parpadearLED(int tiempo);
void encenderLED();
void apagarLED();

#endif

