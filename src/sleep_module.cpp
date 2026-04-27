#include "sleep_module.h"
#include "display_module.h"
#include "wifi_module.h"
#include "bt_module.h"
#include <esp_sleep.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
extern bool wifiActive;
extern bool btActive;

void setupSleep() {
    pinMode(SLEEP_PIN, INPUT_PULLUP); // Switch conecta a GND cuando está "ON/Sleep"
}

void enterLightSleep() {
    Serial.println("[SLEEP] Preparando modo bajo consumo...");
    
    // 1. Apagar radios para ahorrar energía antes de dormir
    if (wifiActive) {
        WiFi.disconnect(true);
        wifiActive = false;
    }
    if (btActive) {
        // No podemos llamar stopBluetooth() aquí porque puede usar Serial dentro de ISR/contexto raro
        // Pero como btActive es externo, lo manejamos en checkSleepMode antes de llegar aquí
    }
    
    // 2. Apagar pantalla (sin clear, solo dejar de refrescar)
    display.clearBuffer();
    display.sendBuffer();
    
    // 3. Configurar wake-up: cuando el switch se suelte (pase a HIGH)
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable((gpio_num_t)SLEEP_PIN, GPIO_INTR_HIGH_LEVEL);
    
    Serial.println("[SLEEP] Entrando en light sleep... Zzz");
    delay(100);
    
    esp_light_sleep_start();
    
    // ======== AL DESPERTAR (continúa desde aquí) ========
    Serial.println("[SLEEP] Despertando...");
    
    // Restaurar pantalla
    display.begin();
    delay(50);
    
    // Mensaje de wake
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(20, 30, "FLIPPER-DIY");
    display.drawStr(25, 45, "DESPERTANDO...");
    display.sendBuffer();
    delay(800);
}

void checkSleepMode() {
    // Si el switch está cerrado (LOW) -> entrar en sleep
    if (digitalRead(SLEEP_PIN) == LOW) {
        delay(200); // Debounce
        
        if (digitalRead(SLEEP_PIN) == LOW) {
            // Apagar BT de forma segura antes de dormir
            extern BluetoothSerial SerialBT; // de bt_module.h
            if (btActive) {
                SerialBT.end();
                btActive = false;
            }
            
            enterLightSleep();
        }
    }
}