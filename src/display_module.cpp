#include "display_module.h"

// CONFIGURACIÓN DE PANTALLA 
// OPCIÓN A: Para la pantalla chiquita de 0.96" (Chip SSD1306) 
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 26, /* data=*/ 25);

// OPCIÓN B: Para la pantalla más grande de 1.2" o 1.3" (Chip SH1106)
// U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 26, /* data=*/ 25);

void setupDisplay() {
  display.begin();
}

void dibujarPantalla(int menuIndex, bool enSubMenu, String items[], int numItems) {
  display.clearBuffer();

  if (!enSubMenu) {
    display.setFont(u8g2_font_ncenB08_tr);
    display.drawStr(25, 10, "-- SELECCIONA --");
    display.drawLine(0, 14, 128, 14);

    for (int i = 0; i < numItems; i++) {
      int yPos = 30 + (i * 12); // Estos píxeles cuadran perfecto en ambas pantallas
      if (i == menuIndex) {
        display.drawStr(0, yPos, ">"); 
      }
      display.setCursor(15, yPos);
      display.print(items[i]);
    }

  } else {
    display.setFont(u8g2_font_6x12_tr); 
    
    display.setCursor(5, 20);
    display.print("Has entrado a la");
    display.setCursor(5, 32);
    display.print(items[menuIndex]); 
    
    // Este marco (X=0, Y=45, Ancho=128, Alto=19) encaja en ambas pantallas (128x64)
    display.drawFrame(0, 45, 128, 19); 
    display.setCursor(10, 57);
    display.print("Presiona para SALIR");
  }

  display.sendBuffer();
}