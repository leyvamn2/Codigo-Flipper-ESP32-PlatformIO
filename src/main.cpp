#include <Arduino.h>
#include <led_control.h>

void setup() {
//setupLED(); // Configuramos el pin del LED
Serial.begin(115200);
}

void loop() {
parpadearLED(500);
}
