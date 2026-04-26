#ifndef BT_MODULE_H
#define BT_MODULE_H

#include <Arduino.h>
#include <BluetoothSerial.h>
extern BluetoothSerial SerialBT;

void setupBluetooth();
void stopBluetooth();
bool getBTStatus();
String readBTData();
void sendBTData(String data);
void scanBTDevices();
void spamBT();
void flujoRecibirDatos();
void flujoBluetooth(bool &dentroDeOpcion);

#endif