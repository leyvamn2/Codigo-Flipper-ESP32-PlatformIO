#include <Arduino.h>
#include "display_module.h"

// Configuración para el ESP32 pantalla 0.9''(SCL: 26, SDA: 25)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 26, /* data=*/ 25);
//Configuración para el ESP32 pantalla 1.2'' (SCL: 26, SDA: 25)
// U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock/SCL=*/ 26, /* data/SDA=*/ 25);


void iniciarYSaludar() {
    u8g2.begin();
    u8g2.clearBuffer();					
    u8g2.setFont(u8g2_font_ncenB08_tr);	
    u8g2.drawStr(25, 35, "hello world!");	
    u8g2.sendBuffer();					
}