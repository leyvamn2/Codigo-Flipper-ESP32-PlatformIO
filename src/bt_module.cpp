#include "bt_module.h"
#include "display_module.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BluetoothSerial SerialBT;
bool btActive = false;
unsigned long btStartTime = 0;
const unsigned long BT_TIMEOUT = 300000;

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
#define JOY_Y  35
#define JOY_SW 32

// Datos de escaneo BLE
static String bleDeviceNames[10];
static int bleRSSI[10];
static int bleDeviceCount = 0;
static bool bleInitialized = false;

// Callback para capturar dispositivos BLE durante el scan
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (bleDeviceCount < 10) {
            bleDeviceNames[bleDeviceCount] = advertisedDevice.haveName() 
                ? String(advertisedDevice.getName().c_str()) 
                : "Desconocido";
            bleRSSI[bleDeviceCount] = advertisedDevice.getRSSI();
            bleDeviceCount++;
        }
    }
};

// ==================== BÁSICOS ====================
void setupBluetooth() {
    if (btActive) {
        Serial.println("[WARN] BT ya activo");
        return;
    }
    Serial.println("[INFO] Inicializando Bluetooth...");

    // Inicializar stack BLE solo una vez
    if (!bleInitialized) {
        BLEDevice::init("FLIPPER-DIY");
        bleInitialized = true;
    }

    // Inicializar Bluetooth Serial (Classic SPP)
    if (!SerialBT.begin("FLIPPER-DIY")) {
        Serial.println("[ERROR] BT Classic init failed");
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 30, "Error BT");
        display.sendBuffer();
        delay(1500);
        return;
    }

    btActive = true;
    btStartTime = millis();
    Serial.println("[OK] Bluetooth iniciado (Classic + BLE)");

    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 20, "BT Iniciado");
    display.drawStr(10, 35, "Nombre: FLIPPER-DIY");
    display.sendBuffer();
    delay(1500);
}

void stopBluetooth() {
    if (!btActive) return;
    Serial.println("[INFO] Deteniendo BT...");
    SerialBT.end();
    btActive = false;
    display.clearBuffer();
    display.drawStr(10, 30, "BT desactivado");
    display.sendBuffer();
    delay(800);
}

bool getBTStatus() {
    if (btActive && millis() - btStartTime > BT_TIMEOUT) {
        Serial.println("[INFO] BT timeout - desactivando");
        stopBluetooth();
        return false;
    }
    return btActive && SerialBT.hasClient();
}

String readBTData() {
    if (SerialBT.available()) {
        String data = SerialBT.readStringUntil('\n');
        Serial.print("[BT RX] ");
        Serial.println(data);
        return data;
    }
    return "";
}

void sendBTData(String data) {
    if (btActive) {
        SerialBT.println(data);
        Serial.print("[BT TX] ");
        Serial.println(data);
    }
}

// ==================== ESCANEAR BLE ====================
void scanBTDevices() {
    if (!btActive) { setupBluetooth(); delay(500); }
    
    bleDeviceCount = 0;
    for (int i = 0; i < 10; i++) {
        bleDeviceNames[i] = "";
        bleRSSI[i] = 0;
    }

    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(10, 30, "Escaneando BLE...");
    display.sendBuffer();

    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    BLEScanResults foundDevices = pBLEScan->start(5, false);
    pBLEScan->clearResults();

    if (bleDeviceCount == 0) {
        display.clearBuffer();
        display.drawStr(10, 30, "Ninguno encontrado");
        display.sendBuffer();
        delay(1500);
        return;
    }

    // Navegar resultados
    int idx = 0;
    bool viendo = true;
    while (viendo) {
        display.clearBuffer();
        display.setFont(u8g2_font_6x10_tf);
        display.drawStr(5, 10, "DISPOSITIVOS BLE:");
        display.drawLine(0, 12, 128, 12);

        for (int i = 0; i < 3; i++) {
            int id = idx + i;
            if (id >= bleDeviceCount) break;
            int y = 22 + (i * 12);
            if (i == 0) display.drawStr(0, y, ">");
            display.setCursor(8, y);
            String n = bleDeviceNames[id];
            if (n.length() > 10) n = n.substring(0, 10);
            display.printf("%s (%d)", n.c_str(), bleRSSI[id]);
        }
        display.setCursor(5, 60);
        display.printf("%d/%d", idx + 1, bleDeviceCount);
        display.sendBuffer();

        int vy = analogRead(JOY_Y);
        if (vy < 1000) { if (idx > 0) idx--; delay(200); }
        if (vy > 3000) { if (idx < bleDeviceCount - 1) idx++; delay(200); }
        if (digitalRead(JOY_SW) == LOW) { delay(300); viendo = false; }
    }
}

