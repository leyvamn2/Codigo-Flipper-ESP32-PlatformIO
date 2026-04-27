#include "sleep_module.h"
#include "display_module.h"
#include "wifi_module.h"
#include "bt_module.h"
#include <esp_sleep.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;

#define SLEEP_PIN 14

void setupSleep() {
    pinMode(SLEEP_PIN, INPUT_PULLUP);
}

// Apaga radios de forma limpia usando las funciones que ya existen
static void detenerTodo() {
    stopWiFi();
    stopBluetooth();
}

// Rutina central de suspensión
static void entrarEnLightSleep() {
    Serial.println("[SLEEP] Preparando suspensión...");

    detenerTodo();

    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(30, 28, "SUSPENDIDO");
    display.drawStr(10, 44, "Mueve switch a ON");
    display.sendBuffer();
    delay(1000);

    // Pantalla en negro
    display.clearBuffer();
    display.sendBuffer();

    // Wake-up: cuando el switch se mueva a HIGH (posición RUN)
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable((gpio_num_t)SLEEP_PIN, GPIO_INTR_HIGH_LEVEL);

    Serial.println("[SLEEP] Entrando en light sleep... Zzz");
    delay(100);

    esp_light_sleep_start();

    // ========== DESPERTAR ==========
    Serial.println("[SLEEP] Despertando...");

    // Reinicializar pantalla
    display.begin();
    delay(50);

    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(30, 30, "FLIPPER-DIY");
    display.drawStr(25, 45, "DESPERTANDO...");
    display.sendBuffer();
    delay(800);
}

// Verifica el switch físico. Solo duerme si estamos en el menú principal
void checkSleepMode(bool &dentroDeOpcion) {
    if (digitalRead(SLEEP_PIN) == LOW) {
        delay(50); // debounce

        if (digitalRead(SLEEP_PIN) == LOW) {
            if (dentroDeOpcion) {
                // Estamos dentro de un submenú (WiFi, BT, IR, NFC...).
                // NO suspender para no corromper la operación.
                // Silencioso para no interferir con la pantalla del módulo activo.
                Serial.println("[SLEEP] Bloqueado: operación en curso");
                return;
            }
            // Menú principal y switch en OFF → dormir
            entrarEnLightSleep();
        }
    }
}

// Opción de menú "Suspender"
void suspenderDispositivo(bool &dentroDeOpcion) {
    // Doble chequeo: nunca suspender si estamos dentro de una función
    if (dentroDeOpcion) {
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 30, "No disponible");
        display.drawStr(10, 45, "en operacion");
        display.sendBuffer();
        delay(1500);
        return;
    }

    // Si el switch ya está en OFF, suspender directo
    if (digitalRead(SLEEP_PIN) == LOW) {
        entrarEnLightSleep();
        return;
    }

    // Si el switch está en ON, pedir al usuario que lo mueva
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(5, 20, "Mueve el switch");
    display.drawStr(5, 35, "a posicion OFF");
    display.drawStr(5, 50, "para suspender...");
    display.sendBuffer();

    // Esperar hasta 5 segundos a que el usuario mueva el switch
    unsigned long t0 = millis();
    while (digitalRead(SLEEP_PIN) == HIGH && millis() - t0 < 5000) {
        delay(100);
    }

    if (digitalRead(SLEEP_PIN) == LOW) {
        entrarEnLightSleep();
    } else {
        // Se acabó el tiempo sin mover el switch
        display.clearBuffer();
        display.drawStr(10, 30, "Cancelado");
        display.sendBuffer();
        delay(800);
    }
}