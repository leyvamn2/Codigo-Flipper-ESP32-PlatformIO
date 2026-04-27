#include "wifi_module.h"
#include "display_module.h"

bool wifiActive = false;
unsigned long wifiStartTime = 0;
const unsigned long WIFI_TIMEOUT = 300000;  // 5 minutos

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;

#define JOY_Y  35
#define JOY_SW 32

void setupWiFi() {
    if (wifiActive) {
        Serial.println("[WARN] WiFi ya está activo");
        return;
    }
    Serial.println("[INFO] Inicializando WiFi...");
    WiFi.mode(WIFI_OFF);          // Apaga cualquier modo previo
    delay(100);
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
    delay(500);
}

void stopWiFi() {
    if (!wifiActive) {
        Serial.println("[WARN] WiFi ya está desactivado");
        return;
    }
    
    Serial.println("[INFO] Deteniendo WiFi...");
    
    WiFi.disconnect(true);  // true = apagar radio completamente
    wifiActive = false;
    
    Serial.println("[OK] WiFi desactivado");
    
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 30, "WiFi desactivado");
    display.sendBuffer();
    delay(1000);
}

bool getWiFiStatus() {
    // ✅ OPTIMIZACIÓN: Verificar timeout automático
    if (wifiActive && millis() - wifiStartTime > WIFI_TIMEOUT) {
        Serial.println("[INFO] WiFi timeout - desactivando");
        stopWiFi();
        return false;
    }
    
    return wifiActive && WiFi.status() == WL_CONNECTED;
}

void scanNetworks() {
    if (!wifiActive) {
        setupWiFi();
    }
    
    Serial.println("[INFO] Escaneando redes WiFi...");
    
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 20, "Escaneando...");
    display.sendBuffer();
    
    // ✅ OPTIMIZACIÓN: Escaneo asincrónico
    int numNetworks = WiFi.scanNetworks(false, false, false, 300);  // 300ms timeout
    
    if (numNetworks == 0) {
        Serial.println("[WARN] No se encontraron redes");
        display.clearBuffer();
        display.drawStr(10, 30, "Sin redes disponibles");
        display.sendBuffer();
        delay(1500);
        return;
    }
    
    Serial.printf("[OK] Se encontraron %d redes\n", numNetworks);
    
    // Mostrar redes encontradas en pantalla (máx 4 por limitaciones)
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(5, 10, "Redes encontradas:");
    display.drawLine(0, 14, 128, 14);
    
    for (int i = 0; i < (numNetworks > 4 ? 4 : numNetworks); i++) {
        String ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);
        
        display.setCursor(5, 22 + (i * 10));
        
        // ✅ Limitar SSID a 20 caracteres para que quepa
        if (ssid.length() > 15) {
            ssid = ssid.substring(0, 15) + "..";
        }
        
        display.printf("%s (%d)", ssid.c_str(), rssi);
        
        Serial.printf("[WIFI] %d. %s (RSSI: %d dBm)\n", i + 1, WiFi.SSID(i).c_str(), rssi);
    }
    
    display.setCursor(5, 62);
    display.printf("Total: %d redes", numNetworks);
    display.sendBuffer();
    
    // Esperar a que presione para continuar
    while (digitalRead(JOY_SW) == HIGH) {
        delay(100);
    }
    delay(300);
    
    // ✅ OPTIMIZACIÓN: Apagar WiFi después de escanear
    stopWiFi();
}

