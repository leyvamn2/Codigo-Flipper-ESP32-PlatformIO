#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>
#include <WiFi.h>

void setupWiFi();
void stopWiFi();
bool getWiFiStatus();
void scanNetworks();
void connectToNetwork(String ssid, String password);
void startAccessPoint();
void startHoneypot();
void flujoServidorDatos();
void flujoWiFi(bool &dentroDeOpcion);

#endif