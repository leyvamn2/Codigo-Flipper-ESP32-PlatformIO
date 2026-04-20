#include "led_control.h"

void setupLED() {
    pinMode(LED_PIN, OUTPUT);
}

void encenderLED() {
    digitalWrite(LED_PIN, HIGH);
}

void apagarLED() {
    digitalWrite(LED_PIN, LOW);
}

void parpadearLED(int tiempo) {
    encenderLED();
    delay(tiempo);
    apagarLED();
    delay(tiempo);
}

