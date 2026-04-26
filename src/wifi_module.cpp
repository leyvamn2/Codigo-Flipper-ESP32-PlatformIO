#include "wifi_module.h"
#include "display_module.h"
#include "sd_module.h"
#include <WiFi.h>

bool wifiActive = false;
unsigned long wifiStartTime = 0;
const unsigned long WIFI_TIMEOUT = 300000;

// Redes escaneadas (para conexión rápida)
String redesEncontradas[10];
int numRedesEncontradas = 0;

// Servidor TCP para modo transmisión de datos
WiFiServer dataServer(8080);

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
#define JOY_Y  35
#define JOY_SW 32

// ==================== BÁSICOS ====================
void setupWiFi() {
    if (wifiActive) {
        Serial.println("[WARN] WiFi ya está activo");
        return;
    }
    Serial.println("[INFO] Inicializando WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    WiFi.setSleep(false);
    wifiActive = true;
    wifiStartTime = millis();
    Serial.println("[OK] WiFi STA mode iniciado");
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 30, "WiFi Iniciado");
    display.sendBuffer();
    delay(1000);
}

void stopWiFi() {
    if (!wifiActive) return;
    Serial.println("[INFO] Deteniendo WiFi...");
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    wifiActive = false;
    Serial.println("[OK] WiFi desactivado");
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 30, "WiFi desactivado");
    display.sendBuffer();
    delay(800);
}

bool getWiFiStatus() {
    if (wifiActive && millis() - wifiStartTime > WIFI_TIMEOUT) {
        Serial.println("[INFO] WiFi timeout - desactivando");
        stopWiFi();
        return false;
    }
    return wifiActive && WiFi.status() == WL_CONNECTED;
}

// ==================== ESCANEAR ====================
void scanNetworks() {
    if (!wifiActive) setupWiFi();
    
    Serial.println("[INFO] Escaneando redes WiFi...");
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 20, "Escaneando...");
    display.sendBuffer();
    
    int numNetworks = WiFi.scanNetworks(false, false, false, 300);
    numRedesEncontradas = (numNetworks > 10) ? 10 : numNetworks;
    for (int i = 0; i < numRedesEncontradas; i++) {
        redesEncontradas[i] = WiFi.SSID(i);
    }
    
    if (numNetworks == 0) {
        display.clearBuffer();
        display.drawStr(10, 30, "Sin redes");
        display.sendBuffer();
        delay(1500);
        return;
    }
    
    // Mostrar redes (máx 4)
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(5, 10, "Redes encontradas:");
    display.drawLine(0, 14, 128, 14);
    for (int i = 0; i < (numNetworks > 4 ? 4 : numNetworks); i++) {
        String ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);
        if (ssid.length() > 12) ssid = ssid.substring(0, 12) + "..";
        display.setCursor(5, 24 + (i * 10));
        display.printf("%s (%d)", ssid.c_str(), rssi);
    }
    display.setCursor(5, 64);
    display.printf("Total: %d", numNetworks);
    display.sendBuffer();
    
    while (digitalRead(JOY_SW) == HIGH) delay(100);
    delay(300);
    stopWiFi();
}

// ==================== CONECTAR ====================
void connectToNetwork(String ssid, String password) {
    if (!wifiActive) setupWiFi();
    Serial.printf("[INFO] Conectando a %s...\n", ssid.c_str());
    
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 20, "Conectando a:");
    display.setCursor(10, 35);
    display.print(ssid);
    display.sendBuffer();
    
    if (password.length() > 0) WiFi.begin(ssid.c_str(), password.c_str());
    else WiFi.begin(ssid.c_str());
    
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
        delay(500);
        intentos++;
        display.clearBuffer();
        display.drawStr(10, 20, "Conectando...");
        display.setCursor(10, 35);
        display.print(ssid);
        display.setCursor(10, 50);
        for (int i = 0; i < (intentos % 4); i++) display.print(".");
        display.sendBuffer();
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[OK] Conectado");
        display.clearBuffer();
        display.setFont(u8g2_font_6x10_tf);
        display.drawStr(10, 15, "Conectado!");
        display.setCursor(10, 30);
        display.print("IP: ");
        display.print(WiFi.localIP().toString().c_str());
        display.sendBuffer();
        delay(3000);
    } else {
        Serial.println("[ERROR] Fallo");
        display.clearBuffer();
        display.drawStr(10, 30, "Error de conexion");
        display.sendBuffer();
        delay(1500);
        stopWiFi();
    }
}

// ==================== ACCESS POINT ====================
void startAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    if (!WiFi.softAP("FLIPPER-DIY", "12345678")) {
        display.clearBuffer();
        display.drawStr(10, 30, "Error AP");
        display.sendBuffer();
        delay(1500);
        return;
    }
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(5, 15, "Modo Access Point");
    display.drawStr(5, 28, "SSID: FLIPPER-DIY");
    display.drawStr(5, 40, "PASS: 12345678");
    display.setCursor(5, 52);
    display.print("IP: ");
    display.print(WiFi.softAPIP().toString().c_str());
    display.sendBuffer();
    delay(3000);
}

// ==================== HONEYPOT ====================
void startHoneypot() {
    if (wifiActive) stopWiFi();
    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    
    String ssid = "Free_Public_WiFi";
    WiFi.softAP(ssid.c_str(), "12345678");
    wifiActive = true;
    
    bool enHoneypot = true;
    while (enHoneypot) {
        int clientes = WiFi.softAPgetStationNum();
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(5, 10, "HONEYPOT ACTIVO");
        display.drawLine(0, 14, 128, 14);
        display.setCursor(5, 26);
        display.print("SSID: Free_WiFi");
        display.setCursor(5, 38);
        display.printf("Clientes: %d", clientes);
        display.setCursor(5, 50);
        display.print("Click para salir");
        display.sendBuffer();
        
        if (digitalRead(JOY_SW) == LOW) {
            delay(300);
            enHoneypot = false;
        }
        delay(500);
    }
    WiFi.softAPdisconnect(true);
    wifiActive = false;
}

