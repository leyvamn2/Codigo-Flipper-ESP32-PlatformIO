#include "nfc_module.h"
#include "sd_module.h"

// Asegúrate de que display esté disponible (extern si viene de otro archivo)
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C display; 

#define JOY_Y  35
#define JOY_SW 32
#define PN532_IRQ   (21)  
#define PN532_RESET (5) 

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
uint8_t uidLeido[7];
uint8_t uidLongitud = 0;
String uidString = "";

// --- INICIALIZACIÓN ---
void setupNFC() {
    nfc.begin();               
    uint32_t version = nfc.getFirmwareVersion();
    if (!version) {
        Serial.println("Error: PN532 no encontrado.");
        return;
    }
    nfc.SAMConfig();
    Serial.println("NFC listo.");
}

// --- FUNCIONES DE APOYO ---
bool capturarUID() {
    uidString = "";
    // Bajamos el timeout a 500ms para que el joystick responda más rápido
    uint8_t success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidLeido, &uidLongitud, 500);
    
    if (success) {
        for (uint8_t i = 0; i < uidLongitud; i++) {
            if (uidLeido[i] < 0x10) uidString += "0"; // Formato HEX profesional
            uidString += String(uidLeido[i], HEX);
        }
        uidString.toUpperCase();
        return true;
    }
    return false;
}

void guardarTarjetaEnSD(String uid) {
    File dataFile = SD.open("/nfc_log.txt", FILE_WRITE);
    if (dataFile) {
        dataFile.println("UID: " + uid);
        dataFile.close();
    }
}

bool clonarTarjeta() {
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t uidDestino[7];
    uint8_t uidLenDestino;

    display.clearBuffer();
    display.drawStr(10, 30, "Acerque T. Virgen...");
    display.sendBuffer();

    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidDestino, &uidLenDestino, 5000)) return false;
    if (!nfc.mifareclassic_AuthenticateBlock(uidDestino, uidLenDestino, 0, 0, keya)) return false;

    uint8_t bloque0[16] = {0};
    for(int i = 0; i < 4; i++) bloque0[i] = uidLeido[i];
    bloque0[4] = uidLeido[0] ^ uidLeido[1] ^ uidLeido[2] ^ uidLeido[3];
    bloque0[5] = 0x08; bloque0[6] = 0x04; bloque0[7] = 0x00; 

    return nfc.mifareclassic_WriteDataBlock(0, bloque0);
}

// --- VISUALIZAR SD ---
void mostrarSeñalesGuardadas() {
    bool viendo = true;
    while (viendo) {
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(0, 10, "--- SEÑALES EN SD ---");
        
        File dataFile = SD.open("/nfc_log.txt");
        if (dataFile) {
            int yPos = 25;
            while (dataFile.available() && yPos < 60) {
                String linea = dataFile.readStringUntil('\n');
                display.setCursor(0, yPos);
                display.print(linea);
                yPos += 12;
            }
            dataFile.close();
        } else {
            display.drawStr(10, 35, "Archivo no encontrado");
        }
        display.sendBuffer();

        if (digitalRead(JOY_SW) == LOW) { delay(300); viendo = false; }
    }
}

// --- SUBMENÚ DE OPCIONES TRAS CAPTURAR ---
void menuPostCaptura(String uid) {
    String opciones[] = {"1. Guardar SD", "2. Clonar", "3. Regresar"};
    int sel = 0;
    bool salir = false;

    while (!salir) {
        int vy = analogRead(JOY_Y);
        if (vy < 1000) { if (sel > 0) sel--; delay(200); }
        if (vy > 3000) { if (sel < 2) sel++; delay(200); }

        display.clearBuffer();
        display.setCursor(0, 10);
        display.print("ID: " + uid);
        display.drawLine(0, 14, 128, 14);

        for (int i = 0; i < 3; i++) {
            int yPos = 30 + (i * 12);
            display.drawStr(10, yPos, (i == sel ? ">" : " "));
            display.setCursor(22, yPos);
            display.print(opciones[i]);
        }
        display.sendBuffer();

        if (digitalRead(JOY_SW) == LOW) {
            delay(300);
            if (sel == 0) {
                guardarTarjetaEnSD(uid);
                display.drawStr(20, 50, "¡Guardado!");
                display.sendBuffer();
                delay(1000);
            } else if (sel == 1) {
                if(clonarTarjeta()) display.drawStr(20, 50, "¡Clonado OK!");
                else display.drawStr(20, 50, "Error Clonar");
                display.sendBuffer();
                delay(1000);
            }
            salir = true;
        }
    }
}

// --- MENÚ PRINCIPAL DEL MÓDULO ---
void flujoNFC(bool &dentroDeOpcion) {
    int indexNFC = 0;
    String opcionesNFC[3] = {"1. Capturar", "2. Ver SD", "3. Salir"};

    while (dentroDeOpcion) {
        int vy = analogRead(JOY_Y);
        if (vy < 1000) { if (indexNFC > 0) indexNFC--; delay(200); }
        if (vy > 3000) { if (indexNFC < 2) indexNFC++; delay(200); }

        display.clearBuffer();
        display.drawStr(30, 10, "MODO RFID");
        for (int i = 0; i < 3; i++) {
            int yPos = 30 + (i * 12);
            display.drawStr(10, yPos, (i == indexNFC ? ">" : " "));
            display.setCursor(22, yPos);
            display.print(opcionesNFC[i]);
        }
        display.sendBuffer();

        if (digitalRead(JOY_SW) == LOW) {
            delay(300);
            if (indexNFC == 0) {
                display.clearBuffer();
                display.drawStr(10, 35, "Esperando tarjeta...");
                display.sendBuffer();
                while(!capturarUID()) {
                    if (digitalRead(JOY_SW) == LOW) break;
                }
                if (uidString != "") menuPostCaptura(uidString);
            } 
            else if (indexNFC == 1) mostrarSeñalesGuardadas();
            else if (indexNFC == 2) dentroDeOpcion = false;
        }
    }
}