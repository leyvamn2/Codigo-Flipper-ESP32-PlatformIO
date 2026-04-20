#ifndef DISPLAY_MODULE_H
#define DISPLAY_MODULE_H

#include <U8g2lib.h>
#include <Wire.h>

void setupDisplay();
// Solo recibe las variables para dibujarlas, no las modifica
void dibujarPantalla(int menuIndex, bool enSubMenu, String items[], int numItems);

#endif