// ==================== SPAMMING BLE ====================
void spamBT() {
    if (!btActive) { setupBluetooth(); delay(500); }

    Serial.println("[INFO] BLE Spam activo");
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tr);
    display.drawStr(5, 10, "BLE SPAM ACTIVO");
    display.drawLine(0, 14, 128, 14);
    display.drawStr(5, 28, "Broadcasting...");
    display.drawStr(5, 42, "Click para detener");
    display.sendBuffer();
    delay(1000);

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    
    // Configuración base de la publicidad
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);

    const char* nombresSpam[] = {
        "AirPods Pro", "Samsung Buds2", "Google Pixel",
        "iPhone de Juan", "Free WiFi", "Flipper Zero"
    };
    const int numNombres = 6;
    int contador = 0;
    bool spamming = true;
    unsigned long lastChange = 0;
    const unsigned long INTERVALO = 800; // ms entre cada cambio de nombre

    while (spamming) {
        if (millis() - lastChange > INTERVALO) {
            String nombre = nombresSpam[contador % numNombres];

            // Detener publicidad actual antes de cambiar datos
            pAdvertising->stop();

            // Crear nuevo paquete de publicidad con el nombre falso
            BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
            oAdvertisementData.setName(nombre.c_str());
            
            // Aplicar los nuevos datos
            pAdvertising->setAdvertisementData(oAdvertisementData);
            
            // Reiniciar publicidad con el nuevo nombre
            pAdvertising->start();

            lastChange = millis();
            contador++;
            Serial.printf("[BLE SPAM] Nombre: %s\n", nombre.c_str());
        }

        display.clearBuffer();
        display.drawStr(5, 10, "SPAMMING BLE...");
        display.setCursor(5, 26);
        display.printf("Ciclos: %d", contador);
        display.setCursor(5, 40);
        display.print("Actual: ");
        String actual = nombresSpam[(contador - 1) % numNombres];
        if (actual.length() > 10) actual = actual.substring(0, 10);
        display.print(actual);
        display.drawStr(5, 56, "Click = detener");
        display.sendBuffer();

        if (digitalRead(JOY_SW) == LOW) {
            delay(300);
            spamming = false;
        }
        delay(50);
    }

    // Detener publicidad al salir
    pAdvertising->stop();
    
    display.clearBuffer();
    display.drawStr(10, 30, "Spam finalizado");
    display.sendBuffer();
    delay(1000);
}

// ==================== RECIBIR DATOS (Classic) ====================
void flujoRecibirDatos() {
    if (!btActive) {
        display.clearBuffer();
        display.drawStr(10, 30, "BT no activo");
        display.sendBuffer();
        delay(1500);
        return;
    }
    display.clearBuffer();
    display.drawStr(10, 20, "Esperando datos...");
    display.drawStr(10, 35, "(Presiona para salir)");
    display.sendBuffer();

    unsigned long inicio = millis();
    const unsigned long TIMEOUT = 30000;
    bool leyendo = true;
    String datos = "";

    while (leyendo) {
        String data = readBTData();
        if (data.length() > 0) {
            datos = data;
            display.clearBuffer();
            display.setFont(u8g2_font_6x10_tf);
            display.drawStr(10, 15, "Dato recibido:");
            display.setCursor(10, 30);
            if (datos.length() > 20) {
                display.print(datos.substring(0, 20));
                display.setCursor(10, 42);
                display.print(datos.substring(20));
            } else {
                display.print(datos);
            }
            display.drawStr(10, 55, "Presiona para cont.");
            display.sendBuffer();
            while (digitalRead(JOY_SW) == HIGH) delay(100);
            delay(300);
            leyendo = false;
        }
        if (millis() - inicio > TIMEOUT) {
            display.clearBuffer();
            display.drawStr(10, 30, "Tiempo agotado");
            display.sendBuffer();
            delay(1500);
            leyendo = false;
        }
        if (digitalRead(JOY_SW) == LOW) { delay(300); leyendo = false; }
        delay(100);
    }
}

// ==================== FLUJO PRINCIPAL ====================
void flujoBluetooth(bool &dentroDeOpcion) {
    bool enFlujoBT = true;
    int indexBT = 0;
    const int NUM_OPCIONES = 5;
    String opcionesBT[NUM_OPCIONES] = {
        "1. Iniciar BT",
        "2. Escanear BLE",
        "3. Spamming BLE",
        "4. Recibir datos",
        "5. Regresar"
    };

    while (enFlujoBT) {
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.setCursor(85, 10);
        display.print(getBTStatus() ? "BT:ON" : "BT:OFF");
        display.drawLine(0, 14, 128, 14);

        if (btActive) {
            unsigned long t = (BT_TIMEOUT - (millis() - btStartTime)) / 1000;
            display.setCursor(10, 10);
            display.printf("T:%ldm", t / 60);
        }
        for (int i = 0; i < NUM_OPCIONES; i++) {
            int yPos = 24 + (i * 11);
            if (i == indexBT) display.drawStr(0, yPos, ">");
            display.setCursor(10, yPos);
            display.print(opcionesBT[i]);
        }
        display.sendBuffer();

        int valorY = analogRead(JOY_Y);
        if (valorY < 1000) { if (indexBT > 0) indexBT--; delay(200); }
        if (valorY > 3000) { if (indexBT < NUM_OPCIONES - 1) indexBT++; delay(200); }

        if (digitalRead(JOY_SW) == LOW) {
            delay(300);
            switch (indexBT) {
                case 0: setupBluetooth(); break;
                case 1: scanBTDevices(); break;
                case 2: spamBT(); break;
                case 3: flujoRecibirDatos(); break;
                case 4:
                    enFlujoBT = false;
                    dentroDeOpcion = false;
                    if (btActive) stopBluetooth();
                    break;
            }
        }
    }
}