// ==================== SERVIDOR DE DATOS ====================
void flujoServidorDatos() {
    if (wifiActive) stopWiFi();
    
    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    WiFi.softAP("FLIPPER-DATA", "12345678");
    delay(500);
    dataServer.begin();
    
    bool enServidor = true;
    int datosRecibidos = 0;
    unsigned long lastActivity = millis();
    
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(5, 10, "SERVIDOR DATOS");
    display.drawLine(0, 14, 128, 14);
    display.setCursor(5, 26);
    display.print("AP: FLIPPER-DATA");
    display.setCursor(5, 38);
    display.print("IP: 192.168.4.1");
    display.setCursor(5, 50);
    display.print("Puerto: 8080");
    display.setCursor(5, 62);
    display.print("Esperando...");
    display.sendBuffer();
    delay(2000);
    
    while (enServidor) {
        WiFiClient client = dataServer.available();
        if (client) {
            while (client.connected()) {
                if (client.available()) {
                    String line = client.readStringUntil('\n');
                    line.trim();
                    if (line.length() > 0) {
                        File f = SD.open("/datos.txt", FILE_APPEND);
                        if (f) { f.println(line); f.close(); }
                        datosRecibidos++;
                        lastActivity = millis();
                        client.println("OK:" + String(datosRecibidos));
                    }
                }
                display.clearBuffer();
                display.drawStr(5, 10, "RECIBIENDO...");
                display.setCursor(5, 28);
                display.printf("Total: %d", datosRecibidos);
                display.drawStr(5, 42, "Click para detener");
                display.sendBuffer();
                
                if (digitalRead(JOY_SW) == LOW) {
                    delay(300);
                    client.stop();
                    enServidor = false;
                    break;
                }
                delay(50);
            }
            client.stop();
        }
        if (millis() - lastActivity > 300000) enServidor = false;
        if (digitalRead(JOY_SW) == LOW) { delay(300); enServidor = false; }
        delay(100);
    }
    
    dataServer.stop();
    WiFi.softAPdisconnect(true);
    wifiActive = false;
    display.clearBuffer();
    display.drawStr(10, 30, "Servidor detenido");
    display.sendBuffer();
    delay(1000);
}

// ==================== FLUJO PRINCIPAL ====================
void flujoWiFi(bool &dentroDeOpcion) {
    bool enFlujoWiFi = true;
    int indexWiFi = 0;
    const int NUM_OPCIONES = 5;
    String opcionesWiFi[NUM_OPCIONES] = {
        "1. Escanear",
        "2. Conectar",
        "3. Honeypot",
        "4. Servidor Datos",
        "5. Regresar"
    };
    
    while (enFlujoWiFi) {
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.setCursor(85, 10);
        display.print(getWiFiStatus() ? "WF:ON" : "WF:OFF");
        display.drawLine(0, 14, 128, 14);
        
        if (wifiActive) {
            unsigned long t = (WIFI_TIMEOUT - (millis() - wifiStartTime)) / 1000;
            display.setCursor(10, 10);
            display.printf("T:%ldm", t / 60);
        }
        for (int i = 0; i < NUM_OPCIONES; i++) {
            int yPos = 24 + (i * 11);
            if (i == indexWiFi) display.drawStr(0, yPos, ">");
            display.setCursor(10, yPos);
            display.print(opcionesWiFi[i]);
        }
        display.sendBuffer();
        
        int valorY = analogRead(JOY_Y);
        if (valorY < 1000) { if (indexWiFi > 0) indexWiFi--; delay(200); }
        if (valorY > 3000) { if (indexWiFi < NUM_OPCIONES - 1) indexWiFi++; delay(200); }
        
        if (digitalRead(JOY_SW) == LOW) {
            delay(300);
            switch (indexWiFi) {
                case 0: scanNetworks(); break;
                case 1: {
                    // Submenú de selección de red
                    if (numRedesEncontradas > 0) {
                        int sel = 0;
                        bool selRed = true;
                        while (selRed) {
                            display.clearBuffer();
                            display.drawStr(5, 10, "SELECCIONAR RED:");
                            display.drawLine(0, 12, 128, 12);
                            for (int i = 0; i < 3; i++) {
                                int idx = sel + i;
                                if (idx >= numRedesEncontradas) break;
                                int y = 22 + i * 11;
                                if (i == 0) display.drawStr(0, y, ">");
                                display.setCursor(8, y);
                                String s = redesEncontradas[idx];
                                if (s.length() > 14) s = s.substring(0, 14);
                                display.print(s);
                            }
                            display.setCursor(5, 60);
                            display.printf("%d/%d", sel + 1, numRedesEncontradas);
                            display.sendBuffer();
                            
                            int vy = analogRead(JOY_Y);
                            if (vy < 1000) { if (sel > 0) sel--; delay(200); }
                            if (vy > 3000) { if (sel < numRedesEncontradas - 1) sel++; delay(200); }
                            if (digitalRead(JOY_SW) == LOW) {
                                delay(300);
                                connectToNetwork(redesEncontradas[sel], "");
                                selRed = false;
                            }
                        }
                    } else {
                        connectToNetwork("TuSSID", "TuPassword");
                    }
                    break;
                }
                case 2: startHoneypot(); break;
                case 3: flujoServidorDatos(); break;
                case 4:
                    enFlujoWiFi = false;
                    dentroDeOpcion = false;
                    if (wifiActive) stopWiFi();
                    break;
            }
        }
    }
}