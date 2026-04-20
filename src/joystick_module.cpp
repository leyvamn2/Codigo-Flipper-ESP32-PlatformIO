#include "joystick_module.h"

#define JOY_Y  35  
#define JOY_SW 32  

void setupJoystick() {
  pinMode(JOY_SW, INPUT_PULLUP);
}

void actualizarJoystick(int &menuIndex, bool &enSubMenu, int numItems) {
  if (!enSubMenu) {
    int valorY = analogRead(JOY_Y);
    
    if (valorY < 1000) { 
      if (menuIndex > 0) menuIndex--;
      delay(200);
    }
    if (valorY > 3000) { 
      if (menuIndex < numItems - 1) menuIndex++;
      delay(200);
    }

    if (digitalRead(JOY_SW) == LOW) {
      enSubMenu = true;
      delay(300); 
    }
  } else {
    if (digitalRead(JOY_SW) == LOW) {
      enSubMenu = false;
      delay(300); 
    }
  }
}

