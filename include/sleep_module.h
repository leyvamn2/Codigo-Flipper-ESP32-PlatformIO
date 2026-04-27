#ifndef SLEEP_MODULE_H
#define SLEEP_MODULE_H

#include <Arduino.h>

void setupSleep();
void checkSleepMode(bool &dentroDeOpcion);
void suspenderDispositivo(bool &dentroDeOpcion);

#endif