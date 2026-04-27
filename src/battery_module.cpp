#include "battery_module.h"

#define BAT_PIN 34          // ADC1_CH6 (ESP32 clásico). Usa GPIO4 si tienes ESP32-S3
#define ADC_MAX 4095.0
#define V_REF 3.3
#define DIVIDER_RATIO 2.0   // Divisor 10k/10k

void setupBattery() {
    analogSetAttenuation(ADC_11db); // Permite leer hasta ~3.3V (con divisor soporta hasta 6.6V)
    pinMode(BAT_PIN, INPUT);
}

int getBatteryPercentage() {
    int raw = 0;
    // Promedio de 10 lecturas para estabilidad
    for (int i = 0; i < 10; i++) {
        raw += analogRead(BAT_PIN);
        delay(1);
    }
    raw /= 10;
    
    float voltage = (raw / ADC_MAX) * V_REF * DIVIDER_RATIO;
    
    // LiPo: 3.0V = 0%, 4.2V = 100%
    int pct = map((int)(voltage * 100), 300, 420, 0, 100);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

void drawBatteryIcon(U8G2_SSD1306_128X64_NONAME_F_HW_I2C &display, int x, int y) {
    int pct = getBatteryPercentage();
    
    // Marco de batería
    display.drawFrame(x, y, 14, 8);
    display.drawBox(x + 14, y + 2, 2, 4); // Terminal positivo
    
    // Nivel de carga (máx 10 px internos)
    int fill = (10 * pct) / 100;
    if (fill < 0) fill = 0;
    if (fill > 10) fill = 10;
    
    // Color de relleno según nivel (solo visible si la pantalla lo soporta, aquí dibujamos relleno)
    display.drawBox(x + 2, y + 2, fill, 4);
    
    // Si batería < 20%, mostrar símbolo de alerta (opcional, se puede agregar en main)
}