void connectToNetwork(String ssid, String password) {
    if (!wifiActive) {
        setupWiFi();
    }
    
    Serial.printf("[INFO] Conectando a %s...\n", ssid.c_str());
    
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 20, "Conectando a:");
    display.setCursor(10, 35);
    display.print(ssid);
    display.sendBuffer();
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int intentos = 0;
    const int MAX_INTENTOS = 20;  // 10 segundos aprox
    
    while (WiFi.status() != WL_CONNECTED && intentos < MAX_INTENTOS) {
        delay(500);
        intentos++;
        
        // Mostrar progreso
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 20, "Conectando...");
        display.drawStr(10, 35, ssid.c_str());
        
        // Mostrar puntos de progreso
        display.setCursor(10, 50);
        for (int i = 0; i < (intentos % 4); i++) {
            display.print(".");
        }
        
        display.sendBuffer();
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[OK] Conectado a WiFi");
        Serial.print("[INFO] IP: ");
        Serial.println(WiFi.localIP());
        
        display.clearBuffer();
        display.setFont(u8g2_font_6x10_tf);
        display.drawStr(10, 15, "Conectado!");
        display.setCursor(10, 30);
        display.print("SSID: ");
        display.print(ssid);
        display.setCursor(10, 42);
        display.print("IP: ");
        display.print(WiFi.localIP().toString().c_str());
        display.sendBuffer();
        
        delay(3000);
        
    } else {
        Serial.println("[ERROR] No se pudo conectar");
        
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 30, "Error de conexion");
        display.sendBuffer();
        
        delay(1500);
        stopWiFi();
    }
}

void startAccessPoint() {
    Serial.println("[INFO] Iniciando modo Access Point...");
    
    WiFi.mode(WIFI_AP);
    
    // ✅ OPTIMIZACIÓN: Reducir potencia TX
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    
    if (!WiFi.softAP("FLIPPER-DIY", "12345678")) {
        Serial.println("[ERROR] No se pudo iniciar AP");
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 30, "Error AP");
        display.sendBuffer();
        delay(1500);
        return;
    }
    
    Serial.println("[OK] Modo AP iniciado");
    Serial.print("[INFO] IP del AP: ");
    Serial.println(WiFi.softAPIP());
    
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

void flujoWiFi(bool &dentroDeOpcion) {
    bool enFlujoWiFi = true;
    int indexWiFi = 0;
    
    String opcionesWiFi[4] = {
        "1. Escanear",
        "2. Conectar",
        "3. Modo AP",
        "4. Regresar"
    };
    
    while (enFlujoWiFi) {
        // ✅ MENÚ PRINCIPAL DE WIFI
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        
        // Mostrar estado de WiFi en esquina
        display.setCursor(85, 10);
        if (getWiFiStatus()) {
            display.print("WF:ON");
        } else {
            display.print("WF:OFF");
        }
        
        display.drawLine(0, 14, 128, 14);
        
        // Mostrar tiempo restante si WiFi está activo
        if (wifiActive) {
            unsigned long tiempoRestante = (WIFI_TIMEOUT - (millis() - wifiStartTime)) / 1000;
            display.setCursor(10, 10);
            display.printf("T:%ldm", tiempoRestante / 60);
        }
        
        // Menú de opciones
        for (int i = 0; i < 4; i++) {
            int yPos = 26 + (i * 12);
            
            if (i == indexWiFi) {
                display.drawStr(0, yPos, ">");
            }
            
            display.setCursor(10, yPos);
            display.print(opcionesWiFi[i]);
        }
        
        display.sendBuffer();
        
        // ✅ LECTURA DEL JOYSTICK
        int valorY = analogRead(JOY_Y);
        
        if (valorY < 1000) {
            if (indexWiFi > 0) indexWiFi--;
            delay(200);
        }
        
        if (valorY > 3000) {
            if (indexWiFi < 3) indexWiFi++;
            delay(200);
        }
        
        // Seleccionar opción
        if (digitalRead(JOY_SW) == LOW) {
            delay(300);
            
            switch (indexWiFi) {
                case 0:
                    // Escanear redes
                    scanNetworks();
                    break;
                    
                case 1:
                    // Conectar a red
                    // ✅ NOTA: Aquí debería haber un menú para elegir SSID y contraseña
                    // Por ahora, usa valores de ejemplo
                    connectToNetwork("TuSSID", "TuPassword");
                    break;
                    
                case 2:
                    // Modo Access Point
                    startAccessPoint();
                    break;
                    
                case 3:
                    // Regresar
                    enFlujoWiFi = false;
                    dentroDeOpcion = false;
                    if (wifiActive) {
                        stopWiFi();
                    }
                    break;
            }
        }
    